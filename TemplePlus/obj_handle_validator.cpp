#include "stdafx.h"
#include "obj_handle_validator.h"
#include "anomaly_notices.h"
#include "gamesystems/objects/objsystem.h"
#include "obj.h"
#include "description.h"
#include <unordered_set>
#include <unordered_map>
#include <deque>

ObjHandleValidator objHandleValidator;

static std::unordered_set<uint64_t> sLive;
static std::unordered_set<uint64_t> sFreed;     // freed since last clear (bounded)
static std::deque<uint64_t> sFreedOrder;
static const size_t kMaxFreed = 200000;

void ObjHandleValidator::OnAdd(uint64_t h) {
	sLive.insert(h);
	sFreed.erase(h);
}

void ObjHandleValidator::OnRemove(uint64_t h) {
	sLive.erase(h);
	if (sFreed.insert(h).second) {
		sFreedOrder.push_back(h);
		if (sFreedOrder.size() > kMaxFreed) { sFreed.erase(sFreedOrder.front()); sFreedOrder.pop_front(); }
	}
}

void ObjHandleValidator::OnClear() {
	// ObjRegistry::Clear leaks the bodies and only drops the id index; handles stay valid.
}

static std::string Describe(objHndl h) {
	if (!h) return "null";
	if (!objSystem->IsValidHandle(h)) return fmt::format("{:x}(invalid)", h.handle);
	auto obj = objSystem->GetObject(h);
	return fmt::format("{:x} '{}' id={}", h.handle, description.getDisplayName(h), obj->id.ToString());
}

static void Problem(const char *what, const char *site, void *caller, objHndl parent, int idx, objHndl item, const std::string &extra) {
	auto details = fmt::format("site={} caller={} parent={} slot={} item={} {}", site, caller, Describe(parent), idx, Describe(item), extra);
	logger->error("[debugObjHandles] {}: {}", what, details);
	auto key = fmt::format("{}:{}:{:x}:{:x}", what, site, parent.handle, item.handle);
	anomalyNotices.Report("objhandle", key, fmt::format("Inventory handle problem: {}", what), details);
}

int ObjHandleValidator::CheckInventory(uint64_t parentRaw, const char *site, void *caller) {
	if (!Enabled()) return 0;
	objHndl parent{ parentRaw };
	if (!parent || !objSystem->IsValidHandle(parent)) return 0;
	auto pobj = objSystem->GetObject(parent);
	if (!pobj) return 0;
	obj_f listF, numF;
	if (pobj->IsContainer()) { listF = obj_f_container_inventory_list_idx; numF = obj_f_container_inventory_num; }
	else if (pobj->IsCritter()) { listF = obj_f_critter_inventory_list_idx; numF = obj_f_critter_inventory_num; }
	else return 0;

	int problems = 0;
	int n = pobj->GetInt32(numF);
	int arrN = (int)pobj->GetObjectIdArray(listF).GetSize();
	if (n != arrN) {
		Problem("count mismatch", site, caller, parent, -1, objHndl::null, fmt::format("num={} array={}", n, arrN));
		problems++;
	}
	std::unordered_set<uint64_t> seen;
	for (int i = 0; i < n && i < arrN; i++) {
		auto item = pobj->GetObjHndl(listF, i);
		if (!item) { Problem("null slot", site, caller, parent, i, item, ""); problems++; continue; }
		if (!sLive.count(item.handle)) {
			bool freed = sFreed.count(item.handle) != 0;
			Problem(freed ? "dangling (freed) handle" : "unknown handle", site, caller, parent, i, item, ""); problems++; continue;
		}
		if (!seen.insert(item.handle).second) { Problem("duplicate item", site, caller, parent, i, item, ""); problems++; }
		auto iobj = objSystem->GetObject(item);
		if (!iobj->IsItem()) { Problem("non-item in inventory", site, caller, parent, i, item, ""); problems++; continue; }
		auto ip = iobj->GetObjHndl(obj_f_item_parent);
		if (ip != parent) {
			Problem("item parent mismatch", site, caller, parent, i, item, fmt::format("item_parent={}", Describe(ip))); problems++;
		}
	}
	return problems;
}

int ObjHandleValidator::CheckAll(const char *site) {
	if (!Enabled()) return 0;
	std::vector<objHndl> parents, items;
	objSystem->ForEachObj([&](objHndl h, GameObjectBody &obj) {
		if (obj.GetFlags() & OF_DESTROYED) return;
		if (obj.IsContainer() || obj.IsCritter()) parents.push_back(h);
		else if (obj.IsItem() && obj.GetObjHndl(obj_f_item_parent)) items.push_back(h);
	});
	int problems = 0;
	std::unordered_set<uint64_t> listed;
	for (auto p : parents) {
		problems += CheckInventory(p.handle, site);
		auto pobj = objSystem->GetObject(p);
		auto f = pobj->IsContainer() ? obj_f_container_inventory_list_idx : obj_f_critter_inventory_list_idx;
		auto arr = pobj->GetObjectIdArray(f);
		for (size_t i = 0; i < arr.GetSize(); i++) { auto it = pobj->GetObjHndl(f, i); if (it) listed.insert(it.handle); }
	}
	// item claims a (live) parent but is in nobody's list -> orphan (the "swapped" half of a swap)
	for (auto it : items) {
		if (listed.count(it.handle)) continue;
		auto ip = objSystem->GetObject(it)->GetObjHndl(obj_f_item_parent);
		if (!sLive.count(ip.handle)) { Problem("item parent dangling", site, nullptr, ip, -1, it, ""); problems++; continue; }
		Problem("orphan item (parent doesn't list it)", site, nullptr, ip, -1, it, ""); problems++;
	}
	logger->info("[debugObjHandles] {}: swept {} inventories, {} parented items, {} live handles, {} problems",
		site, parents.size(), items.size(), sLive.size(), problems);
	return problems;
}
