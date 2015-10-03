
#include "gsl/gsl.h"
#include "infrastructure/keyboard.h"
#include "infrastructure/logging.h"
#include "platform/windows.h"

namespace infrastructure {

	Keyboard::Keyboard() : mKeyState(256, 0) {
		mScanCodeMap.reserve(256);
	}

	bool Keyboard::IsKeyPressed(int virtualKey) const {
		auto state = mKeyState[virtualKey];
		return (state & 0x80) != 0;
	}

	bool Keyboard::IsModifierActive(int virtualKey) const {
		auto state = mKeyState[virtualKey];
		return (state & 0x01) != 0;
	}

	void Keyboard::Update() {
		if (!GetKeyboardState(&mKeyState[0])) {
			logger->error("Unable to retrieve keyboard state: {}", 
				GetLastWin32Error());
		}
	}

	Keyboard gKeyboard;

}
