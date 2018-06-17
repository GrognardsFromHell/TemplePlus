
#include <infrastructure/exception.h>

#include "aas_cloth_sim.h"
#include "aas_cloth_collision.h"

namespace aas {
	
	AasClothStuff::~AasClothStuff()
	{
		delete[] clothVertexPos1;
		delete[] bytePerVertex;
		delete[] boneDistances;
		delete[] boneDistances2;
	}

	void AasClothStuff::UpdateBoneDistances()
	{
		memcpy(clothVertexPos3, clothVertexPos2, sizeof(DX::XMFLOAT4) * clothVertexCount);

		for (auto i = 0; i < boneDistancesCountDelta + boneDistancesCount; i++) {
			auto &distance = boneDistances[i];
			auto pos1 = clothVertexPos2[distance.to];
			auto pos2 = clothVertexPos2[distance.from];

			auto deltaX = pos1.x - pos2.x;
			auto deltaY = pos1.y - pos2.y;
			auto deltaZ = pos1.z - pos2.z;
			distance.distSquared = deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
		}

		for (auto i = 0; i < boneDistances2CountDelta + boneDistances2Count; i++) {
			auto &distance = boneDistances2[i];
			auto pos1 = clothVertexPos2[distance.to];
			auto pos2 = clothVertexPos2[distance.from];

			auto deltaX = pos1.x - pos2.x;
			auto deltaY = pos1.y - pos2.y;
			auto deltaZ = pos1.z - pos2.z;
			distance.distSquared = deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
		}
	}

	void AasClothStuff::Simulate(float time)
	{
		int step_count = (int)ceil(time * 200.0);
		float timePerStep;
		if (step_count <= 5) {
			timePerStep = time / step_count;
		}
		else {
			step_count = 5;
			timePerStep = 0.005f;
		}

		for (int i = 0; i < step_count; i++) {
			SimulateStep(timePerStep);
			ApplySprings();
			ApplySprings();
			HandleCollisions();
		}
	}

	void AasClothStuff::SimulateStep(float step_time)
	{
		if (field_34_time <= 0.0 || step_time <= 0.0)
		{
			field_34_time = step_time;
		}
		else
		{
			auto factor = (pow(2.0f, -step_time) * step_time + field_34_time) / field_34_time;

			for (auto i = 0u; i < clothVertexCount; i++) {
				if (bytePerVertex[i]) {
					clothVertexPos3[i] = clothVertexPos2[i];
				}
				else {
					auto &out = clothVertexPos3[i];
					auto &in = clothVertexPos2[i];

					out.x += (in.x - out.x) * factor;
					out.y += (in.y - out.y) * factor;
					out.z += (in.z - out.z) * factor;
					clothVertexPos3[i].y -= step_time * step_time * 2400.0f;
				}
			}

			std::swap(clothVertexPos2, clothVertexPos3);
			field_34_time = step_time;
		}

	}

	void AasClothStuff::ApplySprings()
	{

		for (auto i = 0; i < boneDistancesCount; i++) {
			auto &distance = boneDistances[i];

			auto &pos1 = clothVertexPos2[distance.to];
			auto &pos2 = clothVertexPos2[distance.from];

			auto deltaX = pos1.x - pos2.x;
			auto deltaY = pos1.y - pos2.y;
			auto deltaZ = pos1.z - pos2.z;

			auto factor = 2 * (distance.distSquared / (deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ + distance.distSquared) - 0.5f);

			pos2.x -= deltaX * factor;
			pos2.y -= deltaY * factor;
			pos2.z -= deltaZ * factor;
		}

		for (auto i = 0; i < boneDistancesCountDelta; i++) {

			auto &distance = boneDistances[boneDistancesCount + i];

			auto &pos1 = clothVertexPos2[distance.to];
			auto &pos2 = clothVertexPos2[distance.from];

			auto deltaX = pos1.x - pos2.x;
			auto deltaY = pos1.y - pos2.y;
			auto deltaZ = pos1.z - pos2.z;

			auto factor = (distance.distSquared / (deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ + distance.distSquared) - 0.5f);

			pos1.x += deltaX * factor;
			pos1.y += deltaY * factor;
			pos1.z += deltaZ * factor;
			pos2.x -= deltaX * factor;
			pos2.y -= deltaY * factor;
			pos2.z -= deltaZ * factor;
		}

	}

	void AasClothStuff::HandleCollisions()
	{

		for (auto i = 0u; i < clothVertexCount; i++) {

			if (!bytePerVertex[i]) {
				auto &pos = clothVertexPos2[i];
				if (pos.y < 0.0f) {
					pos.y = 0.0f;
				}

				for (auto sphere = spheres; sphere; sphere = sphere->next.get()) {
					auto posInSphere = transformPosition(sphere->worldMatrixInverse, pos);
					auto distanceFromOriginSq = DX::XMVectorGetX(DX::XMVector3LengthSq(DX::XMLoadFloat4(&posInSphere)));
					if (distanceFromOriginSq < sphere->radius * sphere->radius) {
						auto ratio = sphere->radius / sqrt(distanceFromOriginSq);
						posInSphere.x *= ratio;
						posInSphere.y *= ratio;
						posInSphere.z *= ratio;
						pos = transformPosition(sphere->worldMatrix, posInSphere);
					}
				}

				for (auto cylinder = cylinders; cylinder; cylinder = cylinder->next.get()) {
					auto posInCylinder = transformPosition(cylinder->worldMatrixInverse, pos);

					if (posInCylinder.z >= 0.0 && posInCylinder.z <= cylinder->height) {
						auto distanceFromOriginSq = posInCylinder.x * posInCylinder.x + posInCylinder.y * posInCylinder.y;
						if (distanceFromOriginSq < cylinder->radius * cylinder->radius) {
							auto ratio = cylinder->radius / sqrt(distanceFromOriginSq);
							posInCylinder.x *= ratio;
							posInCylinder.y *= ratio;

							pos = transformPosition(cylinder->worldMatrix, posInCylinder);
						}
					}
				}
			}

		}

	}

	void AasClothStuff::SetStuff(int clothVertexCount,
		DX::XMFLOAT4 *clothVertexPos,
		uint8_t *bytePerClothVertex,
		int edgeCount,
		EdgeDistance *edges,
		int indirectEdgeCount,
		EdgeDistance *indirectEdges)
	{

		if (this->clothVertexCount)
		{
			throw TempleException("Cloth state already initialized");
		}

		this->clothVertexCount = clothVertexCount;
		auto vertexBuffer = new DX::XMFLOAT4[2 * clothVertexCount];

		this->clothVertexPos1 = vertexBuffer;
		this->clothVertexPos2 = vertexBuffer;
		this->clothVertexPos3 = &vertexBuffer[clothVertexCount];

		memcpy(this->clothVertexPos1, clothVertexPos, sizeof(DX::XMFLOAT4) * clothVertexCount);
		memcpy(this->clothVertexPos3, clothVertexPos, sizeof(DX::XMFLOAT4) * clothVertexCount);

		this->bytePerVertex = new uint8_t[clothVertexCount];
		memcpy(this->bytePerVertex, bytePerClothVertex, clothVertexCount);

		this->boneDistancesCount = 0;
		this->boneDistancesCountDelta = 0;
		this->boneDistances = 0;

		if (edgeCount)
		{
			this->boneDistances = new EdgeDistance[edgeCount];
			memcpy(this->boneDistances, edges, sizeof(EdgeDistance) * edgeCount);

			SplitEdges(
				this->boneDistances,
				bytePerClothVertex,
				edgeCount,
				&this->boneDistancesCount,
				&this->boneDistancesCountDelta);
		}

		this->boneDistances2Count = 0;
		this->boneDistances2CountDelta = 0;
		this->boneDistances2 = 0;
		if (indirectEdgeCount)
		{
			this->boneDistances2 = new EdgeDistance[indirectEdgeCount];
			memcpy(this->boneDistances2, indirectEdges, sizeof(EdgeDistance) * indirectEdgeCount);

			SplitEdges(
				this->boneDistances2,
				bytePerClothVertex,
				indirectEdgeCount,
				&this->boneDistances2Count,
				&this->boneDistances2CountDelta);
		}
		field_34_time = 0.0f;

	}

	void AasClothStuff::SetSpheres(const CollisionSphere * head)
	{
		this->spheres = head;
	}

	void AasClothStuff::SetCylinders(const CollisionCylinder * head)
	{
		this->cylinders = head;
	}

	void AasClothStuff::SplitEdges(EdgeDistance *edges, uint8_t *bytePerClothVertex, int edgeCount, int *boneCountOut, int *boneCountOutDelta)
	{
		auto firstBucketCount = 0;
		auto edgesProcessed = 0;
		if (edgeCount > 0)
		{
			auto edgesItB = edges;
			auto edgesBack = &edges[edgeCount - 1];
			auto edgesItA = edgesItB;
			while (edgesProcessed < edgeCount)
			{
				if (!bytePerClothVertex[edgesItB->to]) {
					if (bytePerClothVertex[edgesItB->from])
					{
						// Swap vertices of edgesitb so that the first vertex is the one with flag=1
						std::swap(edgesItB->to, edgesItB->from);

						// Then swap edgesItB with edgesItA
						std::swap(*edgesItA, *edgesItB);
						++firstBucketCount;
						++edgesItA;
					}
					++edgesProcessed;
					++edgesItB;
					continue;
				}
				if (!bytePerClothVertex[edgesItB->from])
				{
					// Swap EdgesitA with EdgesitB
					std::swap(*edgesItA, *edgesItB);

					++firstBucketCount;
					++edgesItA;
					++edgesProcessed;
					++edgesItB;
					continue;
				}
				// Removes the edge at edge iterator position B
				--edgeCount;
				*edgesItB = *edgesBack;
				--edgesBack;
			}
		}

		*boneCountOut = firstBucketCount;
		*boneCountOutDelta = edgeCount - firstBucketCount;
	}

}
