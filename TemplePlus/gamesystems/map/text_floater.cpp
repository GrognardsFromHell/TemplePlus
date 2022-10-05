#include "stdafx.h"
#include "gamesystems/legacymapsystems.h"
#include "float_line.h"
#include <tig/tig_font.h>
#include <gamesystems/gamesystems.h>
#include <gamesystems/objects/objsystem.h>
#include <ui/ui_render.h>
#include <infrastructure/meshes.h>

struct FloatText {
	int floaterLifeTime;
	const char* text = nullptr;
	int unk8;
	int unkC;
	int width;
	int height;
	uint8_t alpha;
	uint8_t unk19;
	uint8_t unk1A;
	uint8_t unk1B;
	int color;
	int categoryBit;

	void UpdateAlpha(int fadeTime);
};

struct FloatTextNode {
	FloatText content;
	FloatTextNode* next = nullptr;
};
static_assert(temple::validate_size<FloatTextNode, 0x28>::value, "FloatTextNode expected size is 0x28");

struct ObjFloats {
	objHndl handle;
	FloatTextNode* floatedTexts;
	int renderHeight;
	ObjFloats* next;
	int unk14;
};
static_assert(temple::validate_size<ObjFloats, 0x18>::value, "ObjFloats expected size is 0x18");

class TextFloaterSystem::TextFloaterImpl {
	friend class FloatLineHooks;
public:
	void FloatMesline(objHndl handle, int category, FloatLineColor color, const char* text);
private:
	uint32_t& mTime = temple::GetRef<uint32_t>(0x10B3D808);
	int& mLineHeightPlus2 = temple::GetRef<int>(0x10B3D80C);
	int& mRectWidth  = temple::GetRef<int>(0x10B3D82C);
	int& mRectHeight = temple::GetRef<int>(0x10B3D830);
	ObjFloats*& mObjFloatsPool = temple::GetRef<ObjFloats*>(0x10B3D834);
	FloatTextNode*& mTextFloaterPool = temple::GetRef<FloatTextNode*>(0x10B3D84C);
	TigTextStyle& mTextStyle = temple::GetRef <TigTextStyle> (0x10B3D850);
	int& mCategoryMask = temple::GetRef<int>(0x10B3D8A0);
	BOOL& mFloatersDisabled = temple::GetRef<BOOL>(0x10B3D8A4);

	ObjFloats*& mObjFloats = temple::GetRef<ObjFloats*>(0x10B3D8AC);
	int& mSpeed = temple::GetRef<int>(0x10B3D8B0);
	int& mDuration = temple::GetRef<int>(0x10B3D928);
};


TextFloaterSystem::TextFloaterSystem(const GameSystemConf& config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100a2040);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system TextFloater");
	}
	mImpl = std::make_unique<TextFloaterImpl>();
}
TextFloaterSystem::~TextFloaterSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100a2980);
	shutdown();
}
void TextFloaterSystem::ResetBuffers(const RebuildBufferInfo& rebuildInfo) {
	auto resetBuffers = temple::GetPointer<void(const RebuildBufferInfo*)>(0x100a1df0);
	resetBuffers(&rebuildInfo);
}
void TextFloaterSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x100a2970);
	reset();
}
void TextFloaterSystem::AdvanceTime(uint32_t time) {
	auto advanceTime = temple::GetPointer<void(uint32_t)>(0x100a2480);
	advanceTime(time);
}
void TextFloaterSystem::CloseMap() {
	auto mapClose = temple::GetPointer<void()>(0x100a2970);
	mapClose();
}
const std::string& TextFloaterSystem::GetName() const {
	static std::string name("TextFloater");
	return name;
}


class FloatLineHooks : public TempleFix {

	void apply() override {
		replaceFunction<void(objHndl, int, FloatLineColor, const char*)>(0x100A2200,
			[](objHndl handle, int category, FloatLineColor color, const char* text) {
				auto ptr = gameSystems->GetTextFloater().mImpl.get();
				return ptr->FloatMesline(handle, category, color, text);
			});
	}
} floatLineHooks;

void TextFloaterSystem::TextFloaterImpl::FloatMesline(objHndl handle, int category, FloatLineColor color, const char* text)
{
	// Find object handle in mObjFloats
	auto objFloatNode = mObjFloats;
	ObjFloats* objFloatNodePrev = nullptr;
	while (objFloatNode && objFloatNode->handle != handle) {
		objFloatNodePrev = objFloatNode;
		objFloatNode = objFloatNode->next;
	}
	
	// handle not found, allocate new
	if (!objFloatNode) {
		ObjFloats* floatNodeAlloced = mObjFloatsPool;
		
		if (floatNodeAlloced) {
			mObjFloatsPool = mObjFloatsPool->next;
		}
		else { // pool is empty, alloc new
			floatNodeAlloced = new ObjFloats();
		}
		
		floatNodeAlloced->floatedTexts = nullptr;
		floatNodeAlloced->renderHeight = 0;
		floatNodeAlloced->next = nullptr;
		floatNodeAlloced->handle = handle;
		objFloatNode = floatNodeAlloced;
	}
	if (objFloatNodePrev)
		objFloatNodePrev->next = objFloatNode;
	else
		mObjFloats = objFloatNode;
	
	// ensure correct render height
	objFloatNode->renderHeight = objSystem->GetObject(handle)->GetFloat(obj_f_3d_render_height);
	if (objFloatNode->renderHeight <= 0.001f) {
		auto model (objects.GetAnimHandle(handle));
		objects.UpdateRenderHeight(handle, *model);
		objFloatNode->renderHeight = objSystem->GetObject(handle)->GetFloat(obj_f_3d_render_height);
	}
	
	// Get the last text node
	FloatTextNode* lastText = objFloatNode->floatedTexts;
	int count = 0;
	while (lastText && lastText->next) {
		lastText = lastText->next;
		count++;
	}

	// Temple+: added this condition to prevent massive text floater spam 
	// that seriously bogged down the system during some automated testing scenarios
	const int MAX_SIMULTANEOUS_FLOATS = 10;
	if (count >= MAX_SIMULTANEOUS_FLOATS) {
		return;
	}

	// Allocate new FloatTextNode (or get from pool)
	FloatTextNode* newText = mTextFloaterPool;
	if (mTextFloaterPool) {
		mTextFloaterPool = mTextFloaterPool->next;
		newText->next = 0;
	}
	else {
		newText = new FloatTextNode();
		newText->content.text = 0;
		newText->next = 0;
	}
	
	int fullAlphaTime = mLineHeightPlus2;
	auto& content = newText->content;
	content.floaterLifeTime = 4 * fullAlphaTime;
	if (lastText) {
		lastText->next = newText;
		auto lastTextLifetime = lastText->content.floaterLifeTime;
		if (lastTextLifetime > 3 * fullAlphaTime) // for a 2nd immediate call, this will be 4*fullAlphaTime, so yes
			content.floaterLifeTime = fullAlphaTime + lastTextLifetime;
	}
	else {
		objFloatNode->floatedTexts = newText;
	}

	if (newText->content.text) {
		free((void*)(newText->content.text));
	}
	content.text = _strdup(text);
	content.color = color;
	content.categoryBit = category;
	UiRenderer::PushFont("priory-12", 12, 1);
	
	auto metrics = UiRenderer::MeasureTextSize(text, mTextStyle);
	content.unkC = 0;
	content.unk8 = 0;
	content.width = metrics.width;
	content.height = metrics.height;
	UiRenderer::PopFont();
	objects.SetFlag(handle, OF_TEXT_FLOATER);
	content.UpdateAlpha(mLineHeightPlus2);
	
}

/* 0x100A1FA0 */
void FloatText::UpdateAlpha(int fullAlphaTime)
{

	uint8_t halfTime = (uint8_t)(fullAlphaTime / 2);
	// this ticks down by 1 every 50ms (real time)
	int offsetTime = fullAlphaTime / 2 + this->floaterLifeTime; 

	/*
	* Alpha vs offsetTime: (counts down)
	* 4.5*fullAlphaTime or greater: alpha = 0
	* 4.5 to 3*fullAlphaTime: Ramp up from 0 to 255 (maps to 0.5 to 2*fullAlphaTime)
	* 3   to 2*fullAlphaTime: Hold 255
	* 2   to 0.5: Ramp down to 0
	* 
	* Normally starts at 4.5*fullAlphaTime.
	* Consecutive floatMesLine calls start larger to ensure no overlap.
	*/
	

	if (offsetTime > 3 * fullAlphaTime)
		offsetTime = 5 * fullAlphaTime - offsetTime;
	
	if (offsetTime <= (int)halfTime) {
		this->alpha = 0;
		return;
	}

	if (offsetTime > 2 * fullAlphaTime) {
		this->alpha = 0xFFu;
		return;
	}
	
	auto alphaNew = (uint32_t)(
		(offsetTime - (int)halfTime)/ (2.0f * fullAlphaTime - halfTime)
		* 255.0f);
	this->alpha = alphaNew;
	
	return;
	
}
