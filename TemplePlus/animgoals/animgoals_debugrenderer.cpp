
#include "stdafx.h"

#include "animgoals_debugrenderer.h"
#include "party.h"

#include <tig/tig_startup.h>
#include <graphics/device.h>
#include <graphics/camera.h>
#include <graphics/shaperenderer2d.h>
#include <graphics/shaperenderer3d.h>
#include <graphics/textengine.h>
#include "gamesystems/map/sector.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "animgoals/anim.h"
#include "animgoals.h"
#include "anim_slot.h"
#include "ui/widgets/widget_styles.h"

#include "obj.h"

bool AnimGoalsDebugRenderer::enabled_ = false;
bool AnimGoalsDebugRenderer::showObjectNames_ = false;
bool AnimGoalsDebugRenderer::pathsEnabled_ = false;

void AnimGoalsDebugRenderer::RenderAllAnimGoals(int tileX1, int tileX2, int tileY1, int tileY2)
{

	if (!enabled_ && !pathsEnabled_) {
		return;
	}

	for (auto secY = tileY1 / 64; secY <= tileY2 / 64; ++secY) {
		for (auto secX = tileX1 / 64; secX <= tileX2 / 64; ++secX) {

			LockedMapSector sector(secX, secY);

			for (auto tx = 0; tx < 64; ++tx) {
				for (auto ty = 0; ty < 64; ++ty) {

					auto obj = sector.GetObjectsAt(tx, ty);
					while (obj) {

						//if (pathsEnabled_) // included in render all
						{
							RenderCurrentGoalPath(obj->handle);
						}
							
						if (enabled_) {
							RenderAnimGoals(obj->handle);
						}
						obj = obj->next;
					}

				}
			}
		}
	}

}

void AnimGoalsDebugRenderer::RenderAnimGoals(objHndl handle)
{

	//RenderCurrentGoalPath(handle);

	auto worldLocAboveHead = objects.GetLocationFull(handle).ToInches3D(objects.GetRenderHeight(handle));

	auto topOfObjectInUi = tig->GetRenderingDevice().GetCamera().WorldToScreenUi(worldLocAboveHead);

	auto &renderer2d = tig->GetShapeRenderer2d();

	auto &textEngine = tig->GetRenderingDevice().GetTextEngine();

	auto &animSystem = gameSystems->GetAnim();

	std::vector<int> slotIdsPerLine;
	std::vector<std::string> lines;
	std::vector<uint32_t> lineHeights;
	uint32_t overallHeight = 0;

	gfx::TextStyle textStyle = widgetTextStyles->GetTextStyle("default-button-text");

	for (auto slotIdx = animSystem.GetFirstRunSlotIdxForObj(handle);
		slotIdx != -1;
		slotIdx = animSystem.GetNextRunSlotIdxForObj(handle, slotIdx)) {

		auto &slot = animSystem.mSlots[slotIdx];

		for (int i = 0; i <= slot.currentGoal; i++) {
			auto &stackEntry = slot.goals[i];
			auto goalName = GetAnimGoalTypeName(stackEntry.goalType);
			if (i == slot.currentGoal) {
				lines.push_back(fmt::format("{} ({})", goalName, slot.currentState));
			} else {
				lines.push_back(std::string(goalName));
			}
			slotIdsPerLine.push_back(slotIdx);
			gfx::TextMetrics metrics;
			metrics.width = 120;
			metrics.height = 500;
			textEngine.MeasureText(textStyle, lines.back(), metrics);
			overallHeight += metrics.height + 1;
			lineHeights.push_back(metrics.height);
		}

	}

	int x = (int)(topOfObjectInUi.x - 60);
	int y = (int)(topOfObjectInUi.y - overallHeight);

	// Render the object name above the goal list
	if (showObjectNames_) {
		gfx::FormattedText t;
		t.defaultStyle = textStyle;
		t.defaultStyle.align = gfx::TextAlign::Center;
		t.defaultStyle.dropShadow = true;
		auto protoNum = gameSystems->GetObj().GetProtoId(handle);
		auto displayName = objects.GetDisplayName(handle, handle);
		t.text = local_to_ucs2(fmt::format("{} #{}", displayName, protoNum));
		auto &nameStyle = t.formats.push_back();
		nameStyle.startChar = 0;
		nameStyle.length = displayName.size();
		nameStyle.style = t.defaultStyle;
		nameStyle.style.bold = true;

		gfx::TextMetrics nameMetrics;
		textEngine.MeasureText(t, nameMetrics);

		TigRect nameRect;
		nameRect.x = x - 60;
		nameRect.width = 240;
		nameRect.y = y - nameMetrics.lineHeight - 2;
		nameRect.height = nameMetrics.lineHeight;
		textEngine.RenderText(nameRect, t);
	}

	// Draw in reverse because the stack is actually ordered the other way around
	int prevSlotIdx = -1;
	for (int i = lines.size() - 1; i >= 0; i--) {
		auto lineHeight = lineHeights[i];
		auto line = lines[i];
		auto slotIdx = slotIdsPerLine[i];
		renderer2d.DrawRectangle((float) x, (float) y, 120.0f, (float) lineHeight, 0x7F7F7F7F);

		// When starting rendering info about a new slot, draw a border at the top
		// and draw the slot idx on the left, outside of the text box
		if (slotIdx != prevSlotIdx) {
			prevSlotIdx = slotIdx;
			XMFLOAT2 from{ (float) x, y - 1.0f };
			XMFLOAT2 to{ x + 120.0f, y - 1.0f };
			gfx::Line2d line(from, to, XMCOLOR(0xFFFFFFFF));
			renderer2d.DrawLines(gsl::span(&line, 1));

			gfx::FormattedText t;
			t.defaultStyle = textStyle;
			t.defaultStyle.align = gfx::TextAlign::Left;
			t.text = fmt::format(L"#{}", slotIdx);

			gfx::TextMetrics metrics;
			textEngine.MeasureText(t, metrics);

			TigRect rect;
			rect.x = x - metrics.width - 2;
			rect.y = y + (lineHeight - metrics.lineHeight) / 2;
			rect.width = metrics.width;
			rect.height = metrics.height;
			textEngine.RenderText(rect, t);
		}
		
		gfx::FormattedText t;
		t.defaultStyle = textStyle;
		t.text = local_to_ucs2(line);

		TigRect rect;
		rect.x = x;
		rect.y = y;
		rect.width = 120;
		rect.height = lineHeight;
		textEngine.RenderText(rect, t);

		y += lineHeight + 1;
	}

}

void AnimGoalsDebugRenderer::RenderCurrentGoalPath(objHndl handle)
{
	auto &animSystem = gameSystems->GetAnim();
	auto slotIdx = animSystem.GetFirstRunSlotIdxForObj(handle);
	if (slotIdx == -1) {
		return;
	}

	auto &slot = animSystem.mSlots[slotIdx];
	if (!slot.path.IsComplete()) {
		return;
	}

	auto &renderer3d = tig->GetShapeRenderer3d();

	XMCOLOR color(1.0f, 1.0f, 1.0f, 0.5f);
	XMCOLOR circleBorderColor(1.0f, 1.0f, 1.0f, 1.0f);
	XMCOLOR circleFillColor(0.0f, 0.0f, 0.0f, 0.0f);

	auto currentPos = objects.GetLocationFull(handle).ToInches3D();
	
	for (int i = slot.path.currentNode; i + 1 < slot.path.nodeCount; i++) {
		auto nextPos = slot.path.nodes[i].ToInches3D();
		renderer3d.DrawLineWithoutDepth(currentPos, nextPos, color);
		renderer3d.DrawFilledCircle(nextPos, 4, circleBorderColor, circleFillColor, false);
		renderer3d.DrawFilledCircle(nextPos, 4, circleBorderColor, circleFillColor, true);
		currentPos = nextPos;
	}

	// Draw the last path node
	auto pathTo = slot.path.to.ToInches3D();
	renderer3d.DrawLineWithoutDepth(currentPos, pathTo, color);
	renderer3d.DrawFilledCircle(pathTo, 4, circleBorderColor, circleFillColor, false);
	renderer3d.DrawFilledCircle(pathTo, 4, circleBorderColor, circleFillColor, true);

}
