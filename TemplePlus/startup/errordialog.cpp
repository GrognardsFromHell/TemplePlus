#define WINVER 0x0600
#pragma warning( push )

// Disable WTL warnings
#pragma warning(disable: 4005)
#pragma warning(disable: 4302)
#pragma warning(disable: 4838)

#include <windows.h>
#include <atlbase.h>
#include <atlapp.h> 

#pragma warning( pop )

#include "errordialog.h"

CAppModule _Module;

class ErrorDialogWnd : public CWindowImpl<ErrorDialogWnd> {
public:
	DECLARE_WND_CLASS(_T("ErrorDialogWndCls"))

	BEGIN_MSG_MAP(ErrorDialogWnd)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
	END_MSG_MAP()
	
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		PostQuitMessage(0);
		bHandled = FALSE;
		return 0;
	}
	
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		PostQuitMessage(0);
		bHandled = FALSE;
		return 0;
	}
};

class ErrorDialogImpl {
public:
	void SetMessage(const std::wstring& message);
	void Show();
private:
	std::wstring mMessage;
};

void ErrorDialogImpl::SetMessage(const std::wstring& message) {
	mMessage = message;
}

void ErrorDialogImpl::Show() {

	CMessageLoop msgLoop;
	ErrorDialogWnd dialogWnd;

	_Module.Init(NULL, GetModuleHandle(NULL));
	_Module.AddMessageLoop(&msgLoop);
	
	dialogWnd.Create(nullptr, CWindow::rcDefault, _T("Temple Plus Error"),
		WS_OVERLAPPEDWINDOW);
	dialogWnd.ShowWindow(WM_SHOWWINDOW);
	dialogWnd.UpdateWindow();

	msgLoop.Run();

	_Module.RemoveMessageLoop();
	_Module.Term();

}

ErrorDialog::ErrorDialog() : mImpl(std::make_unique<ErrorDialogImpl>()) {
}

ErrorDialog::~ErrorDialog() {
}

void ErrorDialog::SetMessage(const std::wstring& text) {
	mImpl->SetMessage(text);
}

void ErrorDialog::Show() {
	mImpl->Show();
}
