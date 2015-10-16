#include "stdafx.h"

#include <atlcomcli.h>
#include <platform/windows.h>

#include "installationdirpicker.h"
#include "installationdir.h"

// {42992164-25E1-4FF3-9FFD-1D65F21AD4DF}
static const GUID sDialogGuid =
{0x42992164, 0x25e1, 0x4ff3,{0x9f, 0xfd, 0x1d, 0x65, 0xf2, 0x1a, 0xd4, 0xdf}};

InstallationDir InstallationDirPicker::Pick(gsl::not_null<bool*> userCancelled) {
	*userCancelled = false;

	auto title = L"Temple of Elemental Evil - Installation Directory";
	MessageBox(nullptr,
		L"Sorry! TemplePlus could not find your Temple of Elemental Evil installation automatically.\n\n"
		L"Please select your Temple of Elemental Evil installation directory manually.",
		title,
		MB_OK | MB_ICONERROR);

	InstallationDirPicker dirPicker;
	do {
		if (!dirPicker.PickDirectory()) {
			*userCancelled = true;
			return InstallationDir();
		}

		InstallationDir pickedDir(dirPicker.GetDirectory());
		if (pickedDir.IsUsable()) {
			return pickedDir;
		}

		auto msg = fmt::format(L"Oops! The selected directory '{}' cannot be used by TemplePlus for the following reasons:\n{}",
			pickedDir.GetDirectory(),
			pickedDir.GetNotUsableReason());
		MessageBox(nullptr, msg.c_str(), title, MB_OK | MB_ICONERROR);
	} while (true);
}

bool InstallationDirPicker::PickDirectory() {

	ComInitializer com;

	CComPtr<IFileOpenDialog> dlg;
	auto result = dlg.CoCreateInstance(CLSID_FileOpenDialog);
	if (!SUCCEEDED(result)) {
		logger->error("Unable to create the folder picker dialog");
		return false;
	}

	// Set the option to pick folders instead of files
	DWORD options;
	if (SUCCEEDED(dlg->GetOptions(&options))) {
		dlg->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_PICKFOLDERS);
	}

	// Set a GUID so this dialog remembers it state separately
	dlg->SetClientGuid(sDialogGuid);
	dlg->SetTitle(L"Please select your Temple of Elemental Evil Installation Directory");

	// Show the folder selection dialog
	result = dlg->Show(nullptr);

	if (!(SUCCEEDED(result))) {
		return false; // User cancelled
	}

	// Extract the filename from the dialog
	CComPtr<IShellItem> resultItem;
	if (!SUCCEEDED(dlg->GetResult(&resultItem))) {
		logger->error("Unable to get result item from open folder dialog");
		return false;
	}

	LPWSTR pathName;
	if (!SUCCEEDED(resultItem->GetDisplayName(SIGDN_FILESYSPATH, &pathName))) {
		logger->error("Unable to get the selected path");
		return false;
	}

	mDirectory = pathName;
	CoTaskMemFree(pathName);
	return true;
}
