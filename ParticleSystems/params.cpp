
#include "particles/params.h"
#include "particles/external.h"
#include "particles/instances.h"

uint32_t PartSysRandomGen::mState = 0x1127E5;

static const float DefaultValues[] = {
	0, // 0
	0, // 1
	0, // 2
	0, // 3
	0, // 4
	0, // 5
	0, // 6
	0, // 7
	0, // 8
	0, // 9
	0, // 10
	0, // 11
	1.0f, // 12
	1.0f, // 13
	1.0f, // 14
	0, // 15
	0, // 16
	0, // 17
	0, // 18
	0, // 19
	0, // 20
	255.0f, // 21
	255.0f, // 22
	255.0f, // 23
	255.0f, // 24
	0, // 25
	0, // 26
	0, // 27
	0, // 28
	0, // 29
	0, // 30
	0, // 31
	0, // 32
	0, // 33
	0, // 34
	1.0f, // 35
	1.0f, // 36
	1.0f, // 37
	0, // 38
	0, // 39
	0, // 40
	0, // 41
	0, // 42
	0, // 43
	0, // 44
};

float PartSysParam::GetDefaultValue(PartSysParamId id) {
	return DefaultValues[id];
}

float PartSysParamSpecial::GetValue(PartSysEmitter* emitter, int particleIdx, float lifetimeSec) {

	// Returns the radius of the object associated with the emitter
	auto obj = emitter->GetAttachedTo();
	
	if (!obj) {
		return 0; // Fallback value
	}

	if (mSpecialType == PSPST_RADIUS) {
		return IPartSysExternal::GetCurrent()->GetObjRadius(obj);
	} else {
		return 0;
	}
	
}
