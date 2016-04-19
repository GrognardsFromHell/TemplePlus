#include "stdafx.h";
#include "ui_intgame_select.h";
#include <temple/dll.h>
#include <util/fixes.h>
#include <common.h>
#include "ui_render.h"
#include "ui_tooltip.h"
#include "ui.h"
#include <gamesystems/gamesystems.h>
#include <gamesystems/objects/objsystem.h>
#include <critter.h>
#include <party.h>
#include <config/config.h>

UiIntgameSelect uiIntgameSelect;

struct UiIntgameSelectAddresses : temple::AddressTable
{
	int * id;
	UiIntgameSelectAddresses()
	{
		rebase(id, 0x10BE5B90);
	}
} addresses;

class UiIntgameSelectHooks : public TempleFix
{
public: 
	static void TooltipRender(objHndl handle);

	void apply() override 
	{
		replaceFunction(0x10138E20, TooltipRender);
	}
} intgameSelHooks;

void UiIntgameSelectHooks::TooltipRender(objHndl handle)
{
	if (!handle)
		return ;

	//auto ttLen = tooltipText.size();

	//// trim last \n
	//if (ttLen > 0 && tooltipText[ttLen - 1] == '\n')
	//	tooltipText[ttLen - 1] = 0;


	auto ttStyle = tooltips.GetStyle(0);
	UiRenderer::PushFont(ttStyle.fontName, ttStyle.fontSize);
	
	TigTextStyle ttTextStyle;
	ColorRect textColor[3] = { ColorRect(-1),ColorRect(-1) ,ColorRect(0xFF3333FF) };
	ColorRect shadowColor(0xFF000000);
	ColorRect bgColor(0x99111111);
	ttTextStyle.kerning = 2;
	ttTextStyle.tracking = 2;
	ttTextStyle.flags = 0xC08;
	ttTextStyle.field2c = -1;
	ttTextStyle.colors4 = nullptr;
	ttTextStyle.colors2 = nullptr;
	ttTextStyle.field0 = 0;
	ttTextStyle.leading = 0;
	ttTextStyle.textColor = textColor;
	ttTextStyle.shadowColor = &shadowColor;
	ttTextStyle.bgColor = &bgColor;

	// modify text color based on injuries
	auto obj = gameSystems->GetObj().GetObject(handle);
	if (obj->IsCritter()){
		auto subdualDam = obj->GetInt32(obj_f_critter_subdual_damage);

		if (obj->IsPC()){
			auto curHp = objects.GetHPCur(handle);
			if (curHp < objects.StatLevelGet(handle, stat_hp_max)){
				ttTextStyle.textColor[1] = ColorRect(0xFFFF0000);
			}
		} 
		else
		{
			if ( critterSys.IsDeadOrUnconscious(handle)){
				ttTextStyle.textColor[1] = ColorRect(0xFF7F7F7F);
			} else
			{
				uint32_t newTextColor = 0;
				auto getInjuryLevel = temple::GetRef<int(__cdecl)(objHndl)>(0x10123A80);
				switch (getInjuryLevel(handle))
				{
				case 0:
					newTextColor = 0xFF00FF00;
					break;
				case 1:
					newTextColor = 0xFF7FFF00;
					break;
				case 2:
					newTextColor = 0xFFFFFF00;
					break;
				case 3:
					newTextColor = 0xFFFF7F00;
					break;
				case 4:
					newTextColor = 0xFFFF0000;
					break;
				default:
					break;
				}
				ttTextStyle.textColor[1] = ColorRect(newTextColor);
			}
		}
	}

	auto observer = party.GetConsciousPartyLeader();
	auto getTooltipDesc = temple::GetRef<const char*(__cdecl)(objHndl, objHndl)>(0x101247A0);
	auto tooltipDesc = getTooltipDesc(observer, handle);
	if (!tooltipDesc)
		return;

	auto ttLen = strlen(tooltipDesc);
	if (ttLen > 10000 || ttLen <=0 ){
		return; // if it is so, probably a bug
	}
		
	std::string tooltipText(tooltipDesc);



	TigRect extents = UiRenderer::MeasureTextSize(tooltipText, ttTextStyle);
	auto getObjectRect = temple::GetRef<void(__cdecl)(objHndl, int, TigRect&)>(0x10022BC0);
	TigRect objRect;
	getObjectRect(handle, 0, objRect);

	auto screenH = config.renderHeight;
	auto screenW = config.windowWidth;

	if (objRect.x < screenW && objRect.y < screenH
		&& objRect.width + objRect.x > 0 && objRect.y + objRect.height > 0)
	{

		extents.x = objRect.x + (objRect.width - extents.width) / 2;
		if (extents.x < 3){
			extents.x = 3;
		}

		extents.y = objRect.y - extents.height;
		if (objRect.y - extents.height < 3){
			extents.y = 3;
		}

		if (extents.width + extents.x > screenW - 3)
			extents.x = screenW - extents.width - 3;
		
		if (extents.height + extents.y > screenH - 3)
		{
			extents.y = screenH - extents.height - 3;
		}
		UiRenderer::RenderText(tooltipText, extents, ttTextStyle);
	}


	UiRenderer::PopFont();
}

int UiIntgameSelect::GetId()
{
	return *addresses.id;
}
