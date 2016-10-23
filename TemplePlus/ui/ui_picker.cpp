#include "stdafx.h"
#include "ui_picker.h"
#include <maps.h>
#include <temple_functions.h>
#include <tig/tig_loadingscreen.h>
#include "ui_intgame_select.h"
#include <tig/tig_msg.h>
#include <tig/tig_keyboard.h>
#include <party.h>
#include <gamesystems/objects/objsystem.h>
#include <critter.h>

UiPicker uiPicker;

static struct PickerAddresses : temple::AddressTable {
	bool(__cdecl *ShowPicker)(const PickerArgs &args, void *callbackArgs);
	bool(__cdecl *FreeCurrentPicker)();
	uint32_t(__cdecl * sub_100BA030)(objHndl, PickerArgs*);
	int * activePickerIdx; 
	BOOL(__cdecl* PickerActiveCheck)();
	PickerSpec *pickerSpecs; // size 9 array containign specs for: None, Single, Multi, Cone, Area, Location, Personal, Inventory Item, Ray (corresponding to  UiPickerType )

	PickerAddresses() {
		rebase(pickerSpecs, 0x102F9158);

		rebase(ShowPicker, 0x101357E0);
		rebase(PickerActiveCheck, 0x10135970);
		rebase(FreeCurrentPicker, 0x10137680);
		
		rebase(activePickerIdx, 0x102F920C);
		macRebase(sub_100BA030, 100BA030)
		macRebase(sub_100BA480, 100BA480)
		macRebase(sub_100BA540, 100BA540) // for area range type
		macRebase(sub_100BA6A0, 100BA6A0) // something with conical selection
	}

	uint32_t (__cdecl *sub_100BA540)(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
	void (__cdecl *sub_100BA6A0)(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
	uint32_t (__cdecl * sub_100BA480)(objHndl objHnd, PickerArgs* pickerArgs);
} addresses;



class UiPickerHooks : TempleFix
{
	static BOOL PickerMultiKeystateChange(TigMsg *msg);
	void apply() override{

		// Picker Multi Keystate change - support pressing Enter key
		replaceFunction(0x10137DA0, PickerMultiKeystateChange);

		// Config Spell Targeting - added fix to allow AI to cast multiple projectiles with Magic Missile and the like on the selected target
		static BOOL(__cdecl *orgConfigSpellTargeting)(PickerArgs&, SpellPacketBody&) = replaceFunction<BOOL(__cdecl)(PickerArgs &, SpellPacketBody &)>(0x100B9690, [](PickerArgs &args, SpellPacketBody &spPkt)	{

			//return orgConfigSpellTargeting(args, spPkt);
			auto flags = (PickerResultFlags)args.result.flags;
			
			if (flags & PRF_HAS_SINGLE_OBJ){
				spPkt.targetCount = 1;
				spPkt.orgTargetCount = 1;
				spPkt.targetListHandles[0] = args.result.handle;

				// add for the benefit of AI casters
				if (args.IsBaseModeTarget(UiPickerType::Multi) && args.result.handle) {
					auto N = 1;
					if (!args.IsModeTargetFlagSet(UiPickerType::OnceMulti)) {
						N = MAX_SPELL_TARGETS;
					}
					for ( ; spPkt.targetCount < args.maxTargets && spPkt.targetCount < N; spPkt.targetCount++) {
						spPkt.targetListHandles[spPkt.targetCount] = args.result.handle;
					}
				}
			} 
			else	{
				spPkt.targetCount = 0;
				spPkt.orgTargetCount = 0;
			}

			if (flags & PRF_HAS_MULTI_OBJ){

				auto objNode = args.result.objList.objects;

				for (spPkt.targetCount = 0; objNode; ++spPkt.targetCount) {
					if (spPkt.targetCount >= 32)
						break;

					spPkt.targetListHandles[spPkt.targetCount] = objNode->handle;

					if (objNode->next)
						objNode = objNode->next;
					// else apply the rest of the targeting to the last object
					else if (!args.IsModeTargetFlagSet(UiPickerType::OnceMulti)) {
						while (spPkt.targetCount < args.maxTargets) {
							spPkt.targetListHandles[spPkt.targetCount++] = objNode->handle;
						}
						objNode = nullptr;
						break;
					}
				}
			}
			
			if (flags & PRF_HAS_LOCATION){
				spPkt.aoeCenter.location = args.result.location;
				spPkt.aoeCenter.off_z = args.result.offsetz;
			} 
			else
			{
				spPkt.aoeCenter.location.location.locx = 0;
				spPkt.aoeCenter.location.location.locy = 0;
				spPkt.aoeCenter.location.off_x = 0;
				spPkt.aoeCenter.location.off_y = 0;
				spPkt.aoeCenter.off_z = 0;
			}

			if (flags & PRF_UNK8){
				logger->debug("ui_picker: not implemented - BECAME_TOUCH_ATTACK");
			}
			
			return TRUE;
		});
	
	
		static void(__cdecl*orgRenderPickers)() = replaceFunction<void()>(0x101350F0, [](){
			uiPicker.RenderPickers();
			//return orgRenderPickers();
		});

		/*static void(__cdecl*orgRenderRay)(LocAndOffsets, LocAndOffsets, float, float, float, int) = replaceFunction<void(LocAndOffsets, LocAndOffsets, float, float, float, int)>(0x10108340, [](LocAndOffsets srcLoc, LocAndOffsets tgtLoc, float rayWidth, float minRange, float maxRange, int spellEnum)
		{
			return orgRenderRay(srcLoc, tgtLoc, rayWidth, minRange, maxRange, spellEnum);
		});*/
	}
} uiPickerHooks;

BOOL UiPicker::PickerActiveCheck()
{
	return (*addresses.activePickerIdx) >= 0;
}

int UiPicker::ShowPicker(const PickerArgs& args, void* callbackArgs) {
	//if (maps.GetCurrentMapId() == 5118 ) // tutorial map
	//{
	//	if (args.spellEnum == 288)
	//	{
	//		
	//	} else if (args.spellEnum == 171)
	//	{
	//		
	//	}
	//}
	//if (*addresses.activePickerIdx >= 32)
	//	return 0;
	//ui.WidgetSetHidden(uiIntgameSelect.GetId(), 0);
	//(*addresses.activePickerIdx)++;
	//if (*addresses.activePickerIdx >= 32 || *addresses.activePickerIdx < 0)
	//	return 0;
	//
	
	return addresses.ShowPicker(args, callbackArgs);
}

uint32_t UiPicker::sub_100BA030(objHndl objHnd, PickerArgs* pickerArgs)
{
	return addresses.sub_100BA030(objHnd, pickerArgs);
}

void UiPicker::FreeCurrentPicker() {
	addresses.FreeCurrentPicker();
}

PickerCacheEntry & UiPicker::GetPicker(int pickerIdx){
	return temple::GetRef<PickerCacheEntry[32]>(0x10BE3490)[pickerIdx];
}

uint32_t UiPicker::SetSingleTarget(objHndl objHnd, PickerArgs* pickerArgs)
{
	return addresses.sub_100BA480(objHnd, pickerArgs);
}

void UiPicker::SetConeTargets(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs)
{
	addresses.sub_100BA6A0(locAndOffsets, pickerArgs);
}

uint32_t UiPicker::GetListRange(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs)
{
	return addresses.sub_100BA540(locAndOffsets, pickerArgs);
}

void UiPicker::RenderPickers(){
	
	auto &activePickerIdx = temple::GetRef<int>(0x102F920C);
	auto intgameSelTextDraw = temple::GetRef<void(__cdecl)()>(0x10108FA0);
	auto &pickerStatusFlags = temple::GetRef<PickerStatusFlags>(0x10BE5F2C);

	auto drawSpellPlayerPointer = true;
	auto tgtCount = 0;

	// Get the picker and the originator
	if (activePickerIdx < 0 || activePickerIdx >= MAX_PICKER_COUNT){
		intgameSelTextDraw();
		return;
	}

	auto pick = GetPicker(activePickerIdx);

	auto originator = pick.args.caster;
	if (!originator){
		originator = party.GetConsciousPartyLeader();
		if (!originator)
			return;
	}
	
	auto tgt = pick.tgt;
	if (tgt){
		// renders the circle for the current hovered target (using an appropriate shader based on ok/not ok selection)
		if (pickerStatusFlags & PickerStatusFlags::PSF_Invalid)
			temple::GetRef<void(objHndl, objHndl, int)>(0x10109980)(tgt, originator, pick.args.spellEnum); // render Invalid circle
		else
			temple::GetRef<void(objHndl, objHndl, int)>(0x10109940)(tgt, originator, pick.args.spellEnum); // render OK circle
	}

	// Draw rotating circles for selected targets
	if (pick.args.result.flags & PickerResultFlags::PRF_HAS_SINGLE_OBJ){
		auto handle = pick.args.result.handle;
		if (pick.args.result.handle == originator){
			drawSpellPlayerPointer = false;
		}

		if (pick.args.IsBaseModeTarget(UiPickerType::Multi)){
			temple::GetRef<void(objHndl, objHndl, int)>(0x10109940)(handle, originator, pick.args.spellEnum);
			temple::GetRef<void(objHndl, int)>(0x10108ED0)(handle, 1); // text append
		}
	}

	if (pick.args.result.flags & PickerResultFlags::PRF_HAS_MULTI_OBJ) {
		auto objNode = pick.args.result.objList.objects;
		while (objNode){
			
			auto handle = objNode->handle;
			if (handle == originator)
				drawSpellPlayerPointer = false;

			auto handleObj = objSystem->GetObject(handle);
			auto fogFlags = temple::GetRef<uint8_t(__cdecl)(LocAndOffsets)>(0x1002ECB0)(handleObj->GetLocationFull()) ;

			if (!critterSys.IsConcealed(handle) && (fogFlags & 1) ){ // fixed rendering for hidden critters
				temple::GetRef<void(objHndl, objHndl, int)>(0x10109940)(handle, originator, pick.args.spellEnum);
				temple::GetRef<void(objHndl, int)>(0x10108ED0)(handle, ++tgtCount); // text append	
			}
			
			objNode = objNode->next;
		}
	}

	// Draw the Spell Player Pointer
	if (drawSpellPlayerPointer){
		if (tgt){
			if (tgt != originator){
				auto tgtLoc = objSystem->GetObject(tgt)->GetLocationFull();
				temple::GetRef<void(__cdecl)(objHndl, LocAndOffsets&)>(0x10106D10)(originator, tgtLoc); // draw the picker arrow from the originator to the target
			}
		}
		else // draw the picker arrow from the originator to the mouse position
		{
			LocAndOffsets tgtLoc;
			locSys.GetLocFromScreenLocPrecise(pick.x, pick.y, &tgtLoc.location, &tgtLoc.off_x, &tgtLoc.off_y);
			temple::GetRef<void(__cdecl)(objHndl, LocAndOffsets&)>(0x10106D10)(originator, tgtLoc);
		}
	}

	auto origObj = objSystem->GetObject(originator);
	auto originiLoc = origObj->GetLocationFull();
	auto originRadius = objects.GetRadius(originator);

	tgt = pick.tgt; //just in case it got updated
	auto tgtObj = objSystem->GetObject(tgt);

	// Area targeting
	if (pick.args.IsBaseModeTarget(UiPickerType::Area)){
		LocAndOffsets tgtLoc;
		if (tgt){
			tgtLoc = tgtObj->GetLocationFull();
		} 
		else{
			locSys.GetLocFromScreenLocPrecise(pick.x, pick.y, &tgtLoc.location, &tgtLoc.off_x, &tgtLoc.off_y);
		}

		float orgAbsX, orgAbsY, tgtAbsX, tgtAbsY;
		locSys.GetOverallOffset(originiLoc, &orgAbsX, &orgAbsY);
		locSys.GetOverallOffset(tgtLoc, &tgtAbsX, &tgtAbsY);

		auto areaRadiusInch = INCH_PER_FEET * pick.args.radiusTarget;

		// Draw the big AoE circle
		temple::GetRef<void(__cdecl)(LocAndOffsets, float, float, int)>(0x10107610)(tgtLoc, 1.0, areaRadiusInch, pick.args.spellEnum);


		// Draw Spell Effect pointer (points from AoE to caster)
		auto spellEffectPointerSize = areaRadiusInch / 80.0f * 38.885002f;
		if (spellEffectPointerSize <= 135.744f){
			if (spellEffectPointerSize < 11.312f)
				spellEffectPointerSize = 11.312f;
		} else
		{
			spellEffectPointerSize = 135.744f;
		}

		if (originRadius * 1.5f + areaRadiusInch  + spellEffectPointerSize < locSys.distBtwnLocAndOffs(tgtLoc, originiLoc)){
			temple::GetRef<void(__cdecl)(LocAndOffsets, LocAndOffsets, float)>(0x101068A0)(tgtLoc, originiLoc, areaRadiusInch);
		}	
	}

	else if (pick.args.IsBaseModeTarget(UiPickerType::Personal)){
		if (tgt && (pick.args.flagsTarget &UiPickerFlagsTarget::Radius) && tgt != originator){

			temple::GetRef<void(__cdecl)(LocAndOffsets, float, float, int)>(0x10107610)(originiLoc, 1.0, INCH_PER_FEET * pick.args.radiusTarget, pick.args.spellEnum);

		}
	}

	else if (pick.args.IsBaseModeTarget(UiPickerType::Cone)) {
		LocAndOffsets tgtLoc;
		if (tgt) {
			tgtLoc = tgtObj->GetLocationFull();
		}
		else {
			locSys.GetLocFromScreenLocPrecise(pick.x, pick.y, &tgtLoc.location, &tgtLoc.off_x, &tgtLoc.off_y);
		}

		if (pick.args.flagsTarget & UiPickerFlagsTarget::FixedRadius){
			tgtLoc = locSys.TrimToLength(originiLoc, tgtLoc, pick.args.radiusTarget * INCH_PER_FEET);
			
		}

		auto degreesTarget = pick.args.degreesTarget;
		if (!pick.args.flagsTarget & UiPickerFlagsTarget::Degrees){
			degreesTarget = 60.0f;
		}

		temple::GetRef<void(__cdecl)(LocAndOffsets, LocAndOffsets, float, int)>(0x10107920)(originiLoc, tgtLoc, degreesTarget, pick.args.spellEnum);
	}

	else if (pick.args.IsBaseModeTarget(UiPickerType::Ray)){
		if (pick.args.flagsTarget & UiPickerFlagsTarget::Range){
			LocAndOffsets tgtLoc;
			locSys.GetLocFromScreenLocPrecise(pick.x, pick.y, &tgtLoc.location, &tgtLoc.off_x, &tgtLoc.off_y);

			auto rayWidth = pick.args.radiusTarget * INCH_PER_FEET / 2.0f;
			auto rayLength = originRadius + pick.args.trimmedRangeInches;

			temple::GetRef<void(__cdecl)(LocAndOffsets, LocAndOffsets, float, float, float, int)>(0x10108340)(originiLoc, tgtLoc, rayWidth, rayLength, rayLength, pick.args.spellEnum);
		}
	}

	intgameSelTextDraw();
	return;
}

bool PickerArgs::IsBaseModeTarget(UiPickerType type)
{
	auto _type = (uint64_t)type;
	return ( ((uint64_t)modeTarget) & 0xFF) == _type;
}

bool PickerArgs::IsModeTargetFlagSet(UiPickerType type)
{
	return (((uint64_t)modeTarget) & ((uint64_t)type)) == (uint64_t)type;
}

BOOL UiPickerHooks::PickerMultiKeystateChange(TigMsg * msg){
	if (msg->type != TigMsgType::KEYSTATECHANGE)
		return FALSE;

	if (msg->arg2 & 0xFF)
		return FALSE;

	if (msg->arg1 != DIK_SPACE && msg->arg1 != DIK_RETURN)
		return 0;

	return temple::GetRef<BOOL(__cdecl)(TigMsg*)>(0x10136810)(msg);
}
