
#include "stdafx.h"

#include <infrastructure/meshes.h>

#include "froggrapplecontroller.h"
#include "condition.h"
#include "critter.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"

struct GrappleState {
	uint16_t state;
	float currentLength;
	float targetLength;
};

#pragma pack(push, 1)
struct TongueVertex {
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
};
#pragma pack(pop)

using namespace gfx;

FrogGrappleController::FrogGrappleController(gfx::RenderingDevice & device, gfx::MdfMaterialFactory & mdfFactory)
	: mDevice(device)
{
	mTongueMaterial = mdfFactory.LoadMaterial("art/meshes/Monsters/GiantFrog/tongue.mdf");

	mVertexBuffer = device.CreateEmptyVertexBuffer(sizeof(TongueVertex) * VertexCount);

	CreateIndexBuffer();

	mBufferBinding.AddBuffer(mVertexBuffer, 0, sizeof(TongueVertex))
		.AddElement(VertexElementType::Float3, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Float3, VertexElementSemantic::Normal)
		.AddElement(VertexElementType::Float3, VertexElementSemantic::TexCoord);

}

bool FrogGrappleController::IsGiantFrog(objHndl obj) const
{
	// Special rendering for giant frogs of various types
	auto protoNum = gameSystems->GetObj().GetProtoId(obj);
	if (protoNum != 14057 && protoNum != 14445 && protoNum != 14300) {
		return false;
	}
	return true;
}

void FrogGrappleController::AdvanceAndRender(objHndl giantFrog,
	const gfx::AnimatedModelParams & animParams, 
	gfx::AnimatedModel &model, 
	gsl::span<gfx::Light3d> lights,
	float alpha)
{

	auto grappleState = GetGrappleState(giantFrog);

	if (!grappleState.state) {
		return;
	}

	XMFLOAT4X4 worldMatrixFrog;
	model.GetBoneWorldMatrixByName(animParams, "Tongue_Ref", &worldMatrixFrog);

	auto grappledOpponent = GetGrappledOpponent(giantFrog);
	float tongueLength, tonguePosX, tonguePosZ;
	if (grappledOpponent) {
		auto opponentModel = objects.GetAnimHandle(grappledOpponent);
		auto opponentAnimParams = objects.GetAnimParams(grappledOpponent);
		XMFLOAT4X4 worldMatrixOpponent;
		opponentModel->GetBoneWorldMatrixByName(opponentAnimParams, "Bip01 Spine1", &worldMatrixOpponent);

		tonguePosX = worldMatrixOpponent._41;
		tonguePosZ = worldMatrixOpponent._43;

		auto tongueDirX = worldMatrixOpponent._41 - worldMatrixFrog._41;
		auto tongueDirY = worldMatrixOpponent._42 - worldMatrixFrog._42;
		auto tongueDirZ = worldMatrixOpponent._43 - worldMatrixFrog._43;
		tongueLength = sqrt(tongueDirX * tongueDirX + tongueDirY * tongueDirY + tongueDirZ * tongueDirZ);
		worldMatrixFrog._31 = tongueDirX / tongueLength;
		worldMatrixFrog._32 = tongueDirY / tongueLength;
		worldMatrixFrog._33 = tongueDirZ / tongueLength;
		if (tongueLength > 0) {
			tongueLength -= 6.0f;
		}
	}
	else {
		tongueLength = 120.0f;
		tonguePosX = worldMatrixFrog._31 * 120.0f + worldMatrixFrog._41;
		tonguePosZ = worldMatrixFrog._33 * 120.0f + worldMatrixFrog._43;
	}

	switch (grappleState.state)
	{
		// This state seems to mean -> Extending tongue to full length
	case 1u:
		grappleState.currentLength += INCH_PER_TILE;
		if (grappleState.currentLength > tongueLength) {
			grappleState.state = 2;
			grappleState.currentLength = tongueLength;
		}
		break;
		// This state seems to mean -> Retracting tongue
	case 2u:
		grappleState.currentLength -= INCH_PER_TILE;
		if (grappleState.currentLength <= 0) {
			grappleState.state = 0;
			grappleState.currentLength = 0;
		}
		break;
	case 3u:
		grappleState.currentLength += INCH_PER_TILE;
		if (grappleState.currentLength > tongueLength)
		{
			grappleState.state = 4;
			grappleState.currentLength = tongueLength;
			auto frogAnim = critterSys.GetAnimId(giantFrog,
				gfx::WeaponAnim::Special2);
			objects.SetAnimId(giantFrog, frogAnim);

			auto opponentAnim = critterSys.GetAnimId(grappledOpponent,
				gfx::WeaponAnim::Panic);
			objects.SetAnimId(grappledOpponent, opponentAnim);
		}
		break;
	case 4u:
		// Maintain Tongue between frog and opponent without progressing
		grappleState.currentLength = tongueLength;
		break;
	case 5u:
		grappleState.targetLength = tongueLength - 12.0f;
		if (grappleState.targetLength < 0) {
			grappleState.targetLength = 0;
		}
		grappleState.state = 6;
		// Note: falls through
	case 6u:
	{
		grappleState.currentLength = grappleState.currentLength - INCH_PER_HALFTILE;
		// Move the opponent closer to the frog
		float newX = tonguePosX - worldMatrixFrog._31 * INCH_PER_HALFTILE;
		float newZ = tonguePosZ - worldMatrixFrog._33 * INCH_PER_HALFTILE;
		auto newLoc = LocAndOffsets::FromInches(newX, newZ);
		objects.Move(grappledOpponent, newLoc);

		if (grappleState.currentLength < grappleState.targetLength) {
			newX = worldMatrixFrog._41 + grappleState.targetLength * worldMatrixFrog._31;
			newZ = worldMatrixFrog._43 + grappleState.targetLength * worldMatrixFrog._33;
			newLoc = LocAndOffsets::FromInches(newX, newZ);
			objects.Move(grappledOpponent, newLoc);
			grappleState.currentLength = grappleState.targetLength;
			grappleState.state = 4;
		}
	}
	break;
	case 7u:
	{
		grappleState.currentLength = grappleState.currentLength - INCH_PER_HALFTILE;
		// Move the opponent closer to the frog
		float newX = tonguePosX - worldMatrixFrog._31 * INCH_PER_HALFTILE;
		float newZ = tonguePosZ - worldMatrixFrog._33 * INCH_PER_HALFTILE;
		auto newLoc = LocAndOffsets::FromInches(newX, newZ);
		objects.Move(grappledOpponent, newLoc);

		if (grappleState.currentLength < 0) {
			newX = worldMatrixFrog._41;
			newZ = worldMatrixFrog._43;
			newLoc = LocAndOffsets::FromInches(newX, newZ);
			objects.Move(grappledOpponent, newLoc);
			objects.FadeTo(grappledOpponent, 0, 0, 16, 0);
			grappleState.currentLength = 0;
			grappleState.state = 0;

			// Probably the swallow animation
			auto animId = critterSys.GetAnimId(giantFrog, gfx::WeaponAnim::Special3);
			objects.SetAnimId(giantFrog, animId);
		}
	}
	break;
	default:
		break;
	}

	// Update to the new grapple state
	SetGrappleState(giantFrog, grappleState);

	RenderTongue(grappleState, worldMatrixFrog, lights, alpha);

}

void FrogGrappleController::CreateIndexBuffer()
{

	// 12 tris are needed to fill the space between two discs
	// which comes down to 15 * 12 = 180 tris overall, each needing
	// 3 indices
	std::array<uint16_t, TriCount * 3> indices;
	size_t i = 0;
	for (auto disc = 0; disc < 15; ++disc) {
		auto discFirst = disc * 6; // Index of first vertex in the current disc
		auto nextDiscFirst = (disc + 1) * 6; // Index of first vertex in the next disc

		for (auto segment = 0; segment < 5; ++segment) {
			indices[i++] = discFirst + segment;
			indices[i++] = discFirst + segment + 1;
			indices[i++] = nextDiscFirst + segment + 1;
			indices[i++] = discFirst + segment;
			indices[i++] = nextDiscFirst + segment + 1;
			indices[i++] = nextDiscFirst + segment;
		}

		// The last segment of this disc wraps back around 
		// and connects to the first segment
		indices[i++] = discFirst + 5;
		indices[i++] = discFirst;
		indices[i++] = nextDiscFirst;
		indices[i++] = discFirst + 5;
		indices[i++] = nextDiscFirst;
		indices[i++] = nextDiscFirst + 5;
	}

	mIndexBuffer = mDevice.CreateIndexBuffer(indices);

}

objHndl FrogGrappleController::GetGrappledOpponent(objHndl giantFrog)
{
	auto spFrogTongue = conds.GetByName("sp-Frog Tongue");
	auto spFrogTongueSwallowing = conds.GetByName("sp-Frog Tongue Swallowing");
	uint32_t condNameData = reinterpret_cast<uint32_t>(spFrogTongue);
	uint32_t condNameSwalloingData = reinterpret_cast<uint32_t>(spFrogTongueSwallowing);

	int spellIdx;
	if (d20Sys.d20QueryWithData(giantFrog, DK_QUE_Critter_Has_Condition, condNameData, 0) == 1) {
		spellIdx = (int)d20Sys.d20QueryReturnData(giantFrog, DK_QUE_Critter_Has_Condition, condNameData, 0);
	}
	else if (d20Sys.d20QueryWithData(giantFrog, DK_QUE_Critter_Has_Condition, condNameSwalloingData, 0) == 1) {
		spellIdx = (int)d20Sys.d20QueryReturnData(giantFrog, DK_QUE_Critter_Has_Condition, condNameSwalloingData, 0);
	}
	else {
		return objHndl::null; // Nothing attached
	}

	auto spell = spellsCastRegistry.get(spellIdx);
	if (!spell) {
		return objHndl::null;
	}

	return spell->spellPktBody.targetListHandles[0];
}

void FrogGrappleController::SetGrappleState(objHndl giantFrog, const GrappleState & state)
{
	uint32_t newGrappleState = ((uint8_t)state.targetLength) << 24 
		| ((uint8_t)state.currentLength) << 16 
		| state.state;
	objects.setInt32(giantFrog, obj_f_grapple_state, newGrappleState);
}

GrappleState FrogGrappleController::GetGrappleState(objHndl giantFrog)
{
	auto grappleState = objects.getInt32(giantFrog, obj_f_grapple_state);

	// Unpack the grapple state
	GrappleState result;
	result.state = grappleState & 0xFFFF;
	result.currentLength = (float)((grappleState >> 16) & 0xFF);
	result.targetLength = (float)(grappleState >> 24);
	return result;
}

void FrogGrappleController::RenderTongue(const GrappleState &grappleState,
	const XMFLOAT4X4 &worldMatrixOrigin,
	gsl::span<gfx::Light3d> lights,
	float alpha) {

	std::array<TongueVertex, VertexCount> vertices;

	// The directional vector of the tongue ref point on the frog
	auto tongueDir = XMLoadFloat3((XMFLOAT3*)&worldMatrixOrigin._31);
	auto tongueUp = XMLoadFloat3((XMFLOAT3*)&worldMatrixOrigin._11);
	auto tongueRight = XMLoadFloat3((XMFLOAT3*)&worldMatrixOrigin._21);
	auto tonguePos = XMLoadFloat3((XMFLOAT3*)&worldMatrixOrigin._41);

	// Generate 16 "discs" along the path of the tongue
	for (size_t i = 0; i < 16; ++i) {

		// This function ranges from 1 at the origin of the tongue to 0 in the middle and
		// 1 at the end. It causes the tongue to appear slightly thinned out in the middle
		float radiusVariance = (cosf(XM_2PI * i / 15.0f) - 1.0f) * 0.5f;

		// This seems to be the radius of the tongue at this particular given disc
		float tongueRadius = 3.0f + radiusVariance * (grappleState.currentLength / 700.0f) * 3.0f;

		// Calculates the center point of the tongue along the directional vector
		// of the tongue_ref
		float distFromFrog = grappleState.currentLength * i / 15.0f;
		auto posFromOrig = tongueDir * distFromFrog;

		// Each disc has 6 corner points on the 
		// circle around the center of the tongue
		for (int j = 0; j < 6; ++j) {
			// The angle does a full rotation around the tongue's center
			float angle = XMConvertToRadians(360 / 6) * j;
			auto angleCos = cosf(angle);
			auto angleSin = sinf(angle);

			auto scaledSin = tongueRadius * angleSin;
			auto scaledCos = tongueRadius * angleCos;

			auto &vertex = vertices[i * 6 + j];

			auto pos = tonguePos + tongueRight * scaledSin + tongueUp * scaledCos + posFromOrig;
			XMStoreFloat3(&vertex.pos, pos);

			auto normal = tongueRight * angleSin + tongueUp * angleCos;
			XMStoreFloat3(&vertex.normal, normal);

			// The U texture coord just goes around the disc from 0 to 1
			vertex.uv.x = j / 5.0f;
			// Along the tongue's longer axis, the v coord just ranges from 0 
			// on the first disc to 1 on the last
			vertex.uv.y = i / 15.0f;
		}
	}

	mVertexBuffer->Update<TongueVertex>(vertices);

	mBufferBinding.Bind();
	MdfRenderOverrides overrides;
	overrides.alpha = alpha;
	mTongueMaterial->Bind(mDevice, lights, &overrides);
	mDevice.GetDevice()->SetIndices(mIndexBuffer->GetBuffer());

	mDevice.GetDevice()->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,
		0,
		0,
		VertexCount,
		0,
		TriCount);
}
