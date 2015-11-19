
#include <platform/windows.h>
#include <infrastructure/exception.h>

#include "temple/soundsystem.h"

namespace temple {

	struct SoundSystem::Impl {

		/**
		 * Apparently gets the resources currently used by the miles sound system.
		 */
		using FNGetQuickHandles = void __stdcall (SoundDriverHandle *driverOut, 
			void** midiOut, 
			void**deviceOut);

		FNGetQuickHandles *GetQuickHandles;

		Impl() {
			Resolve("_AIL_quick_handles@12", GetQuickHandles);
		}
	private:
		template <typename T>
		void Resolve(const char* entryPoint, T* & funcPtr) {
			auto binkHandle = GetModuleHandle(L"mss32");
			if (!binkHandle) {
				throw TempleException("mss32.dll is not loaded");
			}

			funcPtr = (T*)GetProcAddress(binkHandle, entryPoint);

			if (!funcPtr) {
				throw TempleException("Entry point {} was not found in mss32.dll",
					entryPoint);
			}
		}
	};

	SoundSystem::SoundSystem() : mImpl(std::make_unique<Impl>()) {
	}

	SoundSystem::~SoundSystem() {
	}

	SoundDriverHandle SoundSystem::GetSoundDriverHandle() {

		SoundDriverHandle handle;
		mImpl->GetQuickHandles(&handle, nullptr, nullptr);
		return handle;

	}
}
