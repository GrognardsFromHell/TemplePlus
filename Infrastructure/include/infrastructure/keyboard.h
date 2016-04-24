#pragma once

#include <vector>

namespace infrastructure {


	/*
		Class for managing keyboard input
	*/
	class Keyboard {
	public:

		Keyboard();

		bool IsKeyPressed(int virtualKey) const;

		bool IsModifierActive(int virtualKey) const;

		void Update();

		int ToVirtualKey(int dinputkey) const;
		
	private:
		std::vector<uint8_t> mKeyState;
		std::vector<uint32_t> mScanCodeMap;
	};

	extern Keyboard gKeyboard;

}
