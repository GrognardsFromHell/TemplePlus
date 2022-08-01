#include "stdafx.h"
#include <pybind11/embed.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <ui/ui.h>
#include <ui/widgets/widgets.h>
#include <tig/tig_startup.h>
#include <graphics/device.h>

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
#include "ui/ui_python.h"
#include <ui/ui_systems.h>


UiPython * uiPython = nullptr;

UiPython::UiPython(const UiSystemConf& config) {
	uiPython = this;

}
UiPython::~UiPython() {
	
}
const std::string& UiPython::GetName() const {
	static std::string name("python_ui");
	return name;
}


WidgetBase* UiPython::GetWidget(const std::string& id)
{
	auto wid = pyWidgets.find(id);
	if (wid == pyWidgets.end()) {
		return nullptr;
	}
	return wid->second;
}

WidgetContainer* UiPython::AddRootWidget(const std::string& id)
{
	if (GetWidget(id) != nullptr) {
		logger->error("UiPython::AddRootWidget: Widget id={} already exists", id);
		return nullptr;
	}
	auto wnd = std::make_unique<WidgetContainer>(0, 0);
	wnd->SetId(id);
	auto result = wnd.get();

	mRootWidgets[id] = std::move(wnd);
	pyWidgets[id] = result;

	return result;
}

void UiPython::AddWidget(WidgetBase* wid)
{
	pyWidgets[wid->GetId()] = wid;
}

WidgetContainer* GetContainer(const std::string & id) {
	auto wid = uiPython->GetWidget(id);
	if (!wid || !wid->IsContainer()) return nullptr;

	return (WidgetContainer*)(wid);
}
WidgetButton* GetButton(const std::string& id) {
	auto wid = uiPython->GetWidget(id);
	if (!wid || !wid->IsButton()) return nullptr;

	return (WidgetButton*)(wid);
}

PYBIND11_EMBEDDED_MODULE(tpgui, m) {

	m.doc() = "Temple+ GUI, used for custom user UIs.";

	m.def("_add_root_container", [](const std::string& id, int w, int h)->WidgetContainer* {
		auto wid = uiPython->AddRootWidget(id);
		if (!wid) {
			return nullptr;
		}
		wid->SetSize({ w,h });
		return wid;
		}, py::return_value_policy::reference);

	m.def("_add_container", [](const std::string & parentId, const std::string& id, int w, int h)->WidgetContainer* {
		
		if (uiPython->GetWidget(id)) {
			logger->error("_add_container: Widget id={} already exists", id);
			return nullptr;
		}
		auto parent = GetContainer(parentId);
		if (!parent) {
			logger->error("_add_container: Parent id={} does not exist", id);
			return nullptr;
		}

		auto wnd = std::make_unique<WidgetContainer>(w, h);
		wnd->SetId(id);
		uiPython->AddWidget(wnd.get());
		auto result = wnd.get();

		parent->Add(std::move(wnd));

		return result;
		} , py::return_value_policy::reference);

	m.def("_add_button", [](const std::string& parentId, const std::string& id, int w=-1, int h=-1)->WidgetButton* {
		if (uiPython->GetWidget(id) != nullptr) {
			logger->error("_add_button: Widget id={} already exists", id);
			return nullptr;
		}
		auto parent = GetContainer(parentId);
		if (!parent) {
			logger->error("_add_button: Parent id={} does not exist", id);
			return nullptr;
		}
		
		auto btn = std::make_unique<WidgetButton >();
		btn->SetId(id);
		if (w > 0 && h > 0) {
			btn->SetSize({ w,h });
		}
		auto result = btn.get();

		uiPython->AddWidget(btn.get());
		parent->Add(std::move(btn));

		return result;
		}, py::arg("parent_id"), py::arg("id"), py::arg("width") = -1, py::arg("height") = -1,py::return_value_policy::reference);


	m.def("_get_container", &GetContainer , py::return_value_policy::reference);

	m.def("_get_button", &GetButton, py::return_value_policy::reference);

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
		.def("hide", [](WidgetBase& self) {
				self.Hide();
			})
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
		//.def(py::init<int, int>(), py::arg("width") = 0, py::arg("height") = 0)
		/*.def("add_child", [](WidgetContainer & me, WidgetBase * childWidget)->void {
			if (childWidget->GetParent()) {
				logger->error("child widget is already parented!");
				return;
			}

			me.Add( std::unique_ptr<WidgetBase>( childWidget) );
			})*/
		.def("add_image", [](WidgetContainer& self, const std::string& path) ->WidgetImage*{
			
			if (!tig->GetRenderingDevice().GetTextures().Resolve(path, false)->IsValid()) {
				logger->error("add_image: Invalid image path");
				return nullptr;
			}
			auto img = std::make_unique<WidgetImage>(path);
			auto result = img.get();
			self.AddContent(std::move(img));
			return result;
			}, py::return_value_policy::reference)
		.def("add_text", [](WidgetContainer& self, const std::string& text, const std::string & textStyleId)->WidgetText* 
			{
				auto txt = std::make_unique<WidgetText>(text, textStyleId);
				auto result = txt.get();
				self.AddContent(std::move(txt));
				return result; 
			}, 
			py::arg("text") = "", py::arg("text_style_id") = "", py::return_value_policy::reference)
		/*.def("add_content", [](WidgetContainer& me, WidgetContent* content)->void {
				
				me.AddContent(std::unique_ptr<WidgetContent>(content));
			})*/
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
		.def("set_style_id", [](WidgetButton& self, const std::string & styleName) {
			eastl::string s(styleName.c_str());
			self.SetStyle(s);
			})
		.def("set_text", &WidgetButton::SetText)
		.def("set_click_handler", [](WidgetButton &self, std::function<void()> funcy) {
			self.SetClickHandler(funcy);
			self.SetMouseMsgHandler([](const TigMouseMsg&) {return true; /* just so that it eats the message */ });
			})
		;


}