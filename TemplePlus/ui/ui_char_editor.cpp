#include "stdafx.h"
#include "common.h"
#include "feat.h"
#include "ui_char_editor.h"
#include "obj.h"
#include "ui/ui.h"
#include "util/fixes.h"
#include <tig/tig_texture.h>
#include <tig/tig_font.h>
#include <critter.h>
#include <EASTL/hash_map.h>
#include "ui_render.h"
#include <EASTL/fixed_string.h>
#include <gamesystems/d20/d20stats.h>

class UiCharEditor{
	BOOL WidgetsInit();
	BOOL SystemInit(GameSystemConf & conf);

	int &GetState();


	void StateTitleRender(int widId);

	// widget IDs
	int classWndId=0;

	// caches
	eastl::hash_map<int, eastl::string> classNamesUppercase;
	eastl::hash_map<int, WidgetType2> classBtns;
	eastl::vector<int> classBtnIds;
	eastl::vector<TigRect> classBtnFrameRects;

	// art assets
	int buttonBox =0;
	ColorRect classBtnShadowColor = ColorRect(0xFF000000);
	ColorRect classBtnColorRect = ColorRect(0xFFFFffff);
	TigTextStyle classBtnTextStyle;
} uiCharEditor;

BOOL UiCharEditor::WidgetsInit(){
	static WidgetType1 classWnd(259,117, 405, 271);
	classWnd.widgetFlags = 1;
	classWnd.render = [](int widId) { uiCharEditor.StateTitleRender(widId); };
	if (classWnd.Add(&classWndId))
		return 0;
	int coloff = 0, rowoff = 0;
	for (auto it: d20ClassSys.vanillaClassEnums){
		
		int newId = 0;
		WidgetType2 classBtn("Class btn", classWndId, 81 + coloff, 47 + rowoff, 110, 20);
		coloff = 119 - coloff;
		if (!coloff)
			rowoff += 29;
		classBtn.Add(&newId);
		classBtnFrameRects.push_back(TigRect(classBtn.x-5, classBtn.y-5, classBtn.width+10, classBtn.height+10));
		UiRenderer::PushFont(PredefinedFont::PRIORY_12);
		auto classMeasure = UiRenderer::MeasureTextSize(classNamesUppercase[it].c_str(), classBtnTextStyle);

		UiRenderer::PopFont();
	}

	return TRUE;
}

BOOL UiCharEditor::SystemInit(GameSystemConf& conf){
	if (textureFuncs.RegisterTexture("art\\interface\\pc_creation\\buttonbox.tga", &buttonBox))
		return 0;

	classBtnTextStyle.flags = 8;
	classBtnTextStyle.field2c = -1;
	classBtnTextStyle.textColor = &classBtnColorRect;
	classBtnTextStyle.shadowColor = &classBtnShadowColor;
	classBtnTextStyle.colors4 = &classBtnColorRect;
	classBtnTextStyle.colors2 = &classBtnColorRect;
	classBtnTextStyle.field0 = 0;
	classBtnTextStyle.kerning = 1;
	classBtnTextStyle.leading = 0;
	classBtnTextStyle.tracking = 3;

	for (auto it: d20ClassSys.vanillaClassEnums){
		auto className = _strdup(d20Stats.GetStatName(it));
		classNamesUppercase[it] = className;
		for (auto &letter: classNamesUppercase[it]){
			letter = toupper(letter);
		}
	}

	return WidgetsInit();
}

int &UiCharEditor::GetState(){
	return temple::GetRef<int>(0x10BE8D34);
}

void UiCharEditor::StateTitleRender(int widId){
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	int state = GetState();
	auto &stateTitles = temple::GetRef<const char*[6]>(0x10BE8D1C);
	auto &rect = temple::GetRef<TigRect>(0x10BE8E64);
	auto &style = temple::GetRef<TigTextStyle>(0x10BE9640);
	UiRenderer::DrawTextInWidget(widId, stateTitles[state], rect, style);
	UiRenderer::PopFont();
}

class UiCharEditorHooks : public TempleFix {
	

	void apply() override {

	}
} uiCharEditorHooks;
