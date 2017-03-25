
#pragma once

#include "ui_system.h"

struct objHndl;
struct TigMsg;
struct UiSystemConf;

class UiInGame : public UiSystem, public SaveGameAwareUiSystem {
public:
	static constexpr auto Name = "Intgame";
	UiInGame(const UiSystemConf &config);
	~UiInGame();
	void Reset() override;
	bool LoadGame(const UiSaveFile &saveGame) override;
	void ResizeViewport(const UiResizeArgs &resizeArgs) override;
	const std::string &GetName() const override;

	// Unclear what this does exactly...
	void ClearGroupArray();
	void AddToGroupArray(objHndl handle);
	void ResetFocusObject();
	void SetFocusObject(objHndl handle);

	void ProcessMessage(const TigMsg &msg);

	bool HandleRadialMenuMessage(const TigMsg &msg);
	bool HandlePickerMessage(const TigMsg &msg);
	
	/**
	 * Is the object not targetable by mouse?
	 * Was @ 1001FCB0
	 */
	bool IsUntargetable(objHndl obj);

	/**
	 * Seems to reset any input focus the ingame UI might have. Used
	 * i.e. when dialog is initiated, etc.
	 */
	void ResetInput();


	/**
	* Selects the topmost object under the given screen coordinate.
	*/
	objHndl PickObject(int x, int y, uint32_t flags); // flags - see RaycastFlags

private:
	bool mPrevMsgWasInCombat = false;
	bool mPartyMembersMoving = false;

	void HandleCombatMessage(const TigMsg &msg);
	void HandleNonCombatMessage(const TigMsg &msg);
	void HandleCombatKeyStateChange(const TigMsg &msg);
	void HandleCombatMouseEvent(const TigMsg &msg);
	
	objHndl GetMouseTarget(int x, int y);
	
	

};
