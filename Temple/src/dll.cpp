#include <exception.h>
#include <logging.h>
#include <platform/windows.h>
#include <Shlwapi.h>
#include <MinHook.h>
#include <psapi.h>

#include "temple/dll.h"

namespace temple {

	constexpr uint32_t defaultBaseAddr = 0x10000000;

	// Helper function that will identify when a function pointer is 
	// called before the DLL has been loaded
	static void CalledTooEarlyGuard() {
		abort();
	}

	/*
	Registry for all function pointers pointing into temple.dll
	address space, so we can manually fix them up once the DLL
	has been loaded.
	*/
	class AddressRegistry {
	public:
		static AddressRegistry& GetInstance() {
			static AddressRegistry sInstance;
			return sInstance;
		}

		void Register(uint32_t* addressPtr, uint32_t address) {
			mAddresses.push_back({addressPtr, address});

			if (mFixupHappened) {
				*addressPtr = address + mDeltaFromVanilla;
			}
		}

		void Fixup(int deltaFromVanilla) {
			for (auto& address : mAddresses) {
				*address.mAddressPtr = address.mVanillaAddress + deltaFromVanilla;
			}
			mFixupHappened = true;
			mDeltaFromVanilla = deltaFromVanilla;
		}

	private:
		struct Address {
			uint32_t* mAddressPtr;
			uint32_t mVanillaAddress;
		};

		bool mFixupHappened = false;
		int mDeltaFromVanilla = 0;
		std::vector<Address> mAddresses;
	};

	class DllImpl {
	public:
		DllImpl(const std::string& installationDir);
		~DllImpl();

		int mDeltaFromVanilla = 0;
		HINSTANCE mDllHandle = nullptr;
	};

	DllImpl::DllImpl(const std::string& installationDir) {

		auto dllPath(installationDir + "\\temple.dll");
		
		// Does it even exist?
		if (!PathFileExistsA(installationDir.c_str())) {
			auto msg(fmt::format("Temple.dll does not exist: {}", dllPath));
			throw TempleException(msg);
		}

		// Try to load it
		mDllHandle = LoadLibraryA(dllPath.c_str());
		if (!mDllHandle) {
			throw TempleException("Unable to load temple.dll from {}: {}",
				dllPath, GetLastWin32Error());
		}

		// calculate the offset from the default 0x10000000 base address
		auto baseAddr = reinterpret_cast<uint32_t>(mDllHandle);
		mDeltaFromVanilla = baseAddr - defaultBaseAddr;
		logger->info("The temple.dll base address delta is: {}", mDeltaFromVanilla);

		auto status = MH_Initialize();
		if (status != MH_OK) {
			FreeLibrary(mDllHandle);
			auto msg(fmt::format("Unable to initialize MinHook: {}", MH_StatusToString(status)));
			throw TempleException(msg);
		}
	}

	DllImpl::~DllImpl() {
		if (mDllHandle) {
			if (!FreeLibrary(mDllHandle)) {
				logger->error("Unable to free the temple.dll library handle: {}",
				              GetLastWin32Error());
			}
		}

		auto status = MH_Uninitialize();
		if (status != MH_OK) {
			logger->error("Unable to shutdown MinHook: {}", MH_StatusToString(status));
		}
	}

	Dll& Dll::GetInstance() {
		static Dll sInstance;
		return sInstance;
	}

	void* Dll::GetAddress(uint32_t vanillaAddress) const {
		if (!mImpl) {
			throw TempleException("Trying to get an address ({}) before the DLL has "
			                      "been loaded is not possible.", vanillaAddress);
		}

		return reinterpret_cast<void*>(vanillaAddress + mImpl->mDeltaFromVanilla);
	}

	void Dll::Load(const std::string& installationPath) {
		if (mImpl) {
			throw TempleException("DLL has already been loaded");
		}

		mImpl = std::make_shared<DllImpl>(installationPath);

		// Perform post-load actions
		ReplaceAllocFunctions();
		AddressRegistry::GetInstance().Fixup(mImpl->mDeltaFromVanilla);
	}

	void Dll::Unload() {
		mImpl.reset();
	}

	bool Dll::HasBeenRebased() {
		return mImpl->mDeltaFromVanilla != 0;
	}

	std::string Dll::FindConflictingModule() {
		HMODULE hMods[1024];
		DWORD cbNeeded;
		char moduleName[MAX_PATH];

		auto hProcess = GetCurrentProcess();

		std::string conflicting = "";

		const uint32_t templeImageSize = 0x01EB717E;
		const uint32_t templeDesiredStart = 0x10000000;
		const uint32_t templeDesiredEnd = templeDesiredStart + templeImageSize;

		if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
			for (uint32_t i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
				GetModuleFileNameA(hMods[i], moduleName, MAX_PATH);
				MODULEINFO moduleInfo;
				GetModuleInformation(hProcess, hMods[i], &moduleInfo, cbNeeded);
				auto fromAddress = reinterpret_cast<uint32_t>(moduleInfo.lpBaseOfDll);
				auto toAddress = fromAddress + moduleInfo.SizeOfImage;
				logger->debug(" Module {}: 0x{:08x}-0x{:08x}", moduleName, fromAddress, toAddress);

				if (fromAddress <= templeDesiredEnd && toAddress > templeDesiredStart) {
					conflicting = fmt::format("{} (0x{:08x}-0x{:08x})", moduleName, fromAddress, toAddress);
				}
			}
		}

		CloseHandle(hProcess);

		return conflicting;
	}

	void Dll::RegisterAddressPtr(void** ref) {
		// We manipulate the actual pointer value as-if it were a 32-bit integer,
		// so we treat the pointer to the function pointer like a pointer to an int
		// instead.
		auto* addressPtr = reinterpret_cast<uint32_t*>(ref);

		// Save the current pointer value
		auto vanillaAddress = *addressPtr;

		// Store a guard function in the pointer so when it is called before we actually
		// load the DLL, an error is raised
		*ref = &CalledTooEarlyGuard;

		AddressRegistry::GetInstance().Register(addressPtr, vanillaAddress);
	}

	/*
		Replaces memory allocation in temple.dll with the heap from this 
		module. This allows much safer exchange of data between the DLL and
		this module. In addition, memory profiling tools dont get so confused.
	*/
	void Dll::ReplaceAllocFunctions() const {

		MH_CreateHook(GetAddress(0x10254241), &realloc, nullptr);
		MH_CreateHook(GetAddress(0x10254B44), &calloc, nullptr);
		MH_CreateHook(GetAddress(0x1025444F), &malloc, nullptr);
		MH_CreateHook(GetAddress(0x10254209), &free, nullptr);
		MH_CreateHook(GetAddress(0x10256432), static_cast<void*(*)(size_t)>(&::operator new), nullptr);

	}

}
