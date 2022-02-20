#include "stdafx.h"
#include "ui_logbook.h"
#include <temple/dll.h>
#include <obj.h>
#include <gamesystems/objects/objsystem.h>
#include <gamesystems/objects/gameobject.h>
#include <party.h>
#include <tig/tig_msg.h>
#include <tig/tig_keyboard.h>
#include <ui/ui.h>


UiLogbook uiLogbook;


class UiLogbookHooks : public TempleFix
{
	static BOOL KeyEntryWndMsg(int, TigMsg*); // support pressing enter to confirm
	void apply() override {
		
		writeAddress(0x10197245 + 7, &KeyEntryWndMsg);
	}
} uiLogbookHooks;

void UiLogbook::IncreaseAmount(PartyLogbookPacket & pkt, objHndl handle, int amount){

	if (pkt.HasObj(handle) || pkt.AddObj(handle)){
		for (auto i=0; i < LOGBOOK_MAX_PARTY_MEMBER_COUNT; i++){
			if (objSystem->GetHandleById(pkt.sub[i].id) == handle){
				pkt.sub[i].amount += amount;
				break;
			}
		}
	}
}

void UiLogbook::IncreaseCritHits(objHndl handle){
	if (!handle)
		return;

	if (objects.GetType(handle) == obj_t_pc && party.IsInParty(handle)){
		IncreaseAmount( temple::GetRef<PartyLogbookPacket>(0x10D24878), handle, 1);
	}
}

void UiLogbook::IncreaseHits(objHndl handle)
{
	temple::GetRef<void(__cdecl)(objHndl)>(0x1009A9B0)(handle);
}

void UiLogbook::IncreaseMisses(objHndl handle)
{
	temple::GetRef<void(__cdecl)(objHndl)>(0x1009A9D0)(handle);
}

void UiLogbook::RecordHighestDamage(BOOL isWeaponDamage, int damageAmt, objHndl attacker, objHndl tgt)
{
	static auto uiBridgeRecordHighestDam = temple::GetRef<void(__cdecl)(BOOL, int, objHndl, objHndl)>(0x1009AA10);
	uiBridgeRecordHighestDam(isWeaponDamage, damageAmt, attacker, tgt);
}

void UiLogbook::MarkKey(int keyId, const GameTime& gameTime){
	temple::GetRef<void(__cdecl)(int, GameTime)>(0x1009A7B0)(keyId, gameTime);
}

void UiLogbook::KeyEntryNeedToNotifySet(bool setValue)
{
	temple::GetRef<int>(0x102FEDE0) = setValue ? 1 : 0;
}

BOOL PartyLogbookPacket::HasObj(objHndl handle){
	for (auto i=0; i < LOGBOOK_MAX_PARTY_MEMBER_COUNT; i++){
		ObjectId id = this->sub[i].id;
	
		if (objSystem->GetHandleById(id) == handle)
			return TRUE;
	}

	return FALSE;
}

BOOL PartyLogbookPacket::AddObj(objHndl handle){
	return temple::GetRef<BOOL(__cdecl)(PartyLogbookPacket *, objHndl)>(0x10198C40)(this, handle);
}

BOOL UiLogbookHooks::KeyEntryWndMsg(int id, TigMsg* msg)
{
	if (msg->type == TigMsgType::CHAR) {
		auto charMsg = (TigCharMsg*)msg;

		if (charMsg->charCode == VK_RETURN) {
			uiManager->SetHidden(id, true);
			return TRUE;
		}
	}
	return TRUE;
}
