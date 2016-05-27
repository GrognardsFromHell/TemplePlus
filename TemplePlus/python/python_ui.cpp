
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

	py::class_<Widget>(m, "Widget")
		.def_readwrite("widgetId", &Widget::widgetId)
		.def_readwrite("parentId", &Widget::parentId)
		.def_readwrite("x", &Widget::x)
		.def_readwrite("y", &Widget::y)
		.def_readwrite("x_base", &Widget::xrelated)
		.def_readwrite("y_base", &Widget::yrelated)
		.def_readwrite("width", &Widget::width)
		.def_readwrite("height", &Widget::height)
		;
	
	py::class_<WidgetType1>(m, "WidgetWindow", py::base<Widget>())
		.def(py::init())
		.def_readwrite("children_count", &WidgetType1::childrenCount)
		.def_readwrite("windowId", &WidgetType1::windowId)
		;

	py::class_<WidgetType2>(m, "WidgetButton", py::base<Widget>())
		.def_readwrite("state", &WidgetType2::buttonState, "0 - normal, 1 - hovered, 2 - down, 3 - released, 4 - disabled")
		.def_readwrite("sound_down", &WidgetType2::sndDown)
		.def_readwrite("sound_click", &WidgetType2::sndClick)
		.def_readwrite("sound_hover_on", &WidgetType2::hoverOn)
		.def_readwrite("sound_hover_off", &WidgetType2::hoverOff)
		;

	py::class_<WidgetType3>(m, "WidgetScrollbar", py::base<Widget>())
		.def(py::init())
		.def("init", &WidgetType3::Init)
		.def_readwrite("scroll_position_min", &WidgetType3::yMin)
		.def_readwrite("scroll_position_max", &WidgetType3::yMax)
		.def_readwrite("scroll_position", &WidgetType3::scrollbarY)
		.def_readwrite("scroll_quantum",&WidgetType3::scrollQuantum)
		.def("GetPosition", &WidgetType3::GetY)
		.def("Register", &WidgetType3::Add)
		;

	

	py::class_<CharEditorSelectionPacket>(m,"CharEditorSelection")
		.def_readwrite("classCode", &CharEditorSelectionPacket::classCode)
		;

	return m.ptr();
}
