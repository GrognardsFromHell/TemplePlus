
#include "stdafx.h"
#include <VersionHelpers.h>

wstring GetScreenshotFolder() {
	HRESULT hr;
	IKnownFolderManager *mgr = NULL;

	if (!IsWindowsVistaOrGreater()) {
		return L"";
	}

	hr = CoCreateInstance(CLSID_KnownFolderManager, NULL,
		CLSCTX_INPROC_SERVER, IID_IKnownFolderManager,
		reinterpret_cast<void **>(&mgr));

	if (!SUCCEEDED(hr)) {
		return L"";
	}
	
	IKnownFolder *folder = nullptr;
	if (!SUCCEEDED(mgr->GetFolder(FOLDERID_Screenshots, &folder))) {
		mgr->Release();
		return L"";
	}

	LPWSTR folderPath;
	if (!SUCCEEDED(folder->GetPath(KF_FLAG_CREATE | KF_FLAG_INIT, &folderPath))) {
		folder->Release();
		mgr->Release();
		return L"";
	}

	wstring result(folderPath);

	CoTaskMemFree(folderPath);
	folder->Release();
	mgr->Release();

	return result;
}
