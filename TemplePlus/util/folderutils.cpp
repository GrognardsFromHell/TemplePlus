
#include "stdafx.h"
#include <VersionHelpers.h>

static IKnownFolderManager* GetFolderManager() {
	static CComPtr<IKnownFolderManager> mgr;
	
	if (!mgr) {
		auto hr = CoCreateInstance(CLSID_KnownFolderManager, nullptr,
			CLSCTX_INPROC_SERVER, IID_IKnownFolderManager,
			reinterpret_cast<void **>(&mgr));

		if (!SUCCEEDED(hr)) {
			throw TempleException("Couldn't create the known folder manager. "
				"Launching from before Vista?");
		}
	}

	return mgr;

}

static wstring GetKnownFolder(const KNOWNFOLDERID &folderId) {
	
	auto mgr = GetFolderManager();

	CComPtr<IKnownFolder> folder;
	if (!SUCCEEDED(mgr->GetFolder(folderId, &folder))) {
		wchar_t guid[MAX_PATH];
		StringFromGUID2(folderId, guid, MAX_PATH);
		logger->error("Unable to get known folder {}", ucs2_to_utf8(guid));
		return L"";
	}

	LPWSTR folderPath;
	if (!SUCCEEDED(folder->GetPath(KF_FLAG_CREATE | KF_FLAG_INIT, &folderPath))) {
		wchar_t guid[MAX_PATH];
		StringFromGUID2(folderId, guid, MAX_PATH);
		logger->error("Unable to get folder path for {}", ucs2_to_utf8(guid));
		return L"";
	}

	wstring result(folderPath);
	CoTaskMemFree(folderPath);
	return result;
}

static wstring GetKnownSubFolder(const KNOWNFOLDERID &folderId, const wchar_t *subFolder) {

	auto folder = GetKnownFolder(folderId);

	auto fullPath = fmt::format(L"{}\\{}", folder, subFolder);

	if (!PathIsDirectory(fullPath.c_str())) {
		if (!CreateDirectoryW(fullPath.c_str(), nullptr)) {
			logger->error("Unable to create directory: {}", ucs2_to_utf8(fullPath));
		}
	}

	return fullPath;

}

wstring GetScreenshotFolder() {
	static auto sScreenshotFolder = GetKnownFolder(FOLDERID_SavedGames);
	return sScreenshotFolder;
}

wstring GetUserDataFolder() {
	static auto sUserDataFolder = GetKnownSubFolder(FOLDERID_SavedGames, L"TemplePlus\\");

	return sUserDataFolder;
}
