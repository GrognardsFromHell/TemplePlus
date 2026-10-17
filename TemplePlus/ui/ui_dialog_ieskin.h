#pragma once
/*
	Infinity Engine style dialogue window ("dialog skins") for the Temple+ dialog UI.

	A skin is data only: art/interface/dialog_ie/<skin>/skin.json plus the TGA files it names. The ie2toee converter
	produces both on the player's machine from their own Infinity Engine install, so nothing derived from IE assets
	is shipped with Temple+. art/interface/dialog_ie/skins.json selects which conversations use a skin:

		{ "default": "<skin or empty>",                         module-wide skin
		  "dialogs": { "<dlg id>": "<skin>", ... },             per conversation (ToEE dialog file / script id)
		  "portraits": { "<dlg id>": "<tga path>", ... } }      speaker portraits (else the NPC's ToEE portrait)

	Two layouts are supported (skin.json "layout"):
		"panel"  IWD:EE / BG:EE world dialog: framed panel at the bottom, portrait, NPC text, reply list below it,
		         log-history toggle that opens the conversation log above the panel, Continue / End Dialogue button.
		"log"    classic IE expanded message window: one text area holding the conversation log, the current NPC
		         line and the numbered replies underneath.
	Without skins.json (or for conversations it doesn't list) the stock ToEE dialog window is used unchanged.
*/
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include "tig/tig.h"
#include "obj.h"

struct IeDlgView {
	objHndl npc, pc;
	int dialogId = 0;
	std::string npcText;
	int replyCount = 0;
	std::string replies[5];
	std::vector<std::pair<bool, std::string>> history; // newest first; first = is PC line
	bool historyMode = false;
	bool active = false; // conversation running (false: post-conversation history view)
	int screenW = 1024, screenH = 768;
};

struct IeDlgLayout {
	TigRect wnd;           // main (render/mouse) window
	TigRect responseWnd;   // reply window: must contain every reply button
	TigRect replies[5];    // reply button rects (absolute); height 0 = hidden
	TigRect headBtn;       // log-history toggle
	bool continueMode = false; // single text-less reply shown as the Continue / End Dialogue button
	bool continueIsEnd = false;
	bool historyMode = false;
	int panelX = 0, panelY = 0, panelW = 0, panelH = 0;
};

class IeDialogSkins {
public:
	// Picks the skin for the given conversation (cached). Returns true when a skin applies.
	bool Select(int dialogId);
	bool IsActive() const { return mCur >= 0; }
	IeDialogSkins();
	~IeDialogSkins();
	void Deactivate();
	const std::string &CurrentName() const;

	IeDlgLayout Layout(const IeDlgView &view);
	void RenderMain(const IeDlgView &view, const IeDlgLayout &layout);
	void RenderReply(const IeDlgView &view, const IeDlgLayout &layout, int idx, int btnState);
	void RenderHistoryButton(const IeDlgLayout &layout, int btnState);
	void Scroll(int lines) { mHistoryScroll = std::max(0, mHistoryScroll + lines); }
	void ResetScroll() { mHistoryScroll = 0; }

	struct Skin;
private:
	void LoadSelection();
	int LoadSkin(const std::string &name);
	std::string PortraitFor(const IeDlgView &view);

	bool mSelectionLoaded = false;
	std::string mDefault;
	std::vector<std::pair<int, std::string>> mByDialog, mPortraits;
	std::vector<std::unique_ptr<Skin>> mSkins;
	int mCur = -1;
	int mCurDialog = -1;
	int mHistoryScroll = 0;
	std::unique_ptr<Skin> mScaled; // current skin scaled to the screen
	int mScaledKey = -1;
	int mPortraitDialog = -1, mPortraitTex = 0, mPortraitW = 0, mPortraitH = 0;
};

extern IeDialogSkins ieDialogSkins;
