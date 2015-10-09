
#pragma once

#include "legacy.h"

class GameSystemLoadingScreen {
public:
	GameSystemLoadingScreen() : mes("mes\\loadscreen.mes") {
		if (!mes.valid()) {
			throw TempleException("loadscreen.mes not found");
		}

		sequence.messages = new const char*[gameSystemCount]; // 61 is the number of game systems
		sequence.image = ui.LoadImg("art\\splash\\legal0322.img");
		sequence.textColor = &graycolor;
		sequence.barBorderColor = &graycolor;
		sequence.barBackground = &darkblue;
		sequence.barForeground = &lightblue;
		sequence.offsetLeft = 0;
		sequence.offsetUp = 5;
		sequence.barWidth = 256;
		sequence.barHeight = 24;

		for (int i = 0; i < gameSystemCount; ++i) {
			const auto &system = legacyGameSystems->systems[i];

			if (system.loadscreenMesIdx) {
				auto idx = sequence.messagesCount++;
				sequence.messages[idx] = mes[system.loadscreenMesIdx];
			}
		}

		loadingScreenFuncs.Push(&sequence);
	}

	~GameSystemLoadingScreen() {
		loadingScreenFuncs.Pop();
	}

	void NextMessage() {
		loadingScreenFuncs.NextStep();
	}
private:
	MesFile mes;
	LoadingSequence sequence;
	RectColor darkblue = RectColor(0xFF1C324E);
	RectColor graycolor = RectColor(0xFF808080);
	RectColor lightblue = RectColor(0xFF1AC3FF);
};
