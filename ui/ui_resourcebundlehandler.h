
#pragma once

#include <include/cef_resource_bundle_handler.h>
#include <include/cef_pack_strings.h>
#include <include/cef_pack_resources.h>

class UiResourceBundleHandler : public CefResourceBundleHandler {
public:
	bool GetLocalizedString(int message_id, CefString& string);

	bool GetDataResource(int resource_id, void*& data, size_t& data_size);
private:
	IMPLEMENT_REFCOUNTING(UiResourceBundleHandler)
};
