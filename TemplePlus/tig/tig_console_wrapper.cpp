
#include "stdafx.h"

#include <util/fixes.h>
#include "tig_startup.h"
#include "tig_console.h"

static class ConsoleWrapper : public TempleFix {
public:

	void apply() override {
		// tig_console_append
		replaceFunction<void(const char *)>(0x101dfdc0, [](const char *line) {
			tig->GetConsole().Append(line);
		});

		// tig_console_show
		replaceFunction<void()>(0x101df7c0, []() {
			tig->GetConsole().Show();
		});

		// tig_console_hide
		replaceFunction<void()>(0x101df7f0, []() {
			tig->GetConsole().Hide();
		});

		// console_set_cheats
		replaceFunction<void(Cheat*, int)>(0x101df690, [](Cheat* cheats, int cheatCount) {
			tig->GetConsole().SetCheats(cheats, cheatCount);
		});

	}

} consoleWrapper;
