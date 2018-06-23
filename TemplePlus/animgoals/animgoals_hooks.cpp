
#include "stdafx.h"
#include "util/fixes.h"

#include "anim.h"
#include "obj.h"
#include "gamesystems/gamesystems.h"

static class AnimGoalsHooks : public TempleFix {
	
	virtual void apply() override {

		// anim_first_run_idx_for_obj
		replaceFunction<int(objHndl)>(0x10054e20, [](objHndl handle) {
			return gameSystems->GetAnim().GetFirstRunSlotIdxForObj(handle);
		});

		// anim_next_run_idx_for_obj
		replaceFunction<int(int, objHndl)>(0x10054e70, [](int slotIndex, objHndl handle) {
			return gameSystems->GetAnim().GetNextRunSlotIdxForObj(handle, slotIndex);
		});

		// anim_run_id_for_obj
		replaceFunction<int(objHndl, AnimSlotId *)>(0x1000c430, [](objHndl handle, AnimSlotId *slotIdOut) {

			auto slotId = gameSystems->GetAnim().GetFirstRunSlotId(handle);
			if (slotId) {
				if (slotIdOut) {
					*slotIdOut = *slotId;
				}
				return 1;
			} else {
				if (slotIdOut) {
					slotIdOut->Clear();
				}
				return 0;
			}

		});

		// anim_get_highest_priority
		replaceFunction<AnimGoalPriority(objHndl)>(0x1000c500, [](objHndl handle) {
			return gameSystems->GetAnim().GetCurrentPriority(handle);
		});

		// anim_pop_goal
		replaceFunction<void(AnimSlot &, const uint32_t *, const AnimGoal **, AnimSlotGoalStackEntry **, BOOL *)>(0x10016FC0, 
			[](AnimSlot &slot, const uint32_t *popFlags, const AnimGoal **newGoal, AnimSlotGoalStackEntry **newCurrentGoal, BOOL *stopProcessing) {
			bool stopProcessingBool = *stopProcessing == TRUE;
			gameSystems->GetAnim().PopGoal(slot, *popFlags, newGoal, newCurrentGoal, &stopProcessingBool);
			*stopProcessing = stopProcessingBool ? TRUE : FALSE;
		});

		// anim_break_nodes_to_map
		replaceFunction<int(const char *)>(0x1001af30, [](const char *mapName) {
			gameSystems->GetAnim().SaveToMap(mapName);
			return 0;
		});

	}

} hooks;
