
#include "aas_cloth_sim.h"
#include "aas_cloth_sim_mesh.h"
#include "aas_mesh.h"

#include <fmt/format.h>
#include <fstream>

namespace aas {

	enum class ClothSimType : uint8_t {
		NONE = 0,
		DIRECT,
		INDIRECT
	};

	static bool find_edge(EdgeDistance *edges, int edges_count, int16_t from, int16_t to) {
		if (to < from) {
			std::swap(from, to);
		}

		for (int i = 0; i < edges_count; i++) {
			if (edges[i].from == from && edges[i].to == to) {
				return true;
			}
		}

		return false;
	}

	static void append_edge(EdgeDistance *edges, int &edges_count, int16_t from, int16_t to) {
		if (to < from) {
			std::swap(from, to);
		}

		if (find_edge(edges, edges_count, from, to)) {
			return;
		}

		auto &new_edge = edges[edges_count++];
		new_edge.to = to;
		new_edge.from = from;
		new_edge.distSquared = 1.0f;
	}

	static void build_edges2(EdgeDistance *edges, int edges_count, EdgeDistance *edges2, int &edges2_count) {

		for (int i = 0; i < edges_count - 1; i++) {
			auto firstEdge1 = edges[i].to;
			auto firstEdge2 = edges[i].from;

			for (auto next_edge_idx = i + 1; next_edge_idx < edges_count; next_edge_idx++)
			{
				auto secondEdge1 = edges[next_edge_idx].to;
				auto secondEdge2 = edges[next_edge_idx].from;

				if (firstEdge1 == secondEdge1) {
					if (find_edge(edges, edges_count, secondEdge2, firstEdge2)) {
						continue;
					}

					append_edge(edges2, edges2_count, secondEdge2, firstEdge2);

				}
				else if (firstEdge1 == secondEdge2) {
					if (find_edge(edges, edges_count, secondEdge1, firstEdge2)) {
						continue;
					}

					append_edge(edges2, edges2_count, secondEdge1, firstEdge2);

				}
				else if (firstEdge2 == secondEdge1) {
					if (find_edge(edges, edges_count, secondEdge2, firstEdge1)) {
						continue;
					}

					append_edge(edges2, edges2_count, secondEdge2, firstEdge1);

				}
				else if (firstEdge2 == secondEdge2) {
					if (find_edge(edges, edges_count, secondEdge1, firstEdge1)) {
						continue;
					}

					append_edge(edges2, edges2_count, secondEdge1, firstEdge1);
				}
			}
		}

	}

	void AasClothStuff1::Init(const Mesh &mesh,
		int clothBoneId,
		const CollisionSphere *collilsionSpheresHead,
		const CollisionCylinder *collisionCylindersHead)
	{

		this->mesh = &mesh;

		static ClothSimType vertex_sim_type[0x8000];
		static int mesh_vertex_idx_map[0x8000]; // Map from mesh vertex -> cloth vertex
		static int16_t cloth_vertex_idx_map[0x8000]; // Map from cloth vertex -> skm vertex
		static DX::XMFLOAT4 cloth_vertex_pos[0x8000];
		static uint8_t byte_per_cloth_vertex[0x8000];
		static uint8_t byte_per_cloth_vertex2[0x8000];
		auto cloth_vertex_count = 0;

		static EdgeDistance edges[0x8000];
		auto edges_count = 0;

		// Determine all vertices that are in some way influenced by the cloth simulation
		auto vertices = mesh.GetVertices();
		for (auto i = 0u; i < vertices.size(); i++) {
			auto &vertex = vertices[i];

			// TODO: Also, the bone mapping should be used here!
			if (vertex.IsAttachedTo(clothBoneId)) {
				// Establish the bidirectional mapping between the cloth state and the vertex
				mesh_vertex_idx_map[i] = cloth_vertex_count;
				cloth_vertex_idx_map[cloth_vertex_count] = i;
				cloth_vertex_pos[cloth_vertex_count] = vertex.pos;
				byte_per_cloth_vertex[cloth_vertex_count] = 0;
				byte_per_cloth_vertex2[cloth_vertex_count] = 1;
				cloth_vertex_count++;

				vertex_sim_type[i] = ClothSimType::DIRECT;

			}
			else {
				mesh_vertex_idx_map[i] = -1;
				vertex_sim_type[i] = ClothSimType::NONE;
			}
		}

		// Now process faces that incorporate vertices that are part of cloth sim
		for (auto &face : mesh.GetFaces()) {

			bool has_cloth_sim = false;
			for (int j = 0; j < face.sVertexCount; j++) {
				if (vertex_sim_type[face.vertices[j]] == ClothSimType::DIRECT) {
					has_cloth_sim = true;
					break;
				}
			}

			if (!has_cloth_sim) {
				continue; // Skip faces unaffected by cloth simulation
			}

			// For all vertices that are part of the face, but not directly part of cloth-sim
			// we have to record that they are indirectly part of cloth simulation
			for (int j = 0; j < face.sVertexCount; j++) {
				auto vertex_idx = face.vertices[j];
				if (vertex_sim_type[vertex_idx] == ClothSimType::NONE) {
					vertex_sim_type[vertex_idx] = ClothSimType::INDIRECT;
					mesh_vertex_idx_map[vertex_idx] = cloth_vertex_count;
					cloth_vertex_idx_map[cloth_vertex_count] = vertex_idx;
					cloth_vertex_pos[cloth_vertex_count] = mesh.GetVertex(vertex_idx).pos;
					byte_per_cloth_vertex[cloth_vertex_count] = 1;
					byte_per_cloth_vertex2[cloth_vertex_count] = 1;
					cloth_vertex_count++;
				}
			}

			// Handle the edges of the face (0->1, 1->2, 2->0)
			for (int fromIdx = 0; fromIdx < face.sVertexCount; fromIdx++) {
				int toIdx = (fromIdx + 1) % face.sVertexCount;

				auto from = face.vertices[fromIdx];
				auto to = face.vertices[toIdx];

				// Only consider edges where either end is directly participating in cloth simulation
				if (vertex_sim_type[from] != ClothSimType::DIRECT
					&& vertex_sim_type[to] != ClothSimType::DIRECT) {
					continue;
				}

				auto cloth_state_idx_from = mesh_vertex_idx_map[from];
				auto cloth_state_idx_to = mesh_vertex_idx_map[to];

				// Since the edge-state is commutative, we only record it for from<to,
				// and not the other way around
				if (cloth_state_idx_to < cloth_state_idx_from) {
					std::swap(cloth_state_idx_from, cloth_state_idx_to);
				}

				// Search if there's a record for the edge already
				bool found = false;
				for (int j = 0; j < edges_count; j++) {
					if (edges[j].from == cloth_state_idx_from && edges[j].to == cloth_state_idx_to) {
						found = true;
						break;
					}
				}

				if (!found) {
					auto from_pos = DX::XMLoadFloat4(&mesh.GetVertex(from).pos);
					auto to_pos = DX::XMLoadFloat4(&mesh.GetVertex(to).pos);
					auto dist_sq = DX::XMVectorGetX(DX::XMVector3LengthSq(DX::XMVectorSubtract(from_pos, to_pos)));

					auto &new_edge = edges[edges_count++];
					new_edge.from = cloth_state_idx_from;
					new_edge.to = cloth_state_idx_to;
					new_edge.distSquared = dist_sq;
				}
			}
		}

		static EdgeDistance edges2[0x8000];
		auto edges2_count = 0;
		build_edges2(edges, edges_count, edges2, edges2_count);

		clothStuff = new AasClothStuff;

		//static auto aas_cloth_stuff_set_stuff = temple::GetPointer<int __fastcall(AasClothStuff* self, int, unsigned int someCount, const DX::XMFLOAT4 *a3, uint8_t *b_per_cloth_vertex, int edgesCount, EdgeDistance *edges, int indirectEdgesCount, EdgeDistance *indirectEdges)>(0x10269bc0);
		//aas_cloth_stuff_set_stuff(cs1.clothStuff, 0,
		clothStuff->SetStuff(
			cloth_vertex_count,
			cloth_vertex_pos,
			byte_per_cloth_vertex,
			edges_count,
			edges,
			edges2_count,
			edges2
		);
		clothStuff->SetSpheres(collilsionSpheresHead);
		clothStuff->SetCylinders(collisionCylindersHead);
		clothVertexCount = cloth_vertex_count;
		bytePerClothVertex = new uint8_t[cloth_vertex_count];
		memcpy(bytePerClothVertex, byte_per_cloth_vertex, cloth_vertex_count);
		bytePerClothVertex2 = new uint8_t[cloth_vertex_count];
		memcpy(bytePerClothVertex2, byte_per_cloth_vertex2, cloth_vertex_count);
		vertexIdxForClothVertexIdx = new int16_t[cloth_vertex_count];
		memcpy(vertexIdxForClothVertexIdx, cloth_vertex_idx_map, cloth_vertex_count * sizeof(int16_t));


	}

	static int dump_count = 0;

	static void dumpClothStuff(const aas::AasClothStuff1 &cs1) {

		std::ofstream fout(fmt::format("cloth_out_{}.txt", dump_count++));

		fout << "=====================================" << std::endl;
		for (int i = 0; i < cs1.clothVertexCount; i++) {
			auto &vertex1 = cs1.clothStuff->clothVertexPos1[i];
			auto &vertex3 = cs1.clothStuff->clothVertexPos3[i];
			fout << fmt::format("skm={} b1={} b1*={} b2={} {},{},{} {},{},{}",
				cs1.vertexIdxForClothVertexIdx[i],
				cs1.bytePerClothVertex[i],
				cs1.clothStuff->bytePerVertex[i],
				cs1.bytePerClothVertex2[i],
				vertex1.x, vertex1.y, vertex1.z,
				vertex3.x, vertex3.y, vertex3.z);

			// Print the normalized bone attachments + weights
			auto &vertex = cs1.mesh->GetVertex(cs1.vertexIdxForClothVertexIdx[i]);
			fout << "[";
			for (int j = 0; j < vertex.attachmentCount; j++) {
				if (j > 0) {
					fout << ", ";
				}
				fout << vertex.attachmentBone[j] << "=" << vertex.attachmentWeight[j];
			}
			fout << "]\n";

		}

		AasClothStuff &cs = *cs1.clothStuff;
		fout << "=====================================" << std::endl;
		fout << fmt::format("c={} c2={}", cs.boneDistancesCount, cs.boneDistancesCountDelta) << std::endl;
		for (int i = 0; i < cs.boneDistancesCount + cs.boneDistancesCountDelta; i++) {
			auto &edge = cs.boneDistances[i];
			fout << fmt::format("{}->{} {}\r\n", edge.to, edge.from, edge.distSquared);
		}
		fout << "=====================================" << std::endl;
		fout << fmt::format("c={} c2={}", cs.boneDistances2Count, cs.boneDistances2CountDelta) << std::endl;
		for (int i = 0; i < cs.boneDistances2Count + cs.boneDistances2CountDelta; i++) {
			auto &edge = cs.boneDistances2[i];
			fout << fmt::format("{}->{} {}\r\n", edge.to, edge.from, edge.distSquared);
		}
		fout << "=====================================" << std::endl;
	}

}
