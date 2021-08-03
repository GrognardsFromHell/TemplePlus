#pragma once
#include <obj.h>
#include "ui.h"
#include "ui_system.h"
//#include "../dialog.h"

struct UiSystemConf;
class WidgetContainer;
struct TigMsg;
class UiDialogImpl;
struct DialogState;

class UiDlg : public UiSystem, public SaveGameAwareUiSystem {
public:
	static constexpr auto Name = "Dlg-UI";
	UiDlg(const UiSystemConf &config);
	~UiDlg();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(const UiSaveFile &saveGame) override;
	void ResizeViewport(const UiResizeArgs &resizeArgs) override;
	const std::string &GetName() const override;

	bool IsActive() const;
	bool IsDialogEngaged() const;


	void Hide();

	// this seems to be mostly internal for the python based picker
	DialogState *GetCurrentDialog();
	

	void ReShowDialog(DialogState *info, int line);
	void Unk();

	/*
		Show an ingame popover with the portrait of the speaker and the given text message.
		Can be used outside of dialog too.
		The NPC also turns towards the target it is speaking to.
		The voice sample with the id given in speechId is played back if it is not -1.
	*/
	void ShowTextBubble(objHndl speaker, objHndl speakingTo, const std::string &text, int speechId = -1);

	void ShowHistory(); // TODO

	std::unique_ptr< UiDialogImpl> mImpl;
};

extern UiDlg *uiDialog;