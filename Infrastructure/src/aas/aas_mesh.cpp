
#include <infrastructure/vfs.h>

#include "aas_mesh.h"

namespace aas {
	
	struct MeshDataHeader {
		int boneCount;
		int boneDataStart;
		int materialCount;
		int materialDataStart;
		int vertexCount;
		int vertexDataStart;
		int faceCount;
		int faceDataStart;
	};

	struct MeshMaterial {
		char id[128];
	};

	Mesh::Mesh(std::string filename, std::vector<uint8_t> data)
		: filename_(std::move(filename)), data_(std::move(data))
	{
		assert(data_.size() >= sizeof(MeshDataHeader));

		auto header = *reinterpret_cast<const MeshDataHeader*>(data_.data());

		assert(data_.size() >= header.boneDataStart + sizeof(MeshBone) * header.boneCount);
		assert(data_.size() >= header.materialDataStart + sizeof(MeshMaterial) * header.materialCount);
		assert(data_.size() >= header.vertexDataStart + sizeof(MeshVertex) * header.vertexCount);
		assert(data_.size() >= header.faceDataStart + sizeof(MeshFace) * header.faceCount);

		bones_ = std::span(reinterpret_cast<MeshBone*>(&data_[header.boneDataStart]), header.boneCount);
		
		auto materials = reinterpret_cast<MeshMaterial*>(&data_[header.materialDataStart]);
		materials_.resize(header.materialCount);
		for (size_t i = 0; i < materials_.size(); i++) {
			materials_[i] = std::string_view(materials[i].id);
		}

		vertices_ = std::span(reinterpret_cast<MeshVertex*>(&data_[header.vertexDataStart]), header.vertexCount);
		faces_ = std::span(reinterpret_cast<MeshFace*>(&data_[header.faceDataStart]), header.faceCount);


	}

	void Mesh::RenormalizeClothVertices(int clothBoneId)
	{

		for (auto& vertex : vertices_) {

			auto weight_sum = 0.0f;
			for (int j = 0; j < vertex.attachmentCount; j++) {
				// This is actually the "first cloth bone id" i guess...
				// TODO: This should actually use the bone mapping! The normalBoneCount comes from the
				// SKA file, while the attachment references SKM bones
				if (vertex.attachmentBone[j] == clothBoneId) {
					vertex.attachmentWeight[j] = 0.0f;
				}
				else {
					weight_sum += vertex.attachmentWeight[j];
				}
			}

			// Normalize remaining attachment weights
			if (weight_sum > 0.0f) {
				for (int j = 0; j < vertex.attachmentCount; j++) {
					vertex.attachmentWeight[j] /= weight_sum;
				}
			}
		}

	}

	std::unique_ptr<Mesh> LoadMeshFile(std::string_view filename)
	{
		auto buffer = vfs->ReadAsBinary(filename);

		return std::make_unique<Mesh>(std::string(filename), std::move(buffer));
	}

}
