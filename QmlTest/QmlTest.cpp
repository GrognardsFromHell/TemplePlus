// QmlTest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

int main(int argc, char* argv[])
{
	std::vector<std::string> argsFake{
		argv[0],
		"-platform",
		"offscreen"
	};
	std::vector<char*> argvFake;
	for (auto &str : argsFake) {
		argvFake.push_back(&str[0]);
	}
	int argcFake = argvFake.size();

	QGuiApplication guiApp(argcFake, &argvFake[0]);

	QQuickWindow::setSceneGraphBackend();

	return guiApp.exec();
}
