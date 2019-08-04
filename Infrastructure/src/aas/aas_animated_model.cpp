
#define _SCL_SECURE_NO_WARNINGS

#include <DirectXMath.h>
#include <fmt/format.h>
#include <infrastructure/exception.h>

#include "aas_animated_model.h"
#include "aas_mesh.h"
#include "aas_skeleton.h"
#include "aas_cloth_collision.h"
#include "aas_cloth_sim.h"
#include "aas_cloth_sim_mesh.h"
#include "aas_anim_player.h"

namespace aas {

	struct BoneElementCount {
		short position_count;
		short normal_count;
	};

	struct SubmeshVertexClothStateWithFlag {
		AasClothStuff1 *cloth_stuff1;
		short submesh_vertex_idx;
		short cloth_stuff_vertex_idx;
	};

	struct SubmeshVertexClothStateWithoutFlag {
		AasClothStuff1 *cloth_stuff1;
		short cloth_stuff_vertex_idx;
		short submesh_vertex_idx;
	};

	struct MeshMaterialPair {
		Mesh *mesh;
		int materialIdx;
	};

	struct AasSubmeshWithMaterial {
		AasMaterial materialId;
		IMaterialResolver* materialResolver;
		uint16_t bone_floats_count;
		uint16_t vertexCount;
		uint16_t primCount;
		uint16_t field_E;
		std::unique_ptr<BoneElementCount[]> bone_elem_counts;
		std::unique_ptr<float[]> bone_floats;
		std::unique_ptr<short[]> vertex_copy_positions;
		std::unique_ptr<DX::XMFLOAT2[]> uv;
		std::unique_ptr<DX::XMFLOAT4[]> positions;
		std::unique_ptr<DX::XMFLOAT4[]> normals;
		bool fullyInitialized; // This could actually be: NEEDS UPDATE! (TODO)
		std::vector<SubmeshVertexClothStateWithoutFlag> cloth_vertices_without_flag;
		std::vector<SubmeshVertexClothStateWithFlag> cloth_vertices_with_flag;
		std::unique_ptr<uint16_t[]> indices;

		AasSubmeshWithMaterial(AasMaterial materialId, IMaterialResolver *materialResolver) 
			: materialId(materialId), materialResolver(materialResolver) {}
		~AasSubmeshWithMaterial();

		AasSubmeshWithMaterial(AasSubmeshWithMaterial&& o) = default;
		AasSubmeshWithMaterial &operator=(AasSubmeshWithMaterial&& o) = default;

		int AttachMesh(Mesh *mesh, int materialIdx);
		void DetachMesh(Mesh *mesh);
		bool HasAttachedMeshes() const {
			return !meshes.empty();
		}
		const std::vector<MeshMaterialPair> &GetAttachedMeshes() const {
			return meshes;
		}

		void ResetState();

	private:
		std::vector<MeshMaterialPair> meshes;

	};

	struct BoneMapping {
		short skm_bone_idx;
		short ska_bone_idx;
	};

	/**
	* Originally @ 10265BB0
	* Builds a mapping from the bones of the SKM file to the bones in the SKA file, by comparing their names (ignoring case).
	* If a bone from the SKM cannot be found in the SKA, the mapping of the parent bone is used.
	* Returns the highest index of the SKA bone that was used in the mapping + 1. Presumably to allocate enough space for bone
	* matrices.
	*/
	static int build_bone_mapping(BoneMapping *bone_mapping, const Skeleton &skeleton, const Mesh &mesh) {

		auto bones = mesh.GetBones();

		auto ska_bone_count = 0;

		for (int i = 0; i < bones.size(); i++) {
			auto &bone = bones[i];

			auto effective_skm_bone = -1;
			int ska_bone_id = -1;

			auto skelBoneIdx = skeleton.FindBoneIdxByName(bone.name);
			if (skelBoneIdx != -1) {
				ska_bone_id = skelBoneIdx;
				effective_skm_bone = i;
			}

			// In case no bone mapping could be found, use the one for the parent
			if (effective_skm_bone == -1 && bone.parent_id >= 0)
			{
				effective_skm_bone = bone_mapping[bone.parent_id].skm_bone_idx;
				ska_bone_id = bone_mapping[bone.parent_id].ska_bone_idx;
			}

			if (ska_bone_id >= ska_bone_count) {
				ska_bone_count = ska_bone_id + 1;
			}

			bone_mapping[i].skm_bone_idx = effective_skm_bone;
			bone_mapping[i].ska_bone_idx = ska_bone_id;
		}

		return ska_bone_count;

	}

	struct AasVertexState {
		int16_t count;
		int16_t array1[6];
		int16_t array2[6];
		int16_t array3[6];
		int16_t field26; // padding??
		DX::XMFLOAT2 uv;
	};

	struct AasWorkSet {
		DX::XMFLOAT4 vector;
		float weight;
		int next;
	};

	int workset_add(AasWorkSet *scratch_space, int *scratch_space_size, const DX::XMFLOAT4 &vector, float bone_weight, int *heap_pos_out, int *some_count)
	{
		int insert_pos = 0;

		// Where to actually write the heap_pos
		auto pos_out_ = heap_pos_out;
		if (*heap_pos_out >= 0) {
			auto cur_heap_pos = *heap_pos_out;

			while (true) {
				auto &cur_entry = scratch_space[cur_heap_pos];

				if (distanceSquared3(cur_entry.vector, vector) == 0.0f && bone_weight == cur_entry.weight)
				{
					return insert_pos; // The position within the linked list of the bone apparently
				}
				cur_heap_pos = cur_entry.next;
				pos_out_ = &cur_entry.next;
				++insert_pos;
				if (cur_heap_pos == -1) {
					break; // Didn't find a matching entry in the linked list
				}
			}
		}

		auto heap_pos = *scratch_space_size;
		*pos_out_ = (*scratch_space_size)++;
		++*some_count;
		scratch_space[heap_pos].vector = vector;
		scratch_space[heap_pos].weight = bone_weight;
		scratch_space[heap_pos].next = -1;
		return insert_pos;
	}

	AnimatedModel::AnimatedModel() = default;

	AnimatedModel::~AnimatedModel()
	{
		while (runningAnimsHead) {
			auto running_anim = runningAnimsHead;
			if (running_anim->ownerAnim == this) {
				auto prev_anim = running_anim->prevRunningAnim;
				if (prev_anim) {
					prev_anim->nextRunningAnim = running_anim->nextRunningAnim;
				} else {
					runningAnimsHead = running_anim->nextRunningAnim;
				}
				auto next_anim = running_anim->nextRunningAnim;
				if (next_anim) {
					next_anim->prevRunningAnim = running_anim->prevRunningAnim;
				}
				running_anim->prevRunningAnim = 0;
				running_anim->nextRunningAnim = 0;
				running_anim->ownerAnim = 0;
			}
			delete running_anim;
		}
	}

	Matrix3x4 *AnimatedModel::GetBoneMatrix(std::string_view boneName, Matrix3x4 *matrixOut)
	{
		auto boneIdx = skeleton->FindBoneIdxByName(boneName);
		
		if (boneIdx != -1) {
			Method19();
			*matrixOut = boneMatrices[boneIdx + 1];
		} else {
			*matrixOut = Matrix3x4::identity();
		}
		
		return matrixOut;
	}

	float AnimatedModel::GetHeight()
	{
		SetClothFlagSth();
		Advance(currentWorldMatrix, 0.0f, 0.0f, 0.0f);

		auto maxHeight = -10000.0f;
		auto minHeight = 10000.0f;

		for (size_t i = 0; i < submeshes.size(); i++) {
			auto submesh = GetSubmesh(i);
			auto positions = submesh->GetPositions();

			for (auto j = 0; j < submesh->GetVertexCount(); j++) {
				auto y = positions[j].y;
				if (y < minHeight) {
					minHeight = y;
				}
				if (y > maxHeight) {
					maxHeight = y;
				}
			}
		}

		// No idea how they arrived at this value
		const auto defaultHeight = 28.8f;

		if (maxHeight == -10000.0f) {
			maxHeight = defaultHeight;
		}
		else if (maxHeight <= 0) {
			maxHeight = maxHeight - minHeight;
			if (maxHeight <= 0.01f) {
				maxHeight = defaultHeight;
			}
		}

		return maxHeight;
	}

	float AnimatedModel::GetRadius()
	{
		SetClothFlagSth();
		Advance(currentWorldMatrix, 0.0f, 0.0f, 0.0f);

		auto maxRadiusSquared = -10000.0f;

		for (size_t i = 0; i < submeshes.size(); i++) {
			auto submesh = GetSubmesh(i);
			auto positions = submesh->GetPositions();

			for (auto j = 0; j < submesh->GetVertexCount(); j++) {
				auto &pos = positions[j];

				// Distance from model origin (squared)
				auto distSq = pos.x * pos.x + pos.z * pos.z;

				if (distSq > maxRadiusSquared) {
					maxRadiusSquared = distSq;
				}
			}
		}

		// No idea how they arrived at this value
		if (maxRadiusSquared <= 0) {
			return 0;
		} else {
			return sqrtf(maxRadiusSquared);
		}
	}

	void AnimatedModel::SetSkeleton(std::shared_ptr<Skeleton> skeleton) {

		Expects(!this->skeleton);

		this->skeleton = skeleton;
		scale = 1.0f;
		scaleInv = 1.0f;
		variationCount = 1;
		variations[0].variationId = -1;
		variations[0].factor = 1.0f;

		auto bones = skeleton->GetBones();
		boneMatrices = std::make_unique<Matrix3x4[]>(bones.size() + 1);

		// count normal bones
		clothBoneId = skeleton->FindBoneIdxByName("#ClothBone");
		if (clothBoneId == -1) {
			clothBoneId = bones.size();
			hasClothBones = false;
		} else {
			hasClothBones = true;
		}
		cloth_stuff1_count = 0;

		if (!hasClothBones)
			return;

		// Discover the collision geometry used for cloth simulation
		auto collisionGeom = FindCollisionGeometry(bones);
		collisionSpheresHead = std::move(collisionGeom.firstSphere);
		collisionCylindersHead = std::move(collisionGeom.firstCylinder);

	}

	void AnimatedModel::SetScale(float scale)
	{
		this->scale = scale;
		if (scale <= 0.0f) {
			scaleInv = 0.0f;
		} else {
			scaleInv = 1.0f / scale;
		}
	}

	int AnimatedModel::GetBoneCount() const
	{
		return skeleton->GetBones().size();
	}

	std::string_view AnimatedModel::GetBoneName(int bone_idx) const
	{
		return &skeleton->GetBones()[bone_idx].name[0];
	}

	int AnimatedModel::GetBoneParentId(int bone_idx) const
	{
		return skeleton->GetBones()[bone_idx].parentId;
	}

	void AnimatedModel::AddMesh(Mesh *mesh, IMaterialResolver *matResolver)
	{
		
		/*auto org_method = temple::GetPointer<int __fastcall(IAnimatedModel*, void *, Mesh *, IAasMaterialResolver*, void *)>(0x10266a50);
		int x = org_method(this, 0, mesh, matResolver, matResolverArg);

		for (int i = 0; i < cloth_stuff1_count; i++) {
			if (cloth_stuff1[i].mesh == mesh) {
				dumpClothStuff(cloth_stuff1[i]);
			}
		}

		return x;*/

		ResetSubmeshes();

		// If the skeleton has cloth bones, normalize the vertex bone weights
		if (hasClothBones) {
			mesh->RenormalizeClothVertices(clothBoneId);
		}

		// Create a material group for every material in the SKM file and attach the SKM file to the group (in case a
		// material is used by multiple attached SKM files)
		auto &materials = mesh->GetMaterials();
		for (size_t i = 0; i < materials.size(); i++) {
			
			auto matHandle = matResolver->Acquire(materials[i], mesh->GetFilename());
			if (!matHandle) {
				throw TempleException("Failed to acquire material '{}' for mesh '{}'",
					materials[i], mesh->GetFilename());
			}
			
			auto submesh = GetOrAddSubmesh(matHandle, matResolver);
			Expects(submesh);

			if (submesh->AttachMesh(mesh, i) < 0) {
				throw TempleException("Failed to attach mesh '{}' with material index '{}' to material group.",
					mesh->GetFilename(), i);
			}
		}

		if (!hasClothBones) {
			return;
		}

		if (cloth_stuff1_count > 0) {
			auto new_cloth_stuff1 = new AasClothStuff1[cloth_stuff1_count + 1];
			std::copy_n(cloth_stuff1, cloth_stuff1_count, new_cloth_stuff1);
			delete[] cloth_stuff1;
			cloth_stuff1 = new_cloth_stuff1;
			cloth_stuff1_count++;
		} else {
			cloth_stuff1 = new AasClothStuff1[1];
			cloth_stuff1_count++;
		}

		auto &cs1 = cloth_stuff1[cloth_stuff1_count - 1];
		cs1.Init(*mesh, clothBoneId, collisionSpheresHead.get(), collisionCylindersHead.get());
	}
	
	void AnimatedModel::RemoveMesh(Mesh *mesh)
	{
		ResetSubmeshes();

		for (auto it = submeshes.begin(); it != submeshes.end(); ) {
			auto &submesh = *it;
			submesh.DetachMesh(mesh);

			// Remove the submesh if it has no attachments left
			if (!submesh.HasAttachedMeshes()) {
				it = submeshes.erase(it);
			} else {
				it++;
			}
		}

		if (hasClothBones) {
			for (int i = 0; i < cloth_stuff1_count; i++) {
				auto &clothStuff = cloth_stuff1[i];
				if (clothStuff.mesh == mesh) {
					delete clothStuff.clothStuff;
					
					delete[] clothStuff.bytePerClothVertex;
					delete[] clothStuff.bytePerClothVertex2;
					delete[] clothStuff.vertexIdxForClothVertexIdx;

					if (cloth_stuff1_count <= 1) {
						delete[] cloth_stuff1;
						cloth_stuff1 = nullptr;
						cloth_stuff1_count = 0;
					} else {
						auto newClothStuff = new AasClothStuff1[cloth_stuff1_count - 1];

						auto it = std::copy(cloth_stuff1, cloth_stuff1 + i, newClothStuff);
						std::copy(cloth_stuff1 + i + 1, cloth_stuff1 + cloth_stuff1_count, it);

						delete[] cloth_stuff1;
						cloth_stuff1 = newClothStuff;

						cloth_stuff1_count--;
					}
					break;
				}
			}
		}

		//static auto org_method = temple::GetPointer<int __fastcall(IAnimatedModel*, void*, Mesh*)>(0x10267480);
		//return org_method(this, 0, skm_file);
	}

	void AnimatedModel::ResetSubmeshes()
	{

		if (submeshesValid)
		{
			for (auto &submesh : submeshes) {
				submesh.ResetState();
			}
			submeshesValid = false;
		}
	}

	struct PosPair {
		int first_position_idx;
		int first_normal_idx;
	};

	void AnimatedModel::Method11() {

		static std::array<AasVertexState, 0x7FFF> vertex_state;
		static std::array<AasWorkSet, 0x47FF7> workset;

		// Cached bone mappings for SKM file
		Mesh *bone_mapping_mesh = nullptr;
		BoneMapping bone_mapping[1024];
		int used_ska_bone_count = 0; // Built by bone_mapping, may include gaps
		PosPair pos_pairs[1024];
		struct {
			int pos_count;
			int normals_count;
			int pos_float_start;
			int pos_vec_start;
			int normals_float_start;
			int normals_vec_start;
		} ska_bone_affected_count[1024];

		// Mapping between index of vertex in SKM file, and index of vertex in submesh vertex array
		int16_t vertex_idx_mapping[0x7FFF];
		int16_t prim_vert_idx[0xFFFF * 3]; // 3 vertex indices per face

		static SubmeshVertexClothStateWithFlag cloth_vertices_with_flag[0x7FFF];
		static SubmeshVertexClothStateWithoutFlag cloth_vertices_without_flag[0x7FFF];
		int cloth_vertices_with_flag_count = 0;
		int cloth_vertices_without_flag_count = 0;

		for (auto& submesh : submeshes) {
			int workset_count = 0;

			int primitive_count = 0; // Primarily used for indexing into prim_vert_idx
			int vertex_count = 0;
			int total_vertex_attachment_count = 0;

			for (auto &attachment : submesh.GetAttachedMeshes()) {
				
				auto mesh = attachment.mesh;

				// Rebuild the bone maping if we're working on a new SKM file
				// TODO: This is expensive and stupid since pairs of SKM/SKA files are reused all the time
				// and the mapping between their bones is static
				if (mesh != bone_mapping_mesh) {
					used_ska_bone_count = build_bone_mapping(bone_mapping, *skeleton, *mesh) + 1;
					bone_mapping_mesh = mesh;
				}

				// Find the cloth state (?) for the mesh
				auto matchingclothstuff1idx = cloth_stuff1_count;
				if (hasClothBones) {
					for (int i = 0; i < cloth_stuff1_count; i++) {
						if (cloth_stuff1[i].mesh == mesh) {
							matchingclothstuff1idx = i;
							break;
						}
					}
				}

				// Clear bone state
				PosPair pos_pair{ -1, -1 };
				std::fill(pos_pairs, pos_pairs + used_ska_bone_count, pos_pair);
				memset(&ska_bone_affected_count, 0, sizeof(ska_bone_affected_count));

				// Clear vertex idx mapping state
				std::fill(vertex_idx_mapping, vertex_idx_mapping + mesh->GetVertices().size(), -1);

				for (auto &face : mesh->GetFaces()) {
					// Skip faces with different materials than this submesh
					if (face.material_idx != attachment.materialIdx) {
						continue;
					}

					auto prim_vert_idx_out = &prim_vert_idx[primitive_count++ * 3];

					for (auto skm_vertex_idx : face.vertices) {

						auto &vertex = mesh->GetVertex(skm_vertex_idx);

						// Is it mapped for the submesh already, then reuse the existing vertex
						auto mapped_idx = vertex_idx_mapping[skm_vertex_idx];
						if (mapped_idx != -1) {
							*(prim_vert_idx_out++) = mapped_idx;
							continue;
						}

						// If it's not mapped already, we'll need to create the vertex
						mapped_idx = vertex_count++;
						vertex_idx_mapping[skm_vertex_idx] = mapped_idx;
						*(prim_vert_idx_out++) = mapped_idx;
						auto &cur_vertex_state = vertex_state[mapped_idx];

						// Handle the cloth state
						if (matchingclothstuff1idx < cloth_stuff1_count) {
							auto &cloth_state = cloth_stuff1[matchingclothstuff1idx];

							// Find the current vertex in the cloth vertex list (slow...)
							for (int i = 0; i < cloth_state.clothVertexCount; i++) {
								if (cloth_state.vertexIdxForClothVertexIdx[i] == skm_vertex_idx) {
									if (cloth_state.bytePerClothVertex[i]) {
										cloth_vertices_with_flag[cloth_vertices_with_flag_count].cloth_stuff1 = &cloth_state;
										cloth_vertices_with_flag[cloth_vertices_with_flag_count].submesh_vertex_idx = mapped_idx;
										cloth_vertices_with_flag[cloth_vertices_with_flag_count].cloth_stuff_vertex_idx = i;
										cloth_vertices_with_flag_count++;
									}
									else {
										cloth_vertices_without_flag[cloth_vertices_without_flag_count].cloth_stuff1 = &cloth_state;
										cloth_vertices_without_flag[cloth_vertices_without_flag_count].submesh_vertex_idx = mapped_idx;
										cloth_vertices_without_flag[cloth_vertices_without_flag_count].cloth_stuff_vertex_idx = i;
										cloth_vertices_without_flag_count++;
									}
								}
							}
						}

						// Deduplicate bone attachment ids and weights
						int vertex_bone_attach_count = 0;
						float vertex_bone_attach_weights[6];
						int16_t vertex_bone_attach_ska_ids[6]; // Attached bones mapped to SKA bone ids
						DX::XMFLOAT4 attachment_positions[6];
						DX::XMFLOAT4 attachment_normals[6];

						for (int attachment_idx = 0; attachment_idx < vertex.attachmentCount; attachment_idx++) {
							auto attachment_bone_mapping = bone_mapping[vertex.attachmentBone[attachment_idx]];
							auto attachment_skm_bone_idx = attachment_bone_mapping.skm_bone_idx; // even the SKM bone can be remapped...
							auto attachment_ska_bone_idx = attachment_bone_mapping.ska_bone_idx;
							auto attachment_weight = vertex.attachmentWeight[attachment_idx];

							// Check if for this particular vertex, the same SKA bone has already been used for an attachment
							bool found = false;
							for (int i = 0; i < vertex_bone_attach_count; i++) {
								if (vertex_bone_attach_ska_ids[i] == attachment_ska_bone_idx) {
									vertex_bone_attach_weights[i] += attachment_weight;
									found = true;
									break;
								}
							}

							if (found) {
								// Continue with the next attachment if we were able to merge the current one with an existing attachment
								continue;
							}

							auto attachment_idx_out = vertex_bone_attach_count++;
							auto &attachment_position = attachment_positions[attachment_idx_out];
							auto &attachment_normal = attachment_normals[attachment_idx_out];

							vertex_bone_attach_ska_ids[attachment_idx_out] = attachment_ska_bone_idx;
							vertex_bone_attach_weights[attachment_idx_out] = attachment_weight;

							// Start out with the actual vertex position in model world space
							attachment_position = vertex.pos;
							attachment_normal = vertex.normal;

							// It's possible that the vertex has an assignment, but the assignment's bone is not present in the SKA file,
							// so the SKM bone also get's mapped to -1 in case no parent bone can be used
							if (attachment_skm_bone_idx >= 0) {

								auto &skm_bone = mesh->GetBone(attachment_skm_bone_idx);

								// Transform the vertex position into the bone's local vector space
								attachment_position = transformPosition(skm_bone.full_world_inverse, attachment_position);
								attachment_normal = transformNormal(skm_bone.full_world_inverse, attachment_normal);
							}

							// NOTE: ToEE computed bounding boxes for each bone here, but it didn't seem to be used

						}

						// If the vertex has no bone attachments, simply use the vertex position with a weight of 1.0
						if (vertex_bone_attach_count == 0) {
							vertex_bone_attach_weights[0] = 1.0f;
							vertex_bone_attach_ska_ids[0] = -1;
							attachment_positions[0] = vertex.pos;
							attachment_normals[0] = vertex.normal;
							vertex_bone_attach_count = 1;
						}

						// Now convert from the temporary state to the linearized list of vertex positions
						cur_vertex_state.count = vertex_bone_attach_count;
						cur_vertex_state.uv = vertex.uv;
						total_vertex_attachment_count += vertex_bone_attach_count;
						for (int i = 0; i < vertex_bone_attach_count; i++) {
							auto weight = vertex_bone_attach_weights[i];
							auto ska_bone_idx = vertex_bone_attach_ska_ids[i];
							auto bone_matrix_idx = ska_bone_idx + 1;
							cur_vertex_state.array1[i] = bone_matrix_idx;

							DX::XMFLOAT4 weighted_pos;
							weighted_pos.x = weight * attachment_positions[i].x;
							weighted_pos.y = weight * attachment_positions[i].y;
							weighted_pos.z = weight * attachment_positions[i].z;
							cur_vertex_state.array2[i] = workset_add(&workset[0],
								&workset_count,
								weighted_pos,
								weight,
								&pos_pairs[bone_matrix_idx].first_position_idx,
								&ska_bone_affected_count[bone_matrix_idx].pos_count
							);

							DX::XMFLOAT4 weighted_normal;
							weighted_normal.x = weight * attachment_normals[i].x;
							weighted_normal.y = weight * attachment_normals[i].y;
							weighted_normal.z = weight * attachment_normals[i].z;
							cur_vertex_state.array3[i] = workset_add(&workset[0],
								&workset_count,
								weighted_normal,
								0.0,
								&pos_pairs[bone_matrix_idx].first_normal_idx,
								&ska_bone_affected_count[bone_matrix_idx].normals_count
							);
						}
					}
				}

			}

			if (hasClothBones) {
				submesh.cloth_vertices_without_flag.assign(
					cloth_vertices_without_flag,
					cloth_vertices_without_flag + cloth_vertices_without_flag_count
				);
				cloth_vertices_without_flag_count = 0;

				submesh.cloth_vertices_with_flag.assign(
					cloth_vertices_with_flag,
					cloth_vertices_with_flag + cloth_vertices_with_flag_count
				);
				cloth_vertices_with_flag_count = 0;
			}

			submesh.vertexCount = vertex_count;
			submesh.primCount = primitive_count;
			submesh.bone_elem_counts = std::make_unique<BoneElementCount[]>(used_ska_bone_count);

			int cur_float_offset = 0;
			int cur_vec_offset = 0;
			for (int i = 0; i <used_ska_bone_count; i++) {
				auto &affected_count = ska_bone_affected_count[i];
				auto pos_count = affected_count.pos_count;
				auto normals_count = affected_count.normals_count;
				submesh.bone_elem_counts[i].position_count = pos_count;
				submesh.bone_elem_counts[i].normal_count = normals_count;

				affected_count.pos_float_start = cur_float_offset;
				affected_count.pos_vec_start = cur_vec_offset;
				cur_float_offset += 4 * pos_count;
				cur_vec_offset += pos_count;

				affected_count.normals_float_start = cur_float_offset;
				affected_count.normals_vec_start = cur_vec_offset;
				cur_float_offset += 3 * normals_count;
				cur_vec_offset += normals_count;
			}

			submesh.bone_floats_count = cur_float_offset;
			submesh.bone_floats = std::make_unique<float[]>(cur_float_offset);

			for (int i = 0; i < used_ska_bone_count; i++) {
				auto &affected_count = ska_bone_affected_count[i];

				// Copy over all positions to the float buffer
				auto scratch_idx = pos_pairs[i].first_position_idx;
				auto *float_out = &submesh.bone_floats[affected_count.pos_float_start];
				while (scratch_idx != -1) {
					*(float_out++) = workset[scratch_idx].vector.x;
					*(float_out++) = workset[scratch_idx].vector.y;
					*(float_out++) = workset[scratch_idx].vector.z;
					*(float_out++) = workset[scratch_idx].weight;

					scratch_idx = workset[scratch_idx].next;
				}

				// Copy over all normals to the float buffer
				scratch_idx = pos_pairs[i].first_normal_idx;
				float_out = &submesh.bone_floats[affected_count.normals_float_start];

				while (scratch_idx != -1) {
					*(float_out++) = workset[scratch_idx].vector.x;
					*(float_out++) = workset[scratch_idx].vector.y;
					*(float_out++) = workset[scratch_idx].vector.z;

					scratch_idx = workset[scratch_idx].next;
				}

			}

			submesh.positions = std::make_unique<DX::XMFLOAT4[]>(vertex_count);
			submesh.normals = std::make_unique<DX::XMFLOAT4[]>(vertex_count);
			submesh.vertex_copy_positions = std::make_unique<short[]>(2 * total_vertex_attachment_count + 1);
			submesh.uv = std::make_unique<DX::XMFLOAT2[]>(vertex_count);

			auto cur_vertex_copy_pos_out = submesh.vertex_copy_positions.get();
			for (int i = 0; i < vertex_count; i++) {
				auto &cur_vertex_state = vertex_state[i];
				submesh.uv[i].x = cur_vertex_state.uv.x;
				submesh.uv[i].y = 1.0f - cur_vertex_state.uv.y;

				for (int j = 0; j < cur_vertex_state.count; j++) {
					// TODO: Is this the "target" position in the positions array?
					auto x = ska_bone_affected_count[cur_vertex_state.array1[j]].pos_vec_start + cur_vertex_state.array2[j];
					if (j == 0) {
						x = -1 - x;
					}
					*(cur_vertex_copy_pos_out++) = x;
				}

				for (int j = 0; j < cur_vertex_state.count; j++) {
					// TODO: Is this the "target" position in the positions array?
					auto x = ska_bone_affected_count[cur_vertex_state.array1[j]].normals_vec_start + cur_vertex_state.array3[j];
					if (j == 0) {
						x = -1 - x;
					}
					*(cur_vertex_copy_pos_out++) = x;
				}
			}
			*cur_vertex_copy_pos_out = std::numeric_limits<short>::min();

			// This will actually flip the indices. Weird.
			submesh.indices = std::make_unique<uint16_t[]>(3 * primitive_count);
			size_t indices_idx = 0;
			for (int i = 0; i < primitive_count; i++) {
				submesh.indices[indices_idx] = prim_vert_idx[indices_idx + 2];
				submesh.indices[indices_idx + 1] = prim_vert_idx[indices_idx + 1];
				submesh.indices[indices_idx + 2] = prim_vert_idx[indices_idx];
				indices_idx += 3;
			}
			submesh.fullyInitialized = true;
		}

		submeshesValid = true;

	}

	void AnimatedModel::SetClothFlagSth()
	{

		if (hasClothBones)
		{
			if (!submeshesValid) {
				Method11();
			}

			for (auto &submesh : submeshes) {
				for (auto &vertex_cloth_state : submesh.cloth_vertices_without_flag) {
					auto vertex_idx = vertex_cloth_state.cloth_stuff_vertex_idx;
					vertex_cloth_state.cloth_stuff1->bytePerClothVertex2[vertex_idx] = 1;
				}
			}
		}

	}

	std::vector<AasMaterial> AnimatedModel::GetSubmeshes() const
	{
		std::vector<AasMaterial> result;
		result.reserve(submeshes.size());

		for (auto &submesh : submeshes) {
			result.push_back(submesh.materialId);
		}

		return result;
	}

	int AnimatedModel::Method14()
	{
		return -1;
	}

	bool AnimatedModel::HasAnimation(std::string_view animName) const
	{
		if (!skeleton) {
			return false;
		}

		return skeleton->FindAnimIdxByName(animName) != -1;
	}

	void AnimatedModel::PlayAnim(int animIdx)
	{
		auto player = new AnimPlayer;

		player->Attach(this, animIdx, eventHandler);
		player->Setup2(0.5f);
	}

	void AnimatedModel::CleanupAnimations(AnimPlayer *player)
	{
		// Delete any animation that has ended once it has faded out
		if (!player->GetEventHandlingDepth())
		{
			if (player->weight <= 0.0 && player->fadingSpeed < 0.0)
			{
				delete player;
				return;
			}
		}

		// When any animation that is not currently fading out reaches weight 1,
		// delete any animation that may have ended regardless of whether it has
		// finished fading out
		if (player->weight >= 1.0f && player->fadingSpeed >= 0.0)
		{
			auto curAnim = player->prevRunningAnim;
			while (curAnim)
			{
				auto prev = curAnim->prevRunningAnim;
				if (!curAnim->GetEventHandlingDepth())
				{
					delete curAnim;
				}
				curAnim = prev;
			}
		}
	}

	void AnimatedModel::Advance(const Matrix3x4 & world_matrix, float delta_time, float delta_distance, float delta_rotation)
	{
		this->drivenTime += delta_time;
		this->timeForClothSim += delta_time;
		this->drivenDistance += delta_distance;
		this->drivenRotation += delta_rotation;
		this->worldMatrix = world_matrix;

		auto anim_player = runningAnimsHead;
		while (anim_player) {
			auto next = anim_player->nextRunningAnim;
			anim_player->FadeInOrOut(delta_time);
			anim_player = next;
		}

		auto v8 = newestRunningAnim;
		if (v8)
		{
			v8->EnterEventHandling();
			v8->AdvanceEvents(delta_time, delta_distance, delta_rotation);
			v8->LeaveEventHandling();

			CleanupAnimations(v8);
		}
	}

	void AnimatedModel::SetWorldMatrix(const Matrix3x4 & worldMatrix)
	{
		this->worldMatrix = worldMatrix;
	}

	// Originally @ 0x102682a0
	void AnimatedModel::Method19()
	{

		if (drivenTime <= 0.0f && drivenDistance <= 0.0f && drivenRotation <= 0.0f && currentWorldMatrix == worldMatrix) {
			return;
		}

		// Build the effective translation/scale/rotation for each bone of the animated skeleton
		SkelBoneState boneState[1024];
		auto bones = skeleton->GetBones();

		auto running_anim = runningAnimsHead;
		if (!running_anim || running_anim->weight != 1.0f || running_anim->fadingSpeed < 0.0) {
			for (int i = 0; i < bones.size(); i++) {
				boneState[i] = bones[i].initialState;
			}
		}

		while (running_anim) {
			auto next_running_anim = running_anim->nextRunningAnim;

			running_anim->method6(boneState, drivenTime, drivenDistance, drivenRotation);

			CleanupAnimations(running_anim);

			running_anim = next_running_anim;
		}

		auto scaleMat = scaleMatrix(scale, scale, scale);
		boneMatrices[0] = multiplyMatrix3x3_3x4(scaleMat, worldMatrix);
		
		for (int i = 0; i < bones.size(); i++) {
			auto rotation = rotationMatrix(Quaternion(boneState[i].rotation));
			auto scale = scaleMatrix(boneState[i].scale.x, boneState[i].scale.y, boneState[i].scale.z);

			auto &boneMatrix = boneMatrices[1 + i];
			boneMatrix = multiplyMatrix3x3(scale, rotation);
			boneMatrix.m03 = 0;
			boneMatrix.m13 = 0;
			boneMatrix.m23 = 0;

			auto parentId = bones[i].parentId;
			if (parentId >= 0) {
				auto parent_scale = boneState[parentId].scale;
				auto parent_scale_mat = scaleMatrix(1.0f / parent_scale.x, 1.0f / parent_scale.y, 1.0f / parent_scale.z);
				boneMatrix = multiplyMatrix3x4_3x3(boneMatrix, parent_scale_mat);
			}

			auto translation = translationMatrix(boneState[i].translation.x, boneState[i].translation.y, boneState[i].translation.z);
			boneMatrix = multiplyMatrix3x4(boneMatrix, multiplyMatrix3x4(translation, boneMatrices[1 + parentId]));
		}

		drivenTime = 0;
		drivenDistance = 0;
		drivenRotation = 0;
		currentWorldMatrix = worldMatrix;

		for (auto &submesh : submeshes) {
			submesh.fullyInitialized = true;
		}

	}

	void AnimatedModel::SetTime(float time, const Matrix3x4 & worldMatrix)
	{
		auto player = runningAnimsHead;
		while (player) {
			auto next = player->nextRunningAnim;
			player->weight = 0;

			if (!next) {
				player->weight = 1.0f;
				player->SetTime(time);
				// epsilon
				Advance(
					worldMatrix,
					0.00001f,
					0,
					0);
			}
			player = next;
		}
	}

	float AnimatedModel::GetCurrentFrame()
	{
		auto player = runningAnimsHead;
		if (player)
		{
			while (player->nextRunningAnim)
				player = player->nextRunningAnim;
			return player->GetCurrentFrame();
		}
		else
		{
			return 0.0f;
		}
	}

	void AnimatedModel::DeleteSubmesh(int submeshIdx)
	{
		submeshes.erase(submeshes.begin() + submeshIdx);
	}

	// Originally @ 10265CC0
	bool AnimatedModel::SetAnimByName(std::string_view name)
	{
		if (!skeleton) {
			return false;
		}

		auto animIdx = skeleton->FindAnimIdxByName(name);
		if (animIdx != -1) {
			PlayAnim(animIdx);
			return true;
		}

		return false;

	}

	void AnimatedModel::SetSpecialMaterial(gfx::MaterialPlaceholderSlot slot, AasMaterial material)
	{

		for (auto &submesh : submeshes) {
			auto resolver = submesh.materialResolver;
			if (resolver->IsMaterialPlaceholder(submesh.materialId)) {
				auto submeshSlot = resolver->GetMaterialPlaceholderSlot(submesh.materialId);
				if (submeshSlot == slot) {
					submesh.materialId = material;
				}
			}

		}

	}

	// Originally @ 10266450
	AasSubmeshWithMaterial * AnimatedModel::GetOrAddSubmesh(AasMaterial material, IMaterialResolver *materialResolver)
	{

		// Check if there's an existing submesh for the material+resolver it came from
		for (auto &submesh : submeshes) {
			if (submesh.materialId == material && submesh.materialResolver == materialResolver) {
				return &submesh;
			}
		}

		return &submeshes.emplace_back(material, materialResolver);

		/*static auto aas_class2_GetOrCreateSubmesh = temple::GetPointer<int __fastcall(AnimatedModel*, void*, int *matId, void *matResolver)>(0x10266450);
		int mat_id = (int)material;
		int submesh_idx = aas_class2_GetOrCreateSubmesh(this, 0, &mat_id, mat_resolver);
		if (submesh_idx >= 0) {
			return &submeshes[submesh_idx];
		}
		return nullptr;*/
	}

	class SubmeshAdapter : public gfx::Submesh {
	public:

		SubmeshAdapter(
			int vertexCount,
			int primitiveCount,
			gsl::span<DX::XMFLOAT4> positions,
			gsl::span<DX::XMFLOAT4> normals,
			gsl::span<DX::XMFLOAT2> uv,
			gsl::span<uint16_t> indices
		) : vertexCount_(vertexCount), primitiveCount_(primitiveCount), positions_(positions), normals_(normals), uv_(uv), indices_(indices) {}
		SubmeshAdapter() {}

		virtual int GetVertexCount() override
		{
			return vertexCount_;
		}
		virtual int GetPrimitiveCount() override
		{
			return primitiveCount_;
		}
		virtual gsl::span<DX::XMFLOAT4> GetPositions() override
		{
			return positions_;
		}
		virtual gsl::span<DX::XMFLOAT4> GetNormals() override
		{
			return normals_;
		}
		virtual gsl::span<DX::XMFLOAT2> GetUV() override
		{
			return uv_;
		}
		virtual gsl::span<uint16_t> GetIndices() override
		{
			return indices_;
		}

	private:
		int vertexCount_ = 0;
		int primitiveCount_ = 0;
		gsl::span<DX::XMFLOAT4> positions_;
		gsl::span<DX::XMFLOAT4> normals_;
		gsl::span<DX::XMFLOAT2> uv_;
		gsl::span<uint16_t> indices_;
	};

	std::unique_ptr<gfx::Submesh> AnimatedModel::GetSubmesh(int submeshIdx)
	{

		if (submeshIdx < 0 || submeshIdx >= (int) submeshes.size()) {
			return std::make_unique<SubmeshAdapter>();
		}

		if (!submeshesValid) {
			Method11();
		}

		auto &submesh = submeshes[submeshIdx];

		Method19();

		if (submesh.fullyInitialized) {
			static DX::XMFLOAT4A vectors[0x5FFF6];
			auto vec_out = &vectors[0];

			// The cur_pos "w" component is (in reality) the attachment weight
			auto cur_float_in = submesh.bone_floats.get();
			auto bones = skeleton->GetBones();

			for (int16_t bone_idx = 0; bone_idx <= bones.size(); bone_idx++) {
				Matrix3x4 bone_matrix = boneMatrices[bone_idx];

				auto elem_counts = submesh.bone_elem_counts[bone_idx];

				for (int i = 0; i < elem_counts.position_count; i++) {
					auto x = *(cur_float_in++);
					auto y = *(cur_float_in++);
					auto z = *(cur_float_in++);
					auto weight = *(cur_float_in++);
					vec_out->x = x * bone_matrix.m00 + y * bone_matrix.m01 + z * bone_matrix.m02 + weight * bone_matrix.m03;
					vec_out->y = x * bone_matrix.m10 + y * bone_matrix.m11 + z * bone_matrix.m12 + weight * bone_matrix.m13;
					vec_out->z = x * bone_matrix.m20 + y * bone_matrix.m21 + z * bone_matrix.m22 + weight * bone_matrix.m23;
					vec_out++;
				}

				for (int i = 0; i < elem_counts.normal_count; i++) {
					auto x = *(cur_float_in++);
					auto y = *(cur_float_in++);
					auto z = *(cur_float_in++);
					vec_out->x = x * bone_matrix.m00 + y * bone_matrix.m01 + z * bone_matrix.m02;
					vec_out->y = x * bone_matrix.m10 + y * bone_matrix.m11 + z * bone_matrix.m12;
					vec_out->z = x * bone_matrix.m20 + y * bone_matrix.m21 + z * bone_matrix.m22;
					vec_out++;
				}
			}

			if (submesh.vertexCount > 0) {
				auto position_out = submesh.positions.get();
				auto normal_out = submesh.normals.get();
				auto j = submesh.vertex_copy_positions.get();
				auto v49 = &vectors[-(*j + 1)];			
				j++;
				while (1)
				{
					*position_out = *v49;
					auto v51 = *j;
					for (j = j + 1; v51 >= 0; v51 = *(j - 1))
					{
						auto v53 = &vectors[v51];
						++j;
						position_out->x += v53->x;
						position_out->y += v53->y;
						position_out->z += v53->z;
					}
					auto v54 = &vectors[-(v51 + 1)];
				
					*normal_out = *v54;
					auto v55 = *j;
					for (j = j + 1; v55 >= 0; v55 = *(j - 1))
					{
						auto v56 = &vectors[v55];
						++j;
						normal_out->x += v56->x;
						normal_out->y += v56->y;
						normal_out->z += v56->z;
					}
					if (v55 == -32768)
						break;
					++position_out;
					++normal_out;
					v49 = &vectors[-(v55 + 1)];
				}
			}

			// Renormalize the normals if necessary
			if (scale != 1.0f) {
				auto normals = submesh.normals.get();
				for (int i = 0; i < submesh.vertexCount; i++) {
					auto v = DX::XMLoadFloat4(&normals[i]);
					v = DX::XMVector3Normalize(v);
					DX::XMStoreFloat4(&normals[i], v);
				}
			}

			if (hasClothBones) {
				auto inverseSomeMatrix = invertOrthogonalAffineTransform(currentWorldMatrix);

				for (auto sphere = collisionSpheresHead.get(); sphere; sphere = sphere->next.get()) {
					Method19();
					auto boneMatrix = boneMatrices[sphere->boneId + 1];
					makeMatrixOrthogonal(boneMatrix);

					auto boneMult = multiplyMatrix3x4(boneMatrix, inverseSomeMatrix);
					sphere->worldMatrix = boneMult;
					sphere->worldMatrixInverse = invertOrthogonalAffineTransform(boneMult);
				}
			
				for (auto cylinder = collisionCylindersHead.get(); cylinder; cylinder = cylinder->next.get()) {
					Method19();
					auto boneMatrix = boneMatrices[cylinder->boneId + 1];
					makeMatrixOrthogonal(boneMatrix);

					auto boneMult = multiplyMatrix3x4(boneMatrix, inverseSomeMatrix);
					cylinder->worldMatrix = boneMult;
					cylinder->worldMatrixInverse = invertOrthogonalAffineTransform(boneMult);
				}

				for (auto &state : submesh.cloth_vertices_with_flag) {
					auto pos = submesh.positions[state.submesh_vertex_idx];

					auto &pos_out = state.cloth_stuff1->clothStuff->clothVertexPos2[state.cloth_stuff_vertex_idx];
					pos_out = transformPosition(inverseSomeMatrix, pos);
				}

				for (int i = 0; i < cloth_stuff1_count; i++) {
					auto stuff1 = &cloth_stuff1[i];
					if (stuff1->field_18) {
						stuff1->clothStuff->UpdateBoneDistances();
						stuff1->field_18 = 0;
					}
				}

				if (timeForClothSim > 0.0) {
					for (int i = 0; i < cloth_stuff1_count; i++) {
						cloth_stuff1[i].clothStuff->Simulate(timeForClothSim);
						//static auto aas_cloth_stuff_sim_maybe = temple::GetPointer<void __fastcall(AasClothStuff*, void*, float)>(0x10269d50);
						//aas_cloth_stuff_sim_maybe(cloth_stuff1[i].clothStuff, 0, timeForClothSim);
					}
					timeForClothSim = 0.0f;
				}

				//auto someMatrixVec = DX::XMMatrixTranspose(DX::XMLoadFloat4x3((DX::XMFLOAT4X3*)&someMatrix));
				//auto someMatrixInverse = DX::XMMatrixInverse(nullptr, someMatrixVec);

				for (auto &cloth_vertex : submesh.cloth_vertices_without_flag) {
					auto cloth_stuff_vertex_idx = cloth_vertex.cloth_stuff_vertex_idx;
					auto submesh_vertex_idx = cloth_vertex.submesh_vertex_idx;
					auto cloth_stuff1 = cloth_vertex.cloth_stuff1;
				
					auto &mesh_pos = submesh.positions[submesh_vertex_idx];
					auto &cloth_pos = cloth_stuff1->clothStuff->clothVertexPos2[cloth_stuff_vertex_idx];

					if (cloth_stuff1->bytePerClothVertex2[cloth_stuff_vertex_idx])
					{
						Matrix4x4 m = currentWorldMatrix;
						m = m.inverse();

						cloth_pos.x = m.m00 * mesh_pos.x + m.m01 * mesh_pos.y + m.m02 * mesh_pos.z + m.m03;
						cloth_pos.y = m.m10 * mesh_pos.x + m.m11 * mesh_pos.y + m.m12 * mesh_pos.z + m.m13;
						cloth_pos.z = m.m20 * mesh_pos.x + m.m21 * mesh_pos.y + m.m22 * mesh_pos.z + m.m23;

						// DX::XMStoreFloat4(cloth_pos, DX::XMVector3TransformCoord(DX::XMLoadFloat4(mesh_pos), someMatrixInverse));

						cloth_stuff1->bytePerClothVertex2[cloth_stuff_vertex_idx] = 0;
						cloth_stuff1->field_18 = 1;
					}
					else
					{
						mesh_pos = transformPosition(currentWorldMatrix, cloth_pos);
						// DX::XMStoreFloat4(mesh_pos, DX::XMVector3TransformCoord(DX::XMLoadFloat4(cloth_pos), someMatrixVec));
					}
				}

			}
		
			submesh.fullyInitialized = false;
		}

		// return;
		// static auto method_org = temple::GetPointer<void __fastcall(AnimatedModel*, void*, int submesh_idx, int *vertex_count_out, XMFLOAT4 **positions_out, XMFLOAT4 **normals_out, XMFLOAT2 **uv_out, int *primitive_count_out, uint16_t **indices_out)>(0x102689c0);
		// return method_org(this, 0, submesh_idx, vertex_count_out, positions_out, normals_out, uv_out, primitive_count_out, indices_out);

		return std::make_unique<SubmeshAdapter>(
			submesh.vertexCount,
			submesh.primCount,
			gsl::span(submesh.positions.get(), submesh.vertexCount),
			gsl::span(submesh.normals.get(), submesh.vertexCount),
			gsl::span(submesh.uv.get(), submesh.vertexCount),
			gsl::span(submesh.indices.get(), submesh.primCount * 3)
		);

	}

	void AnimatedModel::AddRunningAnim(AnimPlayer *player)
	{
		Expects(skeleton);
		Expects(!player->ownerAnim);

		player->ownerAnim = this;

		if (!runningAnimsHead)
		{
			player->nextRunningAnim = runningAnimsHead;
			player->prevRunningAnim = nullptr;
			if (runningAnimsHead)
				runningAnimsHead->prevRunningAnim = player;
			runningAnimsHead = player;
			newestRunningAnim = player;
			return;
		}

		auto cur_anim = runningAnimsHead;
		for (auto i = cur_anim->nextRunningAnim; i; i = i->nextRunningAnim)
		{
			cur_anim = i;
		}
		auto v9 = cur_anim->nextRunningAnim;
		player->nextRunningAnim = v9;
		player->prevRunningAnim = cur_anim;
		if (v9)
			v9->prevRunningAnim = player;

		cur_anim->nextRunningAnim = player;
		
		auto v11 = runningAnimsHead->nextRunningAnim;
		auto runningAnimCount = 1;
		if (!v11) {
			newestRunningAnim = player;
			return;
		}
		do
		{
			v11 = v11->nextRunningAnim;
			++runningAnimCount;
		} while (v11);
		if (runningAnimCount <= 10)
		{
			newestRunningAnim = player;
			return;
		}
		
		cur_anim = runningAnimsHead;
		while (runningAnimCount > 1) {
			auto next = runningAnimsHead->nextRunningAnim;
			RemoveRunningAnim(cur_anim);
			delete cur_anim;
			cur_anim = next;
			runningAnimCount--;
		}

		newestRunningAnim = player;
	}

	void AnimatedModel::RemoveRunningAnim(AnimPlayer *anim)
	{
		if (anim->ownerAnim == this)
		{
			auto prev = anim->prevRunningAnim;
			if (prev) {
				prev->nextRunningAnim = anim->nextRunningAnim;
			} else {
				runningAnimsHead = anim->nextRunningAnim;
			}

			auto next = anim->nextRunningAnim;
			if (next) {
				next->prevRunningAnim = anim->prevRunningAnim;
			}

			anim->prevRunningAnim = 0;
			anim->nextRunningAnim = 0;
			anim->ownerAnim = 0;
		}
	}

	int AnimatedModel::GetAnimCount()
	{
		if (skeleton) {
			return skeleton->GetAnimations().size();
		} else {
			return 0;
		}
	}

	std::string_view AnimatedModel::GetAnimName(int animIdx) const
	{
		if (!skeleton) {
			return nullptr;
		}

		auto anims = skeleton->GetAnimations();
		if (animIdx < 0 || animIdx >= anims.size()) {
			return nullptr;
		}

		return anims[animIdx].name;
	}

	bool AnimatedModel::HasBone(std::string_view boneName) const
	{
		return skeleton->FindBoneIdxByName(boneName) != -1;
	}

	void AnimatedModel::ReplaceMaterial(AasMaterial oldMaterial, AasMaterial newMaterial)
	{
		for (auto &submesh : submeshes) {
			if (submesh.materialId == oldMaterial) {
				// TODO: This is actually dangerous if new_id is already another submesh in this model
				submesh.materialId = newMaterial;
				return;
			}
		}
	}

	float AnimatedModel::GetDistPerSec()
	{
		auto result = 0.0f;

		auto cur = runningAnimsHead;
		while (cur) {
			auto next = cur->nextRunningAnim;
			cur->GetDistPerSec(&result);
			cur = next;
		}

		return result;
	}

	float AnimatedModel::GetRotationPerSec()
	{
		auto result = 0.0f;

		auto cur = runningAnimsHead;
		while (cur) {
			auto next = cur->nextRunningAnim;
			cur->GetRotationPerSec(&result);
			cur = next;
		}

		return result;
	}

	AasSubmeshWithMaterial::~AasSubmeshWithMaterial()
	{
		std::string_view context = "";
		// Take the context of the first attachment
		if (!meshes.empty()) {
			context = meshes.front().mesh->GetFilename();
		}

		materialResolver->Release(materialId, context);
	}

	void AasSubmeshWithMaterial::ResetState() {
		cloth_vertices_without_flag.clear();
		cloth_vertices_with_flag.clear();
		bone_elem_counts.reset();
		bone_floats.reset();
		vertex_copy_positions.reset();
		positions.reset();
		normals.reset();
		normals.reset();
		uv.reset();
		indices.reset();
	}

	// Originally @ 0x102665E0
	int AasSubmeshWithMaterial::AttachMesh(Mesh *mesh, int materialIdx)
	{
		// Check if mesh is already attached
		for (size_t i = 0; i < meshes.size(); i++) {
			if (meshes[i].mesh == mesh && meshes[i].materialIdx == materialIdx) {
				return i;
			}
		}
		
		meshes.push_back({ mesh, materialIdx });
		return meshes.size() - 1;

	}

	void AasSubmeshWithMaterial::DetachMesh(Mesh *mesh)
	{
		auto it = meshes.begin();
		while (it != meshes.end()) {
			if (it->mesh == mesh) {
				it = meshes.erase(it);
			}
			else {
				it++;
			}
		}
	}

}
