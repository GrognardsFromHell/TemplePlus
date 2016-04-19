#include "stdafx.h"

#include <util/fixes.h>

static class RenderStatesHooks : public TempleFix {
public:
	void apply() override;

	// Commits the given states
	static void CommitState(const void* states);

	static void EnableLighting();
	static void DisableLighting();
	static int GetMaxActiveLights();
	static void SetLight(int index, const void* light);
	static void EnableLight(int index);
	static void DisableLight(int index);

} fix;

void RenderStatesHooks::apply() {
	replaceFunction(0x101F0A20, CommitState);
	replaceFunction(0x101D82F0, EnableLighting);
	replaceFunction(0x101D8300, DisableLighting);
	replaceFunction(0x101D8350, GetMaxActiveLights);
	replaceFunction(0x101D8360, EnableLight);
	replaceFunction(0x101D8390, DisableLight);
	replaceFunction(0x101D83C0, SetLight);
}

void RenderStatesHooks::CommitState(const void* states) {
	throw TempleException("Unsupported!");
}

void RenderStatesHooks::EnableLighting() {
	throw TempleException("Unsupported!");
}

void RenderStatesHooks::DisableLighting() {
	throw TempleException("Unsupported!");
}

int RenderStatesHooks::GetMaxActiveLights() {
	throw TempleException("Unsupported!");
}

void RenderStatesHooks::SetLight(int index, const void* legacyLight) {
	throw TempleException("Unsupported!");
}

void RenderStatesHooks::EnableLight(int index) {
	throw TempleException("Unsupported!");
}

void RenderStatesHooks::DisableLight(int index) {
	throw TempleException("Unsupported!");
}

// Copy from TP render states into the render states found in ToEE
void ResetLegacyRenderStates() {
	throw TempleException("Unsupported!");
}

void CopyLightingState() {
	throw TempleException("Unsupported!");
}
