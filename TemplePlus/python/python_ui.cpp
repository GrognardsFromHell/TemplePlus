
#include "stdafx.h"
#include <infrastructure/elfhash.h>
#include "python_object.h"

#undef HAVE_ROUND
#include <pybind11/pybind11.h>
#include <pybind11/common.h>
#include <pybind11/cast.h>
#include <ui/ui_char_editor.h>
#include <ui/ui.h>

namespace py = pybind11;
using namespace pybind11;
using namespace pybind11::detail;

//template <> class type_caster<objHndl> {
//public:
//	bool load(handle src, bool) {
//		value = PyObjHndl_AsObjHndl(src.ptr());
//		success = true;
//		return true;
//	}
//
//	static handle cast(const objHndl &src, return_value_policy /* policy */, handle /* parent */) {
//		return PyObjHndl_Create(src);
//	}
//
//	PYBIND11_TYPE_CASTER(objHndl, _("objHndl"));
//protected:
//	bool success = false;
//};




PYBIND11_PLUGIN(tp_ui) {
	py::module m("tpui", "Temple+ UI module, for UI stuff.");

	py::class_<LgcyWidget>(m, "Widget")
		.def_readwrite("widgetId", &LgcyWidget::widgetId)
		.def_readwrite("parentId", &LgcyWidget::parentId)
		.def_readwrite("x", &LgcyWidget::x)
		.def_readwrite("y", &LgcyWidget::y)
		.def_readwrite("x_base", &LgcyWidget::xrelated)
		.def_readwrite("y_base", &LgcyWidget::yrelated)
		.def_readwrite("width", &LgcyWidget::width)
		.def_readwrite("height", &LgcyWidget::height)
		;
	
	py::class_<LgcyWindow>(m, "WidgetWindow", py::base<LgcyWidget>())
		.def(py::init())
		.def_readwrite("children_count", &LgcyWindow::childrenCount)
		.def_readwrite("windowId", &LgcyWindow::zIndex)
		;

	py::class_<LgcyButton>(m, "WidgetButton", py::base<LgcyWidget>())
		.def_readwrite("state", &LgcyButton::buttonState, "0 - normal, 1 - hovered, 2 - down, 3 - released, 4 - disabled")
		.def_readwrite("sound_down", &LgcyButton::sndDown)
		.def_readwrite("sound_click", &LgcyButton::sndClick)
		.def_readwrite("sound_hover_on", &LgcyButton::hoverOn)
		.def_readwrite("sound_hover_off", &LgcyButton::hoverOff)
		;

	py::class_<LgcyScrollBar>(m, "WidgetScrollbar", py::base<LgcyWidget>())
		.def(py::init())
		.def("init", (bool (LgcyScrollBar::*)(int, int, int)) &LgcyScrollBar::Init)
		.def("init", (bool (LgcyScrollBar::*)(int, int, int, int)) &LgcyScrollBar::Init)
		.def_readwrite("scroll_position_min", &LgcyScrollBar::yMin)
		.def_readwrite("scroll_position_max", &LgcyScrollBar::yMax)
		.def_readwrite("scroll_position", &LgcyScrollBar::scrollbarY)
		.def_readwrite("scroll_quantum",&LgcyScrollBar::scrollQuantum)
		.def("GetPosition", &LgcyScrollBar::GetY)
		.def("Register", &LgcyScrollBar::Add)
		;

	

	py::class_<CharEditorSelectionPacket>(m,"CharEditorSelection")
		.def_readwrite("classCode", &CharEditorSelectionPacket::classCode)
		;

	return m.ptr();
}
