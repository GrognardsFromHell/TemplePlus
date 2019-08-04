
#include <gsl/gsl>

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

	int Keyboard::ToVirtualKey(int dinputKey) const {
		switch (dinputKey) {
		case 0x01: // DIK_ESCAPE
			return VK_ESCAPE;
		case 0x02: // DIK_1
			return '1';
		case 0x03: // DIK_2
			return '2';
		case 0x04: // DIK_3
			return '3';
		case 0x05: // DIK_4
			return '4';
		case 0x06: // DIK_5
			return '5';
		case 0x07: // DIK_6
			return '6';
		case 0x08: // DIK_7
			return '7';
		case 0x09: // DIK_8
			return '8';
		case 0x0A: // DIK_9
			return '9';
		case 0x0B: // DIK_0
			return '0';
		case 0x0C: // DIK_MINUS/* - on main keyboard */
			return VK_OEM_MINUS;
		case 0x0D: // DIK_EQUALS
			return VK_OEM_PLUS;
		case 0x0E: // DIK_BACK/* backspace */
			return VK_BACK;
		case 0x0F: // DIK_TAB
			return VK_TAB;
		case 0x10: // DIK_Q
			return 'Q';
		case 0x11: // DIK_W
			return 'W';
		case 0x12: // DIK_E
			return 'E';
		case 0x13: // DIK_R
			return 'R';
		case 0x14: // DIK_T
			return 'T';
		case 0x15: // DIK_Y
			return 'Y';
		case 0x16: // DIK_U
			return 'U';
		case 0x17: // DIK_I
			return 'I';
		case 0x18: // DIK_O
			return 'O';
		case 0x19: // DIK_P
			return 'P';
		case 0x1A: // DIK_LBRACKET
			return VK_OEM_4;
		case 0x1B: // DIK_RBRACKET
			return VK_OEM_6;
		case 0x1C: // DIK_RETURN/* Enter on main keyboard */
			return VK_RETURN;
		case 0x1D: // DIK_LCONTROL
			return VK_LCONTROL;
		case 0x1E: // DIK_A
			return 'A';
		case 0x1F: // DIK_S
			return 'S';
		case 0x20: // DIK_D
			return 'D';
		case 0x21: // DIK_F
			return 'F';
		case 0x22: // DIK_G
			return 'G';
		case 0x23: // DIK_H
			return 'H';
		case 0x24: // DIK_J
			return 'J';
		case 0x25: // DIK_K
			return 'K';
		case 0x26: // DIK_L
			return 'L';
		case 0x27: // DIK_SEMICOLON
			return VK_OEM_1;
		case 0x28: // DIK_APOSTROPHE
			return VK_OEM_7;
		case 0x29: // DIK_GRAVE/* accent grave */
			return VK_OEM_3;
		case 0x2A: // DIK_LSHIFT
			return VK_LSHIFT;
		case 0x2B: // DIK_BACKSLASH
			return VK_OEM_5;
		case 0x2C: // DIK_Z
			return 'Z';
		case 0x2D: // DIK_X
			return 'X';
		case 0x2E: // DIK_C
			return 'C';
		case 0x2F: // DIK_V
			return 'V';
		case 0x30: // DIK_B
			return 'B';
		case 0x31: // DIK_N
			return 'N';
		case 0x32: // DIK_M
			return 'M';
		case 0x33: // DIK_COMMA
			return VK_OEM_COMMA;
		case 0x34: // DIK_PERIOD/* . on main keyboard */
			return VK_OEM_PERIOD;
		case 0x35: // DIK_SLASH/* / on main keyboard */
			return VK_OEM_2;
		case 0x36: // DIK_RSHIFT
			return VK_RSHIFT;
		case 0x37: // DIK_MULTIPLY/* * on numeric keypad */
			return VK_MULTIPLY;
		case 0x38: // DIK_LMENU/* left Alt */
			return VK_LMENU;
		case 0x39: // DIK_SPACE
			return VK_SPACE;
		case 0x3A: // DIK_CAPITAL
			return VK_CAPITAL;
		case 0x3B: // DIK_F1
			return VK_F1;
		case 0x3C: // DIK_F2
			return VK_F2;
		case 0x3D: // DIK_F3
			return VK_F3;
		case 0x3E: // DIK_F4
			return VK_F4;
		case 0x3F: // DIK_F5
			return VK_F5;
		case 0x40: // DIK_F6
			return VK_F6;
		case 0x41: // DIK_F7
			return VK_F7;
		case 0x42: // DIK_F8
			return VK_F8;
		case 0x43: // DIK_F9
			return VK_F9;
		case 0x44: // DIK_F10
			return VK_F10;
		case 0x45: // DIK_NUMLOCK
			return VK_NUMLOCK;
		case 0x46: // DIK_SCROLL/* Scroll Lock */
			return VK_SCROLL;
		case 0x47: // DIK_NUMPAD7
			return VK_NUMPAD7;
		case 0x48: // DIK_NUMPAD8
			return VK_NUMPAD8;
		case 0x49: // DIK_NUMPAD9
			return VK_NUMPAD9;
		case 0x4A: // DIK_SUBTRACT/* - on numeric keypad */
			return VK_SUBTRACT;
		case 0x4B: // DIK_NUMPAD4
			return VK_NUMPAD4;
		case 0x4C: // DIK_NUMPAD5
			return VK_NUMPAD5;
		case 0x4D: // DIK_NUMPAD6
			return VK_NUMPAD6;
		case 0x4E: // DIK_ADD/* + on numeric keypad */
			return VK_ADD;
		case 0x4F: // DIK_NUMPAD1
			return VK_NUMPAD1;
		case 0x50: // DIK_NUMPAD2
			return VK_NUMPAD2;
		case 0x51: // DIK_NUMPAD3
			return VK_NUMPAD3;
		case 0x52: // DIK_NUMPAD0
			return VK_NUMPAD0;
		case 0x53: // DIK_DECIMAL/* . on numeric keypad */
			return VK_DECIMAL;
		case 0x57: // DIK_F11
			return VK_F11;
		case 0x58: // DIK_F12
			return VK_F12;
		case 0x64: // DIK_F13/*                     (NEC PC98) */
			return VK_F13;
		case 0x65: // DIK_F14/*                     (NEC PC98) */
			return VK_F14;
		case 0x66: // DIK_F15/*                     (NEC PC98) */
			return VK_F15;
		case 0x9C: // DIK_NUMPADENTER/* Enter on numeric keypad */
			return VK_RETURN;
		case 0x9D: // DIK_RCONTROL
			return VK_RCONTROL;
		case 0xB5: // DIK_DIVIDE/* / on numeric keypad */
			return VK_DIVIDE;
		case 0xB8: // DIK_RMENU/* right Alt */
			return VK_RMENU;
		case 0xC7: // DIK_HOME/* Home on arrow keypad */
			return VK_HOME;
		case 0xC8: // DIK_UP/* UpArrow on arrow keypad */
			return VK_UP;
		case 0xC9: // DIK_PRIOR/* PgUp on arrow keypad */
			return VK_PRIOR;
		case 0xCB: // DIK_LEFT/* LeftArrow on arrow keypad */
			return VK_LEFT;
		case 0xCD: // DIK_RIGHT/* RightArrow on arrow keypad */
			return VK_RIGHT;
		case 0xCF: // DIK_END/* End on arrow keypad */
			return VK_END;
		case 0xD0: // DIK_DOWN/* DownArrow on arrow keypad */
			return VK_DOWN;
		case 0xD1: // DIK_NEXT/* PgDn on arrow keypad */
			return VK_NEXT;
		case 0xD2: // DIK_INSERT/* Insert on arrow keypad */
			return VK_INSERT;
		case 0xD3: // DIK_DELETE/* Delete on arrow keypad */
			return VK_DELETE;
		case 0xDB: // DIK_LWIN/* Left Windows key */
			return VK_LWIN;
		case 0xDC: // DIK_RWIN/* Right Windows key */
			return VK_RWIN;
		case 0xDD: // DIK_APPS/* AppMenu key */
			return VK_APPS;
		case 0xC5: // DIK_PAUSE
			return VK_PAUSE;
		case 0xB7: // DIK_SYSRQ (print screen)
			return VK_SNAPSHOT;
		default:
			//throw TempleException("Unmappable direct input key: {}", dinputKey);
			return 0;
		}
	}

	Keyboard gKeyboard;

}
