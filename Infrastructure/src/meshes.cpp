
#include "infrastructure/meshes.h"
#include <fmt/format.h>
#include "graphics/collision.h"

#include <DirectXCollision.h>

using namespace gfx;
using namespace DirectX;

static const char* sBardInstrumentTypeNames[] = {
	"bard_flute",
	"bard_drum",
	"bard_mandolin",
	"bard_trumpet",
	"bard_harp",
	"bard_lute",
	"bard_pipes",
	"bard_recorder"
};

static const char* sWeaponTypeNames[] = {
	"unarmed",
	"dagger",
	"sword",
	"mace",
	"hammer",
	"axe",
	"club",
	"battleaxe",
	"greatsword",
	"greataxe",
	"greathammer",
	"spear",
	"staff",
	"polearm",
	"bow",
	"crossbow",
	"sling",
	"shield",
	"flail",
	"chain",
	"2hflail",
	"shuriken",
	"monk"
};

static const char* sWeaponAnimNames[] = {
	"none",
	"rattack",
	"rattack2",
	"rattack3",
	"lattack",
	"lattack2",
	"lattack3",
	"walk",
	"run",
	"idle",
	"fhit",
	"fhit2",
	"fhit3",
	"lhit",
	"lhit2",
	"lhit3",
	"rhit",
	"rhit2",
	"rhit3",
	"bhit",
	"bhit2",
	"bhit3",
	"rcriticalswing",
	"lcriticalswing",
	"fidget",
	"fidget2",
	"fidget3",
	"sneak",
	"panic",
	"rcombatstart",
	"lcombatstart",
	"combatidle",
	"combatfidget",
	"special1",
	"special2",
	"special3",
	"fdodge",
	"rdodge",
	"ldodge",
	"bdodge",
	"rthrow",
	"lthrow",
	"lsnatch",
	"rsnatch",
	"lturn",
	"rturn"
};

static const char* sNormalAnimNames[] = {
	"falldown",
	"prone_idle",
	"prone_fidget",
	"getup",
	"magichands",
	"picklock",
	"picklock_concentrated",
	"examine",
	"throw",
	"death",
	"death2",
	"death3",
	"dead_idle",
	"dead_fidget",
	"death_prone_idle",
	"death_prone_fidget",
	"abjuration_casting",
	"abjuration_conjuring",
	"conjuration_casting",
	"conjuration_conjuring",
	"divination_casting",
	"divination_conjuring",
	"enchantment_casting",
	"enchantment_conjuring",
	"evocation_casting",
	"evocation_conjuring",
	"illusion_casting",
	"illusion_conjuring",
	"necromancy_casting",
	"necromancy_conjuring",
	"transmutation_casting",
	"transmutation_conjuring",
	"conceal",
	"conceal_idle",
	"unconceal",
	"item_idle",
	"item_fidget",
	"open",
	"close",
	"skill_animal_empathy",
	"skill_disable_device",
	"skill_heal",
	"skill_heal_concentrated",
	"skill_hide",
	"skill_hide_idle",
	"skill_hide_fidget",
	"skill_unhide",
	"skill_pickpocket",
	"skill_search",
	"skill_spot",
	"feat_track",
	"trip",
	"bullrush",
	"flurry",
	"kistrike",
	"tumble",
	"special1",
	"special2",
	"special3",
	"special4",
	"throw",
	"wand_abjuration_casting",
	"wand_abjuration_conjuring",
	"wand_conjuration_casting",
	"wand_conjuration_conjuring",
	"wand_divination_casting",
	"wand_divination_conjuring",
	"wand_enchantment_casting",
	"wand_enchantment_conjuring",
	"wand_evocation_casting",
	"wand_evocation_conjuring",
	"wand_illusion_casting",
	"wand_illusion_conjuring",
	"wand_necromancy_casting",
	"wand_necromancy_conjuring",
	"wand_transmutation_casting",
	"wand_transmutation_conjuring",
	"skill_barbarian_rage",
	"open_idle"
};

static const char* GetBardInstrumentTypeName(BardInstrumentType instrumentType) {
	return sBardInstrumentTypeNames[(int)instrumentType];
}

static const char* GetNormalAnimTypeName(NormalAnimType animType) {
	auto idx = (int)animType;
	if (idx >= 79) {
		idx = 0;
	}
	return sNormalAnimNames[idx];
}

static const char* GetWeaponAnimName(WeaponAnim weaponAnim) {
	return sWeaponAnimNames[(int)weaponAnim];
}

static const char* GetWeaponTypeName(WeaponAnimType weaponAnimType) {
	return sWeaponTypeNames[(int)weaponAnimType];
}

bool EncodedAnimId::IsConjuireAnimation() const
{
	if (IsSpecialAnim()) {
		return false;
	}

	auto normalAnim = GetNormalAnimType();

	switch (normalAnim) {
	case NormalAnimType::AbjurationConjuring:
	case NormalAnimType::ConjurationConjuring:
	case NormalAnimType::DivinationConjuring:
	case NormalAnimType::EnchantmentConjuring:
	case NormalAnimType::EvocationConjuring:
	case NormalAnimType::IllusionConjuring:
	case NormalAnimType::NecromancyConjuring:
	case NormalAnimType::TransmutationConjuring:
	case NormalAnimType::WandAbjurationConjuring:
	case NormalAnimType::WandConjurationConjuring:
	case NormalAnimType::WandDivinationConjuring:
	case NormalAnimType::WandEnchantmentConjuring:
	case NormalAnimType::WandEvocationConjuring:
	case NormalAnimType::WandIllusionConjuring:
	case NormalAnimType::WandNecromancyConjuring:
	case NormalAnimType::WandTransmutationConjuring:
		return true;
	default:
		return false;
	}
}

std::string gfx::EncodedAnimId::GetName() const {

	static std::string sUnknown = "<unknown>";

	if (IsWeaponAnim()) {
		return fmt::format("{}_{}_{}",
		                   GetWeaponTypeName(GetWeaponLeftHand()),
		                   GetWeaponTypeName(GetWeaponRightHand()),
		                   GetWeaponAnimName(GetWeaponAnim())
		);
	}

	if (IsBardInstrumentAnim()) {
		return GetBardInstrumentTypeName(GetBardInstrumentType());
	}

	return GetNormalAnimTypeName(GetNormalAnimType());

}

bool gfx::AnimatedModel::HitTestRay(const AnimatedModelParams & params, const Ray3d & ray, float &hitDistance)
{
	
	auto origin = XMLoadFloat3(&ray.origin);
	auto direction = XMVector3Normalize(XMLoadFloat3(&ray.direction));

	auto submeshes = GetSubmeshes();

	bool hit = false;
	hitDistance = std::numeric_limits<float>::max();

	for (size_t i = 0; i < submeshes.size(); i++) {
		auto submesh = GetSubmesh(params, i);
		auto positions = submesh->GetPositions();
		auto indices = submesh->GetIndices();

		for (int j = 0; j < submesh->GetPrimitiveCount(); j++) {
			
			auto v0 = XMLoadFloat4(&positions[indices[j * 3]]);
			auto v1 = XMLoadFloat4(&positions[indices[j * 3 + 1]]);
			auto v2 = XMLoadFloat4(&positions[indices[j * 3 + 2]]);

			float dist;
			if (TriangleTests::Intersects(origin, direction, v0, v1, v2, dist) && dist < hitDistance) {
				hitDistance = dist;
				hit = true;
			}
		}
	}


	return hit;

}

// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
static void Barycentric(XMVECTOR p, XMVECTOR a, XMVECTOR b, XMVECTOR c, float &u, float &v, float &w)
{
	using namespace DirectX;
	auto v0 = b - a, v1 = c - a, v2 = p - a;
	auto d00 = XMVector3Dot(v0, v0);
	auto d01 = XMVector3Dot(v0, v1);
	auto d11 = XMVector3Dot(v1, v1);
	auto d20 = XMVector3Dot(v2, v0);
	auto d21 = XMVector3Dot(v2, v1);
	auto denom = XMVector3Dot(d00, d11) - XMVector3Dot(d01, d01);
	v = XMVectorGetX((XMVector3Dot(d11, d20) - XMVector3Dot(d01, d21)) / denom);
	w = XMVectorGetX((XMVector3Dot(d00, d21) - XMVector3Dot(d01, d20)) / denom);
	u = 1.0f - v - w;
}

static bool IsInTriangle(XMVECTOR p, XMVECTOR a, XMVECTOR b, XMVECTOR c) {
	float u, v, w;
	Barycentric(p, a, b, c, u, v, w);
	return u >= 0 && u <= 1
		&& v >= 0 && v <= 1
		&& w >= 0 && w <= 1;
}

// Returns the distance of "p" from the line p1->p2
static float DistanceFromLine(XMVECTOR p1, XMVECTOR p2, XMVECTOR p) {
	// Project the point P onto the line going through V0V1
	auto edge1 = p2 - p1;
	auto edge1LenVec = XMVector3Length(edge1);
	auto edge1Norm = edge1 / edge1LenVec;
	auto projFactor = XMVectorGetX(XMVector3Dot(p - p1, edge1Norm));
	if (projFactor >= 0 && projFactor < XMVectorGetX(edge1LenVec)) {
		// If projFactor < 0 or > the length of V0V1, it's outside the line
		auto pp = p1 + projFactor * edge1Norm;
		return XMVectorGetX(XMVector3Length(pp - p));
	} else {
		return std::numeric_limits<float>::max();
	}
}

// Originally @ 0x1001E220
float gfx::AnimatedModel::GetDistanceToMesh(const AnimatedModelParams & params, DirectX::XMFLOAT3 pos)
{
	using namespace DirectX;

	float closestDist = std::numeric_limits<float>::max();

	auto p = XMLoadFloat3(&pos);
	
	auto submeshes = GetSubmeshes();

	for (size_t i = 0; i < submeshes.size(); i++) {
		auto submesh = GetSubmesh(params, i);
		auto positions = submesh->GetPositions();
		auto indices = submesh->GetIndices();

		// Get the closest distance to any of the vertices
		for (auto &vertexPos : positions) {
			auto vertexDist = XMVectorGetX(XMVector3Length(XMLoadFloat4(&vertexPos) - p));
			if (vertexDist < closestDist) {
				closestDist = vertexDist;
			}
		}

		for (auto j = 0; j < submesh->GetPrimitiveCount(); j++) {
			auto v0 = XMLoadFloat4(&positions[indices[j * 3]]);
			auto v1 = XMLoadFloat4(&positions[indices[j * 3 + 1]]);
			auto v2 = XMLoadFloat4(&positions[indices[j * 3 + 2]]);

			// Compute the surface normal
			auto n = XMVector3Normalize(XMVector3Cross(v1 - v0, v2 - v0));

			// Project the point into the plane of the triangle
			auto distFromPlaneVec = XMVector3Dot(p - v0, n);
			auto distFromPlane = XMVectorGetX(distFromPlaneVec);
			auto projectedPos = p - distFromPlane * n;

			// If the point is within the triangle when projected onto it using the
			// plane's normal, then use the distance from the plane as the distance
			if (IsInTriangle(projectedPos, v0, v1, v2)) {
				if (fabsf(distFromPlane) < closestDist) {
					closestDist = fabsf(distFromPlane);
				}
			} else {

				// Project the point P onto the line going through V0V1
				float edge1Dist = DistanceFromLine(v0, v1, p);
				if (edge1Dist < closestDist) {
					closestDist = edge1Dist;
				}

				float edge2Dist = DistanceFromLine(v0, v2, p);
				if (edge2Dist < closestDist) {
					closestDist = edge2Dist;
				}

				float edge3Dist = DistanceFromLine(v2, v1, p);
				if (edge3Dist < closestDist) {
					closestDist = edge3Dist;
				}
			}
			
		}

	}

	return closestDist;

}
