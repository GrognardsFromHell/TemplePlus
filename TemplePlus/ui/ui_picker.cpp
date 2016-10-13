#include "stdafx.h"
#include "ui_picker.h"
#include <maps.h>
#include <temple_functions.h>
#include <tig/tig_loadingscreen.h>
#include "ui_intgame_select.h"

UiPicker uiPicker;

static struct PickerAddresses : temple::AddressTable {
	bool(__cdecl *ShowPicker)(const PickerArgs &args, void *callbackArgs);
	bool(__cdecl *FreeCurrentPicker)();
	uint32_t(__cdecl * sub_100BA030)(objHndl, PickerArgs*);
	int * activePickerIdx; 
	BOOL(__cdecl* PickerActiveCheck)();

	PickerAddresses() {
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
	void apply() override{
		static BOOL(__cdecl *orgConfigSpellTargeting)(PickerArgs&, SpellPacketBody&) = replaceFunction<BOOL(__cdecl)(PickerArgs &, SpellPacketBody &)>(0x100B9690, [](PickerArgs &args, SpellPacketBody &spPkt)	{

			//return orgConfigSpellTargeting(args, spPkt);
			

			if (args.IsBaseModeTarget(UiPickerType::Single)){
				spPkt.targetCount = 1;
				spPkt.orgTargetCount = 1;
				spPkt.targetListHandles[0] = args.result.handle;
			} else
			{
				spPkt.targetCount = 0;
				spPkt.orgTargetCount = 0;
			}

			if (args.IsBaseModeTarget(UiPickerType::Multi) ) {


				auto objNode = args.result.objList.objects;

				if (!objNode) {
					if (args.result.handle){
						auto N = 1;
						if (!args.IsModeTargetFlagSet(UiPickerType::OnceMulti)) {
							N = MAX_SPELL_TARGETS;
						}
						for (spPkt.targetCount = 0; spPkt.targetCount < args.maxTargets && spPkt.targetCount < N; ++spPkt.targetCount) {
							spPkt.targetListHandles[spPkt.targetCount] = args.result.handle;
						}
					}
				}
				else{
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
				

				
			}

			if (args.IsBaseModeTarget(UiPickerType::Area)){
				spPkt.aoeCenter.location = args.result.location;
				spPkt.aoeCenter.off_z = args.result.offsetz;
			}
			else {
				spPkt.aoeCenter.location.location.locx = 0;
				spPkt.aoeCenter.location.location.locy = 0;
				spPkt.aoeCenter.location.off_x = 0;
				spPkt.aoeCenter.location.off_y = 0;
				spPkt.aoeCenter.off_z = 0;
			}

			if (args.IsBaseModeTarget(UiPickerType::Ray)){
				auto objNode = args.result.objList.objects;
				auto handle = objHndl::null;

				spPkt.aoeCenter.location = args.result.location;
				spPkt.aoeCenter.off_z = args.result.offsetz;

				if (!objNode) {
					if (args.result.handle) {
						spPkt.targetListHandles[spPkt.targetCount] = args.result.handle;	
					}
				}
				else{
					for (spPkt.targetCount = 0; objNode && spPkt.targetCount < MAX_SPELL_TARGETS; ++spPkt.targetCount) {
						spPkt.targetListHandles[spPkt.targetCount] = objNode->handle;
						objNode = objNode->next;
					}
				}
			}

			return TRUE;
		});
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

uint32_t UiPicker::sub_100BA480(objHndl objHnd, PickerArgs* pickerArgs)
{
	return addresses.sub_100BA480(objHnd, pickerArgs);
}

void UiPicker::sub_100BA6A0(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs)
{
	addresses.sub_100BA6A0(locAndOffsets, pickerArgs);
}

uint32_t UiPicker::sub_100BA540(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs)
{
	return addresses.sub_100BA540(locAndOffsets, pickerArgs);
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
