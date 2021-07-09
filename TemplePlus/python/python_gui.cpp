#include "stdafx.h"
#include <pybind11/embed.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>
#include <ui/ui.h>
#include <ui/widgets/widgets.h>

namespace py = pybind11;
//
//NAMESPACE_BEGIN(PYBIND11_NAMESPACE)
//NAMESPACE_BEGIN(detail)
//
///// Extracts an const lvalue reference or rvalue reference for U based on the type of T (e.g. for
///// forwarding a container element).  Typically used indirect via forwarded_type(), below.
//template <typename T, typename U>
//using eastl_forwarded_type = conditional_t<
//	eastl::is_lvalue_reference<T>::value, remove_reference_t<U>&, remove_reference_t<U>&&>;
//
///// Forwards a value U as rvalue or lvalue according to whether T is rvalue or lvalue; typically
///// used for forwarding a container's elements.
//template <typename T, typename U>
//eastl_forwarded_type<T, U> eastl_forward_like(U&& u) {
//	return eastl::forward<detail::eastl_forwarded_type<T, U>>(eastl::forward<U>(u));
//}
//
//template <typename Type, typename Value> struct eastl_list_caster {
//	using value_conv = make_caster<Value>;
//
//	bool load(handle src, bool convert) {
//		if (!isinstance<sequence>(src))
//			return false;
//		auto s = reinterpret_borrow<sequence>(src);
//		value.clear();
//		reserve_maybe(s, &value);
//		for (auto it : s) {
//			value_conv conv;
//			if (!conv.load(it, convert))
//				return false;
//			value.push_back(cast_op<Value&&>(eastl::move(conv)));
//		}
//		return true;
//	}
//
//private:
//	template <typename T = Type,
//		enable_if_t<eastl::is_same<decltype(eastl::declval<T>().reserve(0)), void>::value, int> = 0>
//		void reserve_maybe(sequence s, Type*) { value.reserve(s.size()); }
//	void reserve_maybe(sequence, void*) { }
//
//public:
//	template <typename T>
//	static handle cast(T&& src, return_value_policy policy, handle parent) {
//		policy = return_value_policy_override<Value>::policy(policy);
//		list l(src.size());
//		size_t index = 0;
//		for (auto&& value : src) {
//			auto value_ = reinterpret_steal<object>(value_conv::cast(eastl_forward_like<T>(value), policy, parent));
//			if (!value_)
//				return handle();
//			PyList_SET_ITEM(l.ptr(), (ssize_t)index++, value_.release().ptr()); // steals a reference
//		}
//		return l.release();
//	}
//
//	PYBIND11_TYPE_CASTER(Type, _("List[") + value_conv::name() + _("]"));
//};
//
//template <typename Type, typename Alloc> struct type_caster<eastl::vector<Type, Alloc>>
//	: eastl_list_caster<eastl::vector<Type, Alloc>, Type> { };
//
////template <typename Type, typename Alloc> struct type_caster<eastl::vector<Type, Alloc>>
////	: list_caster<eastl::vector<Type, Alloc>, Type> { };
//
//NAMESPACE_END(detail)
//NAMESPACE_END(PYBIND11_NAMESPACE)

using WidgetRegistry = std::map<std::string, WidgetBase*>;

std::map<std::string, WidgetRegistry> pythonWidgets;



WidgetContainer* GetContainer(int widId) {
	auto wnd = (WidgetContainer*)uiManager->GetAdvancedWidget(widId);
	if (!wnd || !wnd->IsContainer()) {
		return nullptr;
	}
	return wnd;
}

// Don't use this yet!

PYBIND11_EMBEDDED_MODULE(tpgui, m) {

	m.doc() = "Temple+ GUI, used for custom user UIs.";

	m.def("_add_container", [](const std::string& id, int w, int h) {
		auto wnd = std::make_unique<WidgetContainer>(w, h);
		wnd->SetId(id);
		return wnd;
		});

	m.def("_add_button", [](const std::string& id, int w=-1, int h=-1) {
		auto btn = std::make_unique<WidgetButton >();
		btn->SetId(id);
		if (w > 0 && h > 0) {
			btn->SetSize({ w,h });
		}
		return btn;
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
	py::class_<WidgetContent>(m, "WidgetContent")
		.def_property("x", &WidgetContent::GetX, &WidgetContent::SetX)
		.def_property("y", &WidgetContent::GetY, &WidgetContent::SetY)
		.def_property("width", &WidgetContent::GetFixedWidth, &WidgetContent::SetFixedWidth)
		.def_property("height", &WidgetContent::GetFixedHeight, &WidgetContent::SetFixedHeight)
		;
	py::class_<gfx::TextStyle>(m, "TextStyle")
		.def_readwrite("font", &gfx::TextStyle::fontFace)
		.def_readwrite("point_size", &gfx::TextStyle::pointSize)
		;
	py::class_<WidgetImage, WidgetContent>(m, "WidgetImage")
		.def("set_image", &WidgetImage::SetTexture)
		;
	py::class_<WidgetText, WidgetContent>(m, "WidgetText")
		.def("set_text", &WidgetText::SetText)
		.def("set_centered_vertically", &WidgetText::SetCenterVertically)
		.def_property("style", &WidgetText::GetStyle, &WidgetText::SetStyle)
		.def("set_style_by_id", &WidgetText::SetStyleId)
		.def("get_style_id", &WidgetText::GetStyleId)
		;

	py::class_<WidgetContainer, WidgetBase>(m, "Container")
		.def(py::init<int, int>(), py::arg("width") = 0, py::arg("height") = 0)
		.def("add_child", [](WidgetContainer & me, WidgetBase * childWidget)->void {
			me.Add( std::unique_ptr<WidgetBase>( childWidget) );		
			})
		.def("add_content", [](WidgetContainer& me, WidgetContent* content)->void {
				me.AddContent(std::unique_ptr<WidgetContent>(content));
			})
		//.def_property_readonly("children", []() ->std::vector<int> {}&WidgetContainer::GetChildren) // eastl bah
		;
	py::class_<WidgetButtonStyle>(m, "ButtonStyle")
		.def_readwrite("image_normal", &WidgetButtonStyle::normalImagePath)
		.def_readwrite("image_pressed", &WidgetButtonStyle::pressedImagePath)
		.def_readwrite("image_hovered", &WidgetButtonStyle::hoverImagePath)
		.def_readwrite("image_disabled", &WidgetButtonStyle::disabledImagePath)
		.def_readwrite("text_style", &WidgetButtonStyle::textStyleId)
		.def_readwrite("text_style_pressed", &WidgetButtonStyle::pressedTextStyleId)
		.def_readwrite("text_style_disabled", &WidgetButtonStyle::disabledTextStyleId)
		;
	py::class_<WidgetButton, WidgetBase>(m, "Button")
		.def(py::init())
		.def_property("active", &WidgetButton::IsActive, &WidgetButton::SetActive)
		.def_property("disabled", &WidgetButton::IsDisabled, &WidgetButton::SetDisabled)
		.def_property("style", &WidgetButton::GetStyle, py::overload_cast<const WidgetButtonStyle&>(&WidgetButton::SetStyle))
		;


}