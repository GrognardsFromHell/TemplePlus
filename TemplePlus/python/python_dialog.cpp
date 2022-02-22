#include "stdafx.h"
#include "pybind_common.h"
#include <pybind11/embed.h>
#include "ui/ui_dialog.h"
#include "dialog.h"

PYBIND11_EMBEDDED_MODULE(gamedialog, m) {
	m.def("is_engaged", [](){
		return uiDialog->IsDialogEngaged();
		});
	m.def("get_current_state", []()->DialogState* {
		return uiDialog->GetCurrentDialog();
		}, py::return_value_policy::reference);

	py::class_<DialogState>(m, "DialogState")
		.def_readonly("reply_count", &DialogState::pcLines)
		.def("get_reply_effect", [](DialogState& self, int idx)->py::bytes {
			if (idx < 0 || idx >= self.pcLines || idx >= 5 || !self.effectFields[idx]) {
				return std::string("");
			}

			return std::string(self.effectFields[idx]);
			})
		;
}