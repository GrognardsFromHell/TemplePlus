#include "stdafx.h"
#include "ui_party.h"
#include <util/fixes.h>


UiParty uiParty;

class UiPartyHooks : public TempleFix {
public:


	void apply() override {

		// hook the buff/debuff icon so it doesn't try to get a spell name if the spellEnum is -2
		static char* (__cdecl* orgBuffDebuffGetBaseText)(BuffDebuffPacket*, int, int) = 
			replaceFunction<char*(__cdecl)(BuffDebuffPacket* , int , int )>(0x100F45A0, [](BuffDebuffPacket* bdb, int iconType, int iconIdx)->char*{
			if (iconType == 0){ // buff
				auto spEnum = bdb->buffs[iconIdx ].spellEnum;
				if (spEnum == -2){
					return "";
				}
			}
			return orgBuffDebuffGetBaseText(bdb, iconType, iconIdx);
		});
		
		/*static void (__cdecl*orgBdbTooltip)(int, int, int*) = replaceFunction<void(__cdecl)(int, int, int*)>(0x10131EA0, [](int x, int y, int* widId)
		{
			orgBdbTooltip(x, y, widId);
		});*/

	}
} uiPartyHooks;

