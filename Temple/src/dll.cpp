#include <platform/windows.h>
#include <Shlwapi.h>
#include <MinHook.h>
#include <psapi.h>

#include <infrastructure/exception.h>
#include <infrastructure/logging.h>
#include <infrastructure/stringutil.h>

#include "temple/dll.h"

namespace temple {

	constexpr uint32_t defaultBaseAddr = 0x10000000;
	constexpr uint32_t defaultImageSize = 0x1EB717E;

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
		explicit DllImpl(const std::wstring& installationDir);
		~DllImpl();

		void* GetAddress(uint32_t vanillaAddress) const;
		void ReplaceAllocFunctions() const;
		void ReplaceDebugFunctions() const;
		void SetDebugOutputCallback(std::function<void(const std::string&)> function);
		int mDeltaFromVanilla = 0;
		HINSTANCE mDllHandle = nullptr;


		std::function<void(const std::string& text)> mDebugOutputCallback;

	private:
		// This is injected into ToEE
		static void DebugMessageFormat(const char* format, ...);
		static void DebugMessage(const char* message);
	};

	DllImpl::DllImpl(const std::wstring& installationDir) {

		auto dllPath(installationDir + L"temple.dll");

		// Does it even exist?
		if (!PathFileExists(dllPath.c_str())) {
			auto msg(fmt::format("Temple.dll does not exist: {}", ucs2_to_utf8(dllPath)));
			throw TempleException(msg);
		}

		SetCurrentDirectory(installationDir.c_str());

		// Try to load it
		mDllHandle = LoadLibrary(dllPath.c_str());
		if (!mDllHandle) {
			throw TempleException("Unable to load temple.dll from {}: {}",
			                      ucs2_to_utf8(dllPath), GetLastWin32Error());
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
		auto status = MH_Uninitialize();
		if (status != MH_OK) {
			logger->error("Unable to shutdown MinHook: {}", MH_StatusToString(status));
		}

		if (mDllHandle) {
			if (!FreeLibrary(mDllHandle)) {
				logger->error("Unable to free the temple.dll library handle: {}",
				              GetLastWin32Error());
			}
		}

	}

	void* DllImpl::GetAddress(uint32_t vanillaAddress) const {
		return reinterpret_cast<void*>(vanillaAddress + mDeltaFromVanilla);
	}

	Dll& Dll::GetInstance() {
		static Dll sInstance;
		return sInstance;
	}

	Dll::~Dll() {
		if (mReservedMem) {
			VirtualFree(mReservedMem, 0, MEM_RELEASE);
		}
	}

	void* Dll::GetAddress(uint32_t vanillaAddress) const {
		if (!mImpl) {
			throw TempleException("Trying to get an address ({}) before the DLL has "
			                      "been loaded is not possible.", vanillaAddress);
		}

		return mImpl->GetAddress(vanillaAddress);
	}

	void Dll::Load(const std::wstring& installationPath) {
		if (mImpl) {
			throw TempleException("DLL has already been loaded");
		}

		// Free the reserved memory
		if (mReservedMem) {
			VirtualFree(mReservedMem, 0, MEM_RELEASE);
			mReservedMem = nullptr;
		}

		mImpl = std::make_shared<DllImpl>(installationPath);

		// Perform post-load actions
		mImpl->ReplaceAllocFunctions();
		mImpl->ReplaceDebugFunctions();
		AddressRegistry::GetInstance().Fixup(mImpl->mDeltaFromVanilla);
	}

	void Dll::Unload() {
		mImpl.reset();
	}

	bool Dll::HasBeenRebased() {
		return mImpl->mDeltaFromVanilla != 0;
	}

	std::wstring Dll::FindConflictingModule() {
		HMODULE hMods[1024];
		DWORD cbNeeded;
		TCHAR moduleName[MAX_PATH];

		auto hProcess = GetCurrentProcess();

		std::wstring conflicting;

		const uint32_t templeImageSize = 0x01EB717E;
		const uint32_t templeDesiredStart = 0x10000000;
		const uint32_t templeDesiredEnd = templeDesiredStart + templeImageSize;

		if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
			for (uint32_t i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
				GetModuleFileName(hMods[i], moduleName, MAX_PATH);
				MODULEINFO moduleInfo;
				GetModuleInformation(hProcess, hMods[i], &moduleInfo, cbNeeded);
				auto fromAddress = reinterpret_cast<uint32_t>(moduleInfo.lpBaseOfDll);
				auto toAddress = fromAddress + moduleInfo.SizeOfImage;
				logger->debug(" Module {}: 0x{:08x}-0x{:08x}", ucs2_to_utf8(moduleName), fromAddress, toAddress);

				if (fromAddress <= templeDesiredEnd && toAddress > templeDesiredStart) {
					conflicting = fmt::format(L"{} (0x{:08x}-0x{:08x})", moduleName, fromAddress, toAddress);
				}
			}
		}

		CloseHandle(hProcess);

		return conflicting;
	}

	void Dll::ReserveMemoryRange() {
		if (mReservedMem) {
			throw TempleException("Memory has already been reserved.");
		}
		if (mImpl) {
			throw TempleException("DLL has already been loaded.");
		}

		mReservedMem = VirtualAlloc(reinterpret_cast<void*>(defaultBaseAddr),
		                            defaultImageSize,
		                            MEM_RESERVE,
		                            PAGE_NOACCESS);

	}

	bool Dll::IsVanillaDll() const {
		// One of the differences between vanilla and GoG is the
		// condition table referenced @ 100F7BC0
		void** addrPtr = temple::GetPointer<void*>(0x100F7BC0 + 2);
		return *addrPtr == temple::GetPointer(0x102EEC40);

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

	void Dll::SetDebugOutputCallback(std::function<void(const std::string& text)> callback) {
		if (!mImpl) {
			throw TempleException("DLL has not been loaded.");
		}

		mImpl->SetDebugOutputCallback(callback);
	}

	/*
		Replaces memory allocation in temple.dll with the heap from this 
		module. This allows much safer exchange of data between the DLL and
		this module. In addition, memory profiling tools dont get so confused.
	*/
	void DllImpl::ReplaceAllocFunctions() const {
		MH_CreateHook(GetAddress(0x10254241), &realloc, nullptr);
		MH_CreateHook(GetAddress(0x10254B44), &calloc, nullptr);
		MH_CreateHook(GetAddress(0x1025444F), &malloc, nullptr);
		MH_CreateHook(GetAddress(0x10254209), &free, nullptr);
		MH_CreateHook(GetAddress(0x10256432), static_cast<void*(*)(size_t)>(&::operator new), nullptr);
	}

	void DllImpl::ReplaceDebugFunctions() const {
		MH_CreateHook(temple::GetPointer<0x101E48F0>(), DebugMessageFormat, nullptr);
		MH_CreateHook(temple::GetPointer<0x101E4940>(), DebugMessage, nullptr);
	}

	void DllImpl::SetDebugOutputCallback(std::function<void(const std::string&)> function) {
		mDebugOutputCallback = function;
	}

	void DllImpl::DebugMessageFormat(const char* format, ...) {
		va_list args;
		va_start(args, format);

		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), format, args);

		DebugMessage(buffer);
	}

	void DllImpl::DebugMessage(const char* message) {
		auto callback = Dll::GetInstance().mImpl->mDebugOutputCallback;
		if (callback) {
			callback(message);
		}
	}

}
