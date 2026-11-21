#include "stdafx.h"
#include "container_art.h"
#include "gamesystems/objects/objsystem.h"
#include "inventory.h"
#include "gameview.h"
#include "raycast.h"
#include "tig/tig_mouse.h"
#include "obj.h"
#include "tig/tig_startup.h"
#include <graphics/shaperenderer3d.h>
#include <infrastructure/json11.hpp>
#include <infrastructure/vfs.h>

namespace ContainerArt {
	struct Polygon {
		std::vector<XMFLOAT3> points;
		float area;
	};
	static std::unordered_map<objHndl, Polygon> polygons;

	void Clear() {
		polygons.clear();
	}

	static bool IsPoint(const json11::Json &p) {
		return p.array_items().size() == 2 && p[0].is_number() && p[1].is_number();
	}

	static void Spill(objHndl handle, const json11::Json &point) {
		auto box = objSystem->GetObject(handle);
		if (box->GetFlags() & (OF_OFF | OF_DESTROYED) || box->GetInt32(obj_f_container_flags) & OCOF_LOCKED)
			return;
		std::vector<objHndl> items;
		box->ForEachChild([&](objHndl item) { if (item) items.push_back(item); });
		if (items.size() > 2) return;
		for (size_t i = 0; i < items.size(); ++i) {
			auto item = items[i];
			if (inventory.GetParent(item) != handle) continue;
			// Keep both stacks individually clickable, within the original pile's tile.
			auto delta = items.size() == 2 ? (i == 0 ? -0.22f : 0.22f) : 0.f;
			auto loc = LocAndOffsets::FromInches(
				(float(point[0].number_value()) + delta) * INCH_PER_TILE,
				(float(point[1].number_value()) - delta) * INCH_PER_TILE);
			inventory.ForceRemove(item, handle);
			auto body = objSystem->GetObject(item);
			if (body->GetInt32(obj_f_item_inv_location) != -1) continue;
			body->SetObjHndl(obj_f_item_parent, objHndl::null);
			objects.ClearFlag(item, OF_INVENTORY);
			objects.Move(item, loc);
		}
		// Hide only after successful transfers. Old saves retain looted items and their identities.
		if (box->GetInt32(obj_f_container_inventory_num) == 0) objects.SetFlag(handle, OF_OFF);
	}

	void Load(const std::string &directory) {
		Clear();
		auto path = directory + "\\container_art.json";
		if (!vfs->FileExists(path)) return;
		std::string error;
		auto data = json11::Json::parse(vfs->ReadAsString(path), error);
		if (!error.empty()) {
			logger->warn("Container art: {}: {}", path, error);
			return;
		}
		std::vector<objHndl> boxes;
		objSystem->ForEachObj([&](objHndl h, GameObjectBody &obj) {
			if (obj.type == obj_t_container) boxes.push_back(h);
		});
		unsigned piles = 0;
		for (auto h : boxes) {
			auto &entry = data[objSystem->GetObject(h)->id.ToString()];
			if (IsPoint(entry["pile"])) {
				Spill(h, entry["pile"]);
				++piles;
				continue;
			}
			Polygon polygon{};
			for (auto &p : entry["poly"].array_items()) {
				if (!IsPoint(p)) { polygon.points.clear(); break; }
				polygon.points.emplace_back(float(p[0].number_value()) * INCH_PER_TILE, 0.f,
					float(p[1].number_value()) * INCH_PER_TILE);
			}
			if (polygon.points.size() < 3) continue;
			const auto origin = polygon.points.front();
			for (size_t i = 1; i + 1 < polygon.points.size(); ++i) {
				auto &a = polygon.points[i], &b = polygon.points[i+1];
				polygon.area += (a.x-origin.x)*(b.z-origin.z) - (b.x-origin.x)*(a.z-origin.z);
			}
			polygon.area = std::abs(polygon.area) * 0.5f;
			if (polygon.area > 0) polygons.emplace(h, std::move(polygon));
		}
		logger->info("Container art: {}: {} small piles, {} painted containers", path, piles, polygons.size());
	}

	bool HasPolygon(objHndl h) { return polygons.find(h) != polygons.end(); }

	bool IsHovered(objHndl h) {
		// The viewport can scroll beneath a stationary mouse.
		if (!temple::GetRef<int>(0x102FC640)) return false; // cursor is over a UI widget
		auto pos = mouseFuncs.GetPos();
		auto world = gameView->ScreenToWorld(float(pos.x), float(pos.y));
		if (HitArea(h, world.x, world.z) == 0) return false;
		objHndl target;
		return PickObjectOnScreen(pos.x, pos.y, &target, GRF_HITTEST_3D) && target == h;
	}

	float HitArea(objHndl h, float x, float z) {
		const auto &poly = polygons.at(h);
		bool inside = false;
		auto prev = &poly.points.back();
		for (const auto &p : poly.points) {
			if ((p.z > z) != (prev->z > z) && x < (prev->x-p.x)*(z-p.z)/(prev->z-p.z)+p.x)
				inside = !inside;
			prev = &p;
		}
		return inside ? poly.area : 0.f;
	}

	void DrawOutline(objHndl h) {
		const auto &points = polygons.at(h).points;
		auto prev = &points.back();
		for (auto &p : points) {
			tig->GetShapeRenderer3d().DrawLineWithoutDepth(*prev, p, XMCOLOR(0.25f, 0.90f, 1.f, 1.f));
			prev = &p;
		}
	}
}
