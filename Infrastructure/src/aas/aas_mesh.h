
#pragma once

#include <gsl/gsl>
#include <string>
#include <memory>
#include <vector>
#include <string_view>

#include "aas/aas_math.h"

namespace aas {

	struct MeshVertex {
		static constexpr size_t sMaxBoneAttachments = 6;
		DX::XMFLOAT4 pos;
		DX::XMFLOAT4 normal;
		DX::XMFLOAT2 uv;
		uint16_t padding;
		uint16_t attachmentCount;
		uint16_t attachmentBone[sMaxBoneAttachments];
		float attachmentWeight[sMaxBoneAttachments];

		bool IsAttachedTo(int bone_idx) const {
			for (int i = 0; i < attachmentCount; i++) {
				if (attachmentBone[i] == bone_idx) {
					return true;
				}
			}
			return false;
		}
	};

	struct MeshFace {
		static constexpr size_t sVertexCount = 3;
		int16_t material_idx;
		int16_t vertices[sVertexCount];
	};

	struct MeshBone {
		int16_t flags;
		int16_t parent_id;
		char name[48];
		Matrix3x4 full_world_inverse;
	};

	class Mesh {
	public:
		Mesh(std::string filename, std::vector<uint8_t> data);

		const std::vector<std::string_view> &GetMaterials() const {
			return materials_;
		}

		std::string_view GetMaterial(int materialIdx) const {
			return materials_[materialIdx];
		}

		gsl::span<MeshBone> GetBones() const {
			return bones_;
		}

		const MeshBone &GetBone(int boneIdx) const {
			Expects(boneIdx >= 0 && boneIdx < bones_.size());
			return bones_[boneIdx];
		}

		gsl::span<MeshVertex> GetVertices() const {
			return vertices_;
		}

		const MeshVertex &GetVertex(int vertexIdx) const {
			return vertices_[vertexIdx];
		}

		const size_t GetVertexCount() const {
			return vertices_.size();
		}

		MeshVertex &GetVertex(int vertexIdx) {
			return vertices_[vertexIdx];
		}

		gsl::span<MeshFace> GetFaces() const {
			return faces_;
		}

		const MeshFace &GetFace(int faceIdx) const {
			return faces_[faceIdx];
		}

		void RenormalizeClothVertices(int clothBoneId);

		const std::string &GetFilename() const {
			return filename_;
		}

	private:
		std::string filename_;
		std::vector<uint8_t> data_;
		std::vector<std::string_view> materials_;
		
		// These are views into the data buffer
		gsl::span<MeshBone> bones_;
		gsl::span<MeshVertex> vertices_;
		gsl::span<MeshFace> faces_;
	};

	std::unique_ptr<Mesh> LoadMeshFile(std::string_view filename);

}
