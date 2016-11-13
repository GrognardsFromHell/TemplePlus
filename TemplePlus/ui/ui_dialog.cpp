
#include "stdafx.h"
#include "ui_dialog.h"
#include <temple/dll.h>
#include "ui.h"

UiDialog uiDialog;

static struct UiDialogAddresses : temple::AddressTable {
	
	void(__cdecl *Hide)();
	bool(__cdecl *IsActive)();
	DialogState* (__cdecl *GetCurrentDlg)();
	void(__cdecl *ReShowDialog)(DialogState *info, int line);
	void (__cdecl *Unk)(); // This is speech related
	int (__cdecl *ShowTextBubble)(objHndl pc, objHndl speakingTo, const char *text, int speechId);
	
	UiDialogAddresses() {
		rebase(Hide, 0x1014CA20);
		rebase(IsActive, 0x1014BFE0);
		rebase(GetCurrentDlg, 0x1014BA30);
		rebase(ReShowDialog, 0x100388D0);
		rebase(Unk, 0x1014C8F0);
		rebase(ShowTextBubble, 0x1014CDE0);
	}
} addresses;


void UiDialog::Hide() {
	addresses.Hide();
}

bool UiDialog::IsActive() {
	return addresses.IsActive();
}

DialogState* UiDialog::GetCurrentDialog() {
	return addresses.GetCurrentDlg();
}

void UiDialog::ReShowDialog(DialogState* info, int line) {
	addresses.ReShowDialog(info, line);
}

void UiDialog::Unk() {
	addresses.Unk();
}

void UiDialog::ShowTextBubble(objHndl speaker, objHndl speakingTo, const string &text, int speechId) {
	addresses.ShowTextBubble(speaker, speakingTo, text.c_str(), speechId);
}

BOOL UiDialog::WidgetsInit(int w, int h)
{
	static WidgetType1 dlgWnd(9, h - 374, 611, 292);
	dlgWnd.widgetFlags = 1;
	dlgWnd.render = [](int widId) { uiDialog.WndRender(widId); };
	dlgWnd.handleMessage = [](int widId, TigMsg* msg) { return uiDialog.WndMsg(widId, msg); };
	if (dlgWnd.Add(&wndId))
		return 0;

	// scrollbar
	scrollbar.Init(592, 28, 126, wndId);
	scrollbar.Add(&scrollbarId);
	ui.BindToParent(wndId, scrollbarId);


	int coloff = 0, rowoff = 0;

	//for (auto it : d20ClassSys.vanillaClassEnums) {
	//	// class buttons
	//	int newId = 0;
	//	WidgetType2 classBtn("Class btn", wndId, 71 + coloff, 47 + rowoff, 130, 20);
	//	coloff = 139 - coloff;
	//	if (!coloff)
	//		rowoff += 29;
	//	if (rowoff == 5 * 29) // the bottom button
	//		coloff = 69;

	//	classBtnRects.push_back(TigRect(classBtn.x, classBtn.y, classBtn.width, classBtn.height));
	//	classBtn.x += dlgWnd.x; classBtn.y += dlgWnd.y;
	//	classBtn.render = [](int id) {uiCharEditor.ClassBtnRender(id); };
	//	classBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.ClassBtnMsg(id, msg); };
	//	classBtn.Add(&newId);
	//	classBtnIds.push_back(newId);
	//	ui.SetDefaultSounds(newId);
	//	ui.BindToParent(wndId, newId);

	//	//rects
	//	classBtnFrameRects.push_back(TigRect(classBtn.x - 5, classBtn.y - 5, classBtn.width + 10, classBtn.height + 10));


	//	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	//	auto classMeasure = UiRenderer::MeasureTextSize(classNamesUppercase[it].c_str(), classBtnTextStyle);
	//	TigRect rect(classBtn.x + (110 - classMeasure.width) / 2 - dlgWnd.x,
	//		classBtn.y + (20 - classMeasure.height) / 2 - dlgWnd.y,
	//		classMeasure.width, classMeasure.height);
	//	classTextRects.push_back(rect);
	//	UiRenderer::PopFont();
	//}

	//classNextBtnTextRect = classNextBtnRect = TigRect(dlgWnd.x + 293, dlgWnd.y + 234, 55, 20);
	//classPrevBtnTextRect = classPrevBtnRect = TigRect(dlgWnd.x + 58, dlgWnd.y + 234, 55, 20);
	//classNextBtnFrameRect = TigRect(dlgWnd.x + 293 - 3, dlgWnd.y + 234 - 5, 55 + 6, 20 + 10);
	//classPrevBtnFrameRect = TigRect(dlgWnd.x + 58 - 3, dlgWnd.y + 234 - 5, 55 + 6, 20 + 10);
	//classNextBtnTextRect.x -= dlgWnd.x; classNextBtnTextRect.y -= dlgWnd.y;
	//classPrevBtnTextRect.x -= dlgWnd.x; classPrevBtnTextRect.y -= dlgWnd.y;

	//WidgetType2 nextBtn("Class Next Button", wndId, dlgWnd.x + 293, dlgWnd.y + 230, 55, 20),
	//	prevBtn("Class Prev. Button", wndId, dlgWnd.x + 58, dlgWnd.y + 230, 55, 20);

	//nextBtn.handleMessage = [](int widId, TigMsg*msg)->BOOL {
	//	if (uiCharEditor.classWndPage < uiCharEditor.mPageCount)
	//		uiCharEditor.classWndPage++;
	//	uiCharEditor.ClassSetPermissibles();
	//	return 1; };
	//nextBtn.render = [](int id) { uiCharEditor.ClassNextBtnRender(id); };
	//nextBtn.handleMessage = [](int widId, TigMsg*msg)->BOOL {	return uiCharEditor.ClassNextBtnMsg(widId, msg); };
	//prevBtn.render = [](int id) { uiCharEditor.ClassPrevBtnRender(id); };
	//prevBtn.handleMessage = [](int widId, TigMsg*msg)->BOOL {	return uiCharEditor.ClassPrevBtnMsg(widId, msg); };
	//nextBtn.Add(&classNextBtn);	prevBtn.Add(&classPrevBtn);

	//ui.SetDefaultSounds(classNextBtn);	ui.BindToParent(wndId, classNextBtn);
	//ui.SetDefaultSounds(classPrevBtn);	ui.BindToParent(wndId, classPrevBtn);

	return TRUE;
	return 0;
}

BOOL UiDialog::ResponseWidgetsInit(int w, int h)
{
	return 0;
}

BOOL UiDialog::WndMsg(int widId, TigMsg * msg)
{
	return 0;
}

void UiDialog::WndRender(int widId)
{
}
