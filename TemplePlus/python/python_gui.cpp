#include "stdafx.h"
#include <pybind11/embed.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>
#include <ui/ui.h>
#include <ui/widgets/widgets.h>

namespace py = pybind11;

WidgetContainer* GetContainer(int widId) {
	auto wnd = (WidgetContainer*)uiManager->GetAdvancedWidget(widId);
	if (!wnd || !wnd->IsContainer()) {
		return nullptr;
	}
	return wnd;
}


PYBIND11_EMBEDDED_MODULE(tpgui, m) {

	m.doc() = "Temple+ GUI, used for custom user UIs.";

	m.def("_add_container", [](const std::string& id, int w, int h) {
		WidgetContainer wnd(w, h);
		wnd.SetId(id);
		return wnd.GetWidgetId();
		});

	m.def("_add_button", [](const std::string& id, int w=-1, int h=-1) {
		WidgetButton btn;
		btn.SetId(id);
		if (w > 0 && h > 0) {
			btn.SetSize({ w,h });
		}
		return btn.GetWidgetId();
		});

	m.def("_get_container", &GetContainer, py::return_value_policy::reference);

	m.def("_get_button", [](int widId)->WidgetButton* {
		auto btn = (WidgetButton*)uiManager->GetAdvancedWidget(widId);
		if (!btn || !btn->IsButton()) {
			return nullptr;
		}
		return btn;
		}, py::return_value_policy::reference);

	py::class_<WidgetBase>(m, "Widget")
		.def_property_readonly("id", &WidgetBase::GetWidgetId)
		.def_property("width", &WidgetBase::GetWidth, &WidgetBase::SetWidth)
		.def_property("height", &WidgetBase::GetHeight, &WidgetBase::SetHeight)
		.def_property("x", &WidgetBase::GetX, &WidgetBase::SetX)
		.def_property("y", &WidgetBase::GetY, &WidgetBase::SetY)
		.def_property("pos", &WidgetBase::GetPos, &WidgetBase::SetPos)
		.def_property("parent", &WidgetBase::GetParent, &WidgetBase::SetParent, py::return_value_policy::reference)
		.def_property("visible", &WidgetBase::IsVisible, &WidgetBase::SetVisible)
		.def("show", &WidgetBase::Show)
		.def("hide", &WidgetBase::Hide)
		.def("bring_to_front", &WidgetBase::BringToFront)
		;
	py::class_<WidgetContainer, WidgetBase>(m, "Container")
		.def(py::init<int, int>(), py::arg("width") = 0, py::arg("height") = 0)
		//.def("add_child", &WidgetContainer::Add)
		//.def("add_content", &WidgetContainer::AddContent)
		//.def_property_readonly("children", &WidgetContainer::GetChildren) // eastl bah
		;
	py::class_<WidgetButtonStyle>(m, "ButtonStyle")
		;
	py::class_<WidgetButton, WidgetBase>(m, "Button")
		.def(py::init())
		.def_property("active", &WidgetButton::IsActive, &WidgetButton::SetActive)
		.def_property("disabled", &WidgetButton::IsDisabled, &WidgetButton::SetDisabled)
		.def_property("style", &WidgetButton::GetStyle, py::overload_cast<const WidgetButtonStyle&>(&WidgetButton::SetStyle))
		;


}