
#pragma once

#define API __declspec(dllexport)

#include <memory>
#include <graphics/device.h>

struct MdfPreviewNative;

extern "C" {
	API MdfPreviewNative* MdfPreviewNative_Load(const wchar_t* dllPath,
		const wchar_t* tpDataPath);
	API void MdfPreviewNative_Unload(MdfPreviewNative*);
	API void MdfPreviewNative_InitDevice(MdfPreviewNative *native, 
		HWND handle, 
		int renderWidth, int renderHeight); 
	API void MdfPreviewNative_FreeDevice(MdfPreviewNative *native);
	API void MdfPreviewNative_SetCameraPos(MdfPreviewNative *native, float x, float y);
	API void MdfPreviewNative_GetCameraPos(MdfPreviewNative *native, float* x, float* y);
	API void MdfPreviewNative_Render(MdfPreviewNative *native);
	API void MdfPreviewNative_ScreenToWorld(MdfPreviewNative *native, float x, float y, float* xOut, float *yOut, float *zOut);
	API void MdfPreviewNative_SetRenderSize(MdfPreviewNative *native, int w, int h);
	API bool MdfPreviewNative_SetMaterial(MdfPreviewNative *native, const char *name);
	API bool MdfPreviewNative_SetModel(MdfPreviewNative *native,
		const char *skmFilename,
		const char *skaFilename);
	API void MdfPreviewNative_SetAnimation(MdfPreviewNative* native, int animId, bool combatAnim);
	API void MdfPreviewNative_SetRotation(MdfPreviewNative *native, float rotation);
	API void MdfPreviewNative_SetScale(MdfPreviewNative *native, float scale);
	API void MdfPreviewNative_SetOffsetZ(MdfPreviewNative* native, float offz);
	API void MdfPreviewNative_SetLoopAnimation(MdfPreviewNative* native, bool loopEn);
	API void MdfPreviewNative_SetPauseAnimation(MdfPreviewNative* native, bool en);
	
	
	
	
	API char *MdfPreviewNative_GetError(MdfPreviewNative *native);
	API char *MdfPreviewNative_GetAndClearLog(MdfPreviewNative *native);
}
