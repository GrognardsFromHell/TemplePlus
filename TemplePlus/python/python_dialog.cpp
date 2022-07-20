#include "stdafx.h"
#include "pybind_common.h"
#include <pybind11/embed.h>
#include "ui/ui_dialog.h"
#include "dialog.h"


bool VerifyReplyIdx(const DialogState& state, int idx) {
	return idx >= 0 && idx < DIALOG_REPLIES_MAX&& idx < state.pcLines;
}

PYBIND11_EMBEDDED_MODULE(gamedialog, m) {
	m.def("is_engaged", [](){
		return uiDialog->IsDialogEngaged();
		});
	m.def("get_current_state", []()->DialogState* {
		return uiDialog->GetCurrentDialog();
		}, py::return_value_policy::reference);

	py::class_<DialogState>(m, "DialogState")
		.def_readonly("script_id", &DialogState::dialogScriptId)
		.def_readonly("action_type", &DialogState::actionType)
		.def_readonly("line_number", &DialogState::lineNumber)
		.def_readonly("reply_count", &DialogState::pcLines)
		.def("get_reply_effect", [](DialogState& self, int idx)->py::bytes {
			if (!VerifyReplyIdx(self, idx) || !self.effectFields[idx]) {
				return std::string("");
			}

			return std::string(self.effectFields[idx]);
			})
		.def("get_reply_skill", [](DialogState& self, int idx)->int {
				if (!VerifyReplyIdx(self, idx)) return -1;
				switch (self.pcLineSkillUse[idx]) {
				case 0:
					return -1;
				case 1:
					return SkillEnum::skill_bluff;
				case 2:
					return SkillEnum::skill_diplomacy;
				case 3:
					return SkillEnum::skill_intimidate;
				case 4:
					return SkillEnum::skill_sense_motive;
				case 5:
					return SkillEnum::skill_gather_information;
				default:
					return -1;
				}
			})
		/*.def("get_reply_guard", [](DialogState& self, int idx)-> py::bytes{
				if (!VerifyReplyIdx(self, idx) || self.field_182C[idx] == 0) {
					return std::string("");
				}
				return std::string( (const char*)self.field_182C[idx]);
			})*/
		.def("get_npc_reply_id", [](DialogState& self, int idx)->int { // the NPC line ID that results from answering this reply
				if (!VerifyReplyIdx(self, idx)) {
					return 0;
				}
				return self.npcReplyIds[idx];
			})
		;
}