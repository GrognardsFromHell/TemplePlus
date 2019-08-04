#pragma once
#include "common.h"

enum UiIntgameTurnbasedFlags : int {
	UITB_ShowPathPreview = 0x1,
	UITB_IsLastSequenceActionWithPath = 0x2
};

struct PathQueryResult;

struct AooShaderPacket //used for applying the AoO indicators
{
	LocAndOffsets loc;
	int shaderId;
	int field14;
};


class UiIntgameTurnbased
{
public:
	void CreateMovePreview(PathQueryResult* pqr, UiIntgameTurnbasedFlags flags);
	int PathpreviewGetFromToDist(PathQueryResult* path);
	void RenderCircle(LocAndOffsets loc, float zoffset, int fillColor, int outlineColor, float radius); // used in rendering path related circles; color is in 0xAARRGGBB format
	void PathRenderEndpointCircle(LocAndOffsets* loc, objHndl obj, float zoffset); // the circle that gets drawn when previewing a path in combat mode
	void RenderPositioningBlueCircle(LocAndOffsets loc, objHndl obj); // the circle that gets drawn when you click a destination and persists until you arrive there
	void AooInterceptArrowDraw(LocAndOffsets* perfLoc, LocAndOffsets* targetLoc);
	bool AooPossible(objHndl handle);

	void CursorRenderUpdate();
	
};

extern UiIntgameTurnbased uiIntgameTb;
