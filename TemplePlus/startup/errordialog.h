
#pragma once

#include <memory>
#include <string>

class ErrorDialog {
public:
	ErrorDialog();
	~ErrorDialog();

	void SetMessage(const std::wstring &text);
	void Show();
private:
	std::unique_ptr<class ErrorDialogImpl> mImpl;
};
