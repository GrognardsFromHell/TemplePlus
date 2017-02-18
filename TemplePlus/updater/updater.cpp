#include <stdafx.h>

#include <thread>

#include <QFileInfo>
#include <QDir>

#include "updater.h"
#include "config/config.h"

#include <platform/windows.h>

#include <infrastructure/json11.hpp>

struct Updater::Impl {
	~Impl();

	// Full path to Update.exe or empty if it doesnt exist.
	std::wstring updaterPath;

	// Mutex for communicating between the two threads
	std::mutex mutex;

	// The background process that checks/downloads an update
	std::thread thread;

	// Search for Update.exe
	void FindUpdater();

	void StartUpdate();

	// Handles used for IPC with the Update.exe process
	HANDLE childOutRead = nullptr;
	HANDLE childOutWrite = nullptr;

	std::string status;

	// Safely update the status string for the end user
	template<typename... TArgs>
	void UpdateStatus(const char *fmt, const TArgs&... args) {		
		std::lock_guard<std::mutex> guard(mutex);
		status = fmt::format(fmt, args...);
		logger->info("New updater status: {}", status);
	}

private:
	void RunUpdate();

	std::string RunUpdater(const std::vector<std::wstring> &args);
};

Updater::Impl::~Impl() {
	if (thread.joinable()) {
		thread.join();
	}
}

void Updater::Impl::FindUpdater() {
	wchar_t ownFilename[MAX_PATH];
	GetModuleFileNameW(nullptr, &ownFilename[0], MAX_PATH);

	QFileInfo self(QString::fromWCharArray(ownFilename));
	QDir baseDir(self.dir());
	baseDir.cdUp();

	QString updateExe = baseDir.filePath("Update.exe");

	// Update.exe is one folder up from our own binary in the directory layout
	// used by squirrel
	if (QFile::exists(updateExe)) {
		updaterPath = updateExe.toStdWString();
		logger->info("Update.exe found at {}", updateExe.toStdString());
	} else {
		logger->info("No Update.exe found at {}", updateExe.toStdString());
	}

}

void Updater::Impl::StartUpdate() {

	if (!config.autoUpdate) {
		UpdateStatus("Auto update is disabled in the configuration");
		return;
	}

	thread = std::thread([=] {
		UpdateStatus("Checking for updates...");
		this->RunUpdate();
	});

}

void Updater::Impl::RunUpdate() {

	std::wstring feedUrl = utf8_to_ucs2(config.autoUpdateFeed);

	UpdateStatus("Downloading updates...");

	std::wstring downloadArg = fmt::format(L"--download={}", feedUrl);
	auto downloadResult = RunUpdater({ downloadArg });

	if (downloadResult.empty()) {
		UpdateStatus("Running the latest version");
		return;
	}

	// Find the last line
	auto lastLineStart = downloadResult.rfind('\n', downloadResult.size() - 2);
	if (lastLineStart != std::string::npos) {
		downloadResult = downloadResult.substr(lastLineStart + 1);
	}

	logger->debug("Result from download process: {}", downloadResult);

	std::string jsonError;
	auto downloadObj = json11::Json::parse(downloadResult, jsonError);
	if (!jsonError.empty()) {
		logger->error("Unable to parse response from update server: {}", jsonError);
		UpdateStatus("Could not check for updates");
		return;
	}

	auto currentVersion = downloadObj["currentVersion"].string_value();
	auto futureVersion = downloadObj["futureVersion"].string_value();

	if (currentVersion == futureVersion) {
		UpdateStatus("Running the latest version");
		return;
	}

	UpdateStatus("Installing update to {}", futureVersion);
	logger->info("Update available: {} -> {}", currentVersion, futureVersion);
	
	std::wstring updateArg = fmt::format(L"--update={}", feedUrl);
	auto updateResult = RunUpdater({ updateArg });
	logger->info("Result from update: {}", updateResult);

	UpdateStatus("Restart your game to update to {}", futureVersion);

}

std::string Updater::Impl::RunUpdater(const std::vector<std::wstring>& args) {

	std::wstring commandLine = updaterPath;
	
	for (auto &arg : args) {
		commandLine.append(L" ");
		if (arg.find(L' ') != std::wstring::npos) {
			// TODO This is an incorrect way of quoting...
			// https://blogs.msdn.microsoft.com/twistylittlepassagesallalike/2011/04/23/everyone-quotes-command-line-arguments-the-wrong-way/
			commandLine.append(L"\"");
			commandLine.append(arg);
			commandLine.append(L"\"");
		} else {
			commandLine.append(arg);
		}	
	}

	// Make it a pointer for the Win32 API
	wchar_t* commandLinePtr = nullptr;
	if (!commandLine.empty()) {
		commandLinePtr = &commandLine[0];
	}

	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&childOutRead, &childOutWrite, &saAttr, 0)) {
		throw TempleException("Unable to create pipe for communicating with update.exe: {}",
			GetLastWin32Error());
	}

	if (!SetHandleInformation(childOutRead, HANDLE_FLAG_INHERIT, 0)) {
		throw TempleException("Unable to change inheritance settings of stdout read handle: {}",
			GetLastWin32Error());
	}
	
	// This is similar to https://msdn.microsoft.com/en-us/library/ms682499(VS.85).aspx
	STARTUPINFOW startupInfo;
	memset(&startupInfo, 0, sizeof(startupInfo));
	startupInfo.cb = sizeof(STARTUPINFOW);
	startupInfo.hStdError = childOutWrite;
	startupInfo.hStdOutput = childOutWrite;
	startupInfo.dwFlags |= STARTF_USESTDHANDLES;

	PROCESS_INFORMATION updaterPi;

	auto result = CreateProcessW(
		updaterPath.c_str(),
		commandLinePtr,
		nullptr,
		nullptr,
		TRUE,
		0,
		nullptr,
		nullptr,
		&startupInfo,
		&updaterPi
		);

	if (!result) {
		// Failed to start updater process
		throw TempleException("Unable to start Update.exe: {}", GetLastWin32Error());
	}

	CloseHandle(updaterPi.hProcess);
	CloseHandle(updaterPi.hThread);
	CloseHandle(childOutWrite);

	DWORD bytesRead;
	char buf[4096];
	std::string stdoutBuffer;

	BOOL success;
	do {
		success = ReadFile(childOutRead, buf, sizeof(buf), &bytesRead, nullptr);

		// Process the buffer we've read. The output has to be 
		// processed line-by-line
		if (success) {
			stdoutBuffer.append(buf, bytesRead);
		}	
	} while (success && bytesRead != 0);

	return stdoutBuffer;

}

Updater::Updater() : mImpl(new Impl) {

	mImpl->FindUpdater();

	if (IsSupported()) {
		mImpl->StartUpdate();
	}

}

Updater::~Updater() {
}

bool Updater::IsSupported() const {
	return !mImpl->updaterPath.empty();
}

std::string Updater::GetStatus() const {
	std::lock_guard<std::mutex> lockGuard(mImpl->mutex);
	return mImpl->status;
}
