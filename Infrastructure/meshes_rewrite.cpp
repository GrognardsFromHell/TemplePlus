#include <chrono>

#include "gsl/gsl.h"
#include "infrastructure/meshes.h"
#include "infrastructure/vfs.h"
#include "graphics/math.h"
#include "infrastructure/logging.h"
#include "infrastructure/exception.h"

using namespace DirectX;

class SkaBone {
public:
	explicit SkaBone(const uint8_t* data) : mData(data) {
	}

	uint16_t GetFlags() const {
		return *(uint16_t*)(mData);
	}

	int16_t GetParentId() const {
		return *(uint16_t*)(mData + 2);
	}

	const char* GetName() const {
		return reinterpret_cast<const char*>(mData + 4);
	}

private:
	const uint8_t* mData;
};

#pragma pack(push, 1)

struct SkaAnimStream {
	uint16_t frames;
	int16_t variationId;
	float frameRate;
	float dps;
	// Offset to the start of the key-frame stream in relation to animationStart
	uint32_t dataOffset;
};

struct SkaAnimHeader {
	char name[64];
	uint8_t driveType;
	uint8_t loopable;
	uint16_t eventCount;
	uint32_t eventOffset;
	uint32_t streamCount;
	SkaAnimStream streams[10];
};
#pragma pack(pop)

class SkaFile {
public:
	explicit SkaFile(const std::string& filename);

	size_t GetBoneCount() const {
		return Read<uint32_t>(0);
	}

	size_t GetAnimCount() const {
		return Read<uint32_t>(16);
	}

	SkaBone GetBone(size_t id) const {
		auto boneData = mData.data() + GetBoneDataStart() + id * 0x64;
		return SkaBone(boneData);
	}

	SkaAnimHeader& GetAnim(size_t id) const {
		auto animData = mData.data() + GetAnimDataStart();
		return *(SkaAnimHeader*)(animData + id * sizeof(SkaAnimHeader));
	}

private:

	template <typename T>
	T Read(uint32_t offset) const {
		return *reinterpret_cast<const T*>(mData.data() + offset);
	}

	size_t GetBoneDataStart() const {
		return Read<uint32_t>(4);
	}

	size_t GetAnimDataStart() const {
		return Read<uint32_t>(20);
	}

	std::vector<uint8_t> mData;

};

#pragma pack(push, 1)
struct SkmVertex {
	static constexpr size_t sMaxBoneAttachments = 6;
	XMFLOAT4 pos;
	XMFLOAT4 normal;
	XMFLOAT2 uv;
	uint16_t padding;
	uint16_t attachmentCount;
	uint16_t attachmentBone[sMaxBoneAttachments];
	float attachmentWeight[sMaxBoneAttachments];
};

struct SkmMaterial {
	char id[128];
};
#pragma pack(pop)

class SkmFile {
public:
	explicit SkmFile(const std::string& filename);

	uint32_t GetBoneCount() const {
		return Read<uint32_t>(0);
	}

	uint32_t GetMaterialCount() const {
		return Read<uint32_t>(8);
	}

	uint32_t GetVertexCount() const {
		return Read<uint32_t>(16);
	}

	uint32_t GetFaceCount() const {
		return Read<uint32_t>(24);
	}

	SkmVertex& GetVertex(uint32_t vertexId) const {
		return ((SkmVertex*)GetVertexData())[vertexId];
	}

	SkmMaterial& GetMaterial(uint32_t materialId) const {
		return ((SkmMaterial*)GetMaterialData())[materialId];
	}

private:
	std::vector<uint8_t> mData;

	const uint8_t* GetBoneData() const {
		return mData.data() + Read<uint32_t>(4);
	}

	const uint8_t* GetMaterialData() const {
		return mData.data() + Read<uint32_t>(12);
	}

	const uint8_t* GetVertexData() const {
		return mData.data() + Read<uint32_t>(20);
	}

	const uint8_t* GetFaceData() const {
		return mData.data() + Read<uint32_t>(28);
	}

	template <typename T>
	T Read(uint32_t offset) const {
		return *reinterpret_cast<const T*>(mData.data() + offset);
	}
};

struct AasClothSphere {
	int boneId;
	float param1;
	XMFLOAT4X3A someMatrix;
	XMFLOAT4X3A someMatrix2;
};

struct AasClothCylinder {
	int boneId;
	float param1;
	float param2;
	XMFLOAT4X3A someMatrix;
	XMFLOAT4X3A someMatrix2;
};

class AasClass2;

class AasEventListener {
public:
	virtual ~AasEventListener();
};

using MaterialRef = int;
using MaterialResolver = std::function<MaterialRef(const char*)>;

struct AasSubmesh {

	MaterialRef material;

	void* field_30 = nullptr; // cloth related
	void* field_34 = nullptr; // cloth related

	void* field_10 = nullptr;
	void* field_14 = nullptr;
	void* field_18 = nullptr;

	XMFLOAT3* positions = nullptr;
	XMFLOAT3* normals = nullptr;
	XMFLOAT2* uv = nullptr;
	uint16_t* indices = nullptr;
};

class AasClass2 {
public:
	explicit AasClass2(const SkaFile& skaFile, SkmFile& skmFile, AasEventListener& eventListener);

	void LoadMesh(MaterialResolver matResolver);

	void ResetSubmeshes();

	bool SetAnim(const std::string& name);

private:
	void InitializeBones();

	bool AddRunningAnim(size_t animId);

	AasSubmesh& GetOrCreateSubmeshForMaterial(MaterialRef material);

	AasEventListener& mEventListener;

	const SkaFile& mSkeleton;
	SkmFile& mMesh;

	float scale = 1.0f;
	float scaleInv = 1.0f;

	int field_C = 0; // TODO: Check LOBYTE access pattern

	std::vector<AasSubmesh> mSubmeshes;

	int submeshCount = 0; // TODO: Check hi/lo access pattern
	int submshes = 0;

	int newestRunningAnim = 0;
	int runningAnimsHead = 0;

	XMFLOAT4X3A mWorldMatrix;

	std::vector<XMFLOAT4X3A> mBoneMatrices;

	void* eventListener = nullptr;

	bool mHasClothBones = false;

	size_t mNormalBoneCount = 0;

	std::vector<AasClothSphere> mClothSpheres;
	std::vector<AasClothCylinder> mClothCylinders;

	int field_68 = 0;
	int timeRel1 = 0;
	int timeRel2 = 0;
	int drivenDistance = 0;
	int drivenRotation = 0;
};

class AasModelAndSkeleton {
public:

	AasModelAndSkeleton(const std::shared_ptr<SkaFile>& skaFile,
	                    const std::shared_ptr<SkmFile>& skmFile,
	                    AasEventListener& eventListener,
	                    gfx::EncodedAnimId animId);

	float floatConst = 6.394000053405762f;

	std::shared_ptr<SkaFile> skaFile;
	std::shared_ptr<SkmFile> skmFile;

	AasEventListener& mEventListener;

	AasClass2 aas2;

	void SetAnimId(gfx::EncodedAnimId animId);

	std::chrono::time_point<std::chrono::steady_clock> timeLoaded;

	std::string skaFilename;
	std::string skmFilename;

	gfx::EncodedAnimId mAnimId;

};

SkaFile::SkaFile(const std::string& filename) : mData(vfs->ReadAsBinary(filename)) {

}

SkmFile::SkmFile(const std::string& filename) : mData(vfs->ReadAsBinary(filename)) {
}

AasClass2::AasClass2(const SkaFile& skeleton, SkmFile& mesh, AasEventListener& eventListener)
	: mEventListener(eventListener), mSkeleton(skeleton), mMesh(mesh) {
	XMStoreFloat4x3A(&mWorldMatrix, XMMatrixIdentity());
}

void AasClass2::LoadMesh(MaterialResolver matResolver) {

	ResetSubmeshes();

	/*
		For some reason, any attachments to #ClothBone are removed, without recording first
		what the attachment was -> why???
	*/
	if (mHasClothBones) {

		for (size_t i = 0; i < mMesh.GetVertexCount(); i++) {
			auto& vertex = mMesh.GetVertex(i);

			if (vertex.attachmentCount <= 1) {
				continue;
			}

			// For some reason this only seems to count if the cloth bone is not the only
			// attachment???
			auto weightSum = 0.0f;

			for (size_t j = 0; j < vertex.attachmentCount; j++) {
				// This is the bone id of the #clothBone
				if (vertex.attachmentBone[j] == mNormalBoneCount) {
					vertex.attachmentWeight[j] = 0;
				} else {
					weightSum += vertex.attachmentWeight[j];
				}
			}

			// Renormalize the weights
			if (weightSum > 0) {
				for (size_t j = 0; j < vertex.attachmentCount; j++) {
					// This is the bone id of the #clothBone
					if (vertex.attachmentBone[j] != mNormalBoneCount) {
						vertex.attachmentWeight[j] = vertex.attachmentWeight[j] / weightSum;
					}
				}
			}
		}

	}

	for (size_t i = 0; i < mMesh.GetMaterialCount(); i++) {
		auto& material = mMesh.GetMaterial(i);

		auto resolvedMat = matResolver(material.id);
		auto& submesh = GetOrCreateSubmeshForMaterial(resolvedMat);
		// TODO: Second submesh function
	}

	// TODO: Lots of cloth-bone stuff

}

void AasClass2::ResetSubmeshes() {

	signed int v2; // ebp@2
	int v3; // edi@3

	if (field_C) {

		mSubmeshes.clear();

		/*for (auto i = 0; i < submeshCount; ++i) {
			if (mHasClothBones) {
				j__free_0(v1->submeshes[v3].field_30);
				j__free_0(v1->submeshes[v3].field_34);
			}
			j__free_0(v1->submeshes[v3].field_10);
			v1->submeshes[v3].field_10 = 0;
			j__free_0(v1->submeshes[v3].field_14);
			v1->submeshes[v3].field_14 = 0;
			j__free_0(v1->submeshes[v3].field_18);
			v1->submeshes[v3].field_18 = 0;
			j__free_0(v1->submeshes[v3].positions);
			v1->submeshes[v3].positions = 0;
			j__free_0(v1->submeshes[v3].normals);
			v1->submeshes[v3].normals = 0;
			j__free_0(v1->submeshes[v3].uv);
			v1->submeshes[v3].uv = 0;
			j__free_0(v1->submeshes[v3].indices);
			v1->submeshes[v3].indices = 0;
		}*/

		field_C = 0;

	}

}

bool AasClass2::SetAnim(const std::string& name) {

	// Find the animation
	for (size_t i = 0; i < mSkeleton.GetAnimCount(); i++) {
		auto& anim = mSkeleton.GetAnim(i);
		if (!_stricmp(anim.name, name.c_str())) {
			return AddRunningAnim(i);
		}
	}

	return false;
}

void AasClass2::InitializeBones() {

	mBoneMatrices.resize(mSkeleton.GetBoneCount() + 1);

	// Count the number of normal bones in the skeleton (ones that are unaffected by cloth sim)
	mNormalBoneCount = 0;
	for (size_t i = 0; i < mSkeleton.GetBoneCount(); ++i) {
		auto bone = mSkeleton.GetBone(i);
		if (!_stricmp("#ClothBone", bone.GetName())) {
			mHasClothBones = true;
			break;
		}
		mNormalBoneCount++;
	}

	field_68 = 0;

	if (mHasClothBones) {

		for (size_t i = 0; i < mSkeleton.GetBoneCount(); ++i) {

			auto bone = mSkeleton.GetBone(i);
			auto name = bone.GetName();

			if (!_strnicmp("#Sphere", name, 7)) {

				// Find the opening brace to parse the parameters
				auto params = name;
				while (*params) {
					if (*params++ == '{')
						break;
				}

				if (*params) {
					AasClothSphere sphere;
					sphere.boneId = i;
					sphere.param1 = (float) atof(params); // Most likely radius					
					mClothSpheres.emplace_back(sphere);
				} else {
					logger->warn("Found cloth bone with invalid name '{}'", name);
				}
			} else if (!_strnicmp("#Cylinder", name, 9)) {
				AasClothCylinder cylinder;
				cylinder.boneId = i;

				auto params = name;
				while (*params) {
					if (*params++ == '{') {
						break;
					}
				}
				cylinder.param1 = (float)atof(params);

				while (*params) {
					if (*params++ == ',') {
						break;
					}
				}
				cylinder.param2 = (float)atof(params);

				mClothCylinders.emplace_back(cylinder);
			}


		}
	}

}

class AasRunningStream {
public:
	void* operator new(size_t s) {
		return _aligned_malloc(s, 16);
	}

	void operator delete(void* ptr) {
		_aligned_free(ptr);
	}
};

class AasRunningAnim {
public:
	AasRunningAnim(AasClass2& owner, SkaAnimHeader& animHeader);

private:

	float mDuration = 0;
	std::unique_ptr<AasRunningStream> mStream;

	// These were part of the baseclass
	AasClass2& owner;
	int boneIdx = -2;
	AasRunningAnim* nextRunningAnim = nullptr;
	AasRunningAnim* prevRunningAnim = nullptr;
	uint8_t field_C = 1;
	uint8_t field_D = 0;
	int elapsedTimeRelated = 0;
	int field_14 = 0x40000000;
	int eventHandlingDepth = 0;

	// Part of the subclass
	int streamCount = 0;
	int currentTime = 0;
	int field_58 = 0;

	SkaAnimHeader& mAnimHeader;

};

bool AasClass2::AddRunningAnim(size_t animId) {

	Expects(animId < mSkeleton.GetAnimCount());

	auto& anim = mSkeleton.GetAnim(animId);

	/// PREVIOUSLY: 
	/// ToEE had support for propability based variations i think, but all animations 
	/// Only ever used 1, so we're stripping this feature out.

	// TODO: Finish
	return true;

}

AasSubmesh& AasClass2::GetOrCreateSubmeshForMaterial(MaterialRef material) {

	// Check for an existing submesh
	for (auto& submesh : mSubmeshes) {
		if (submesh.material == material) {
			return submesh;
		}
	}

	// Create a new one
	AasSubmesh submesh;
	submesh.material = material;
	mSubmeshes.emplace_back(submesh);

	return mSubmeshes.back();

}

AasModelAndSkeleton::AasModelAndSkeleton(const std::shared_ptr<SkaFile>& skaFile,
                                         const std::shared_ptr<SkmFile>& skmFile,
                                         AasEventListener& eventListener,
                                         gfx::EncodedAnimId animId)
	: skaFile(skaFile), skmFile(skmFile), aas2(*skaFile, *skmFile, eventListener), mEventListener(eventListener), mAnimId(animId) {

	timeLoaded = std::chrono::steady_clock::now();

	// TODO animations[v8].aas_class2_obj->vt->SetEventListener(animations[v8].aas_class2_obj, aasEventListener);

	SetAnimId(animId);
	// AasAnimatedModelSetAnimId(animations[v8].id, animId);
	// AasAnimatedModelAdvance(animations[v8].id, 0.0, 0.0, 0.0, state, &savedregs);

}


// See AasAnimatedModelSetAnimId
void AasModelAndSkeleton::SetAnimId(gfx::EncodedAnimId animId) {

	mAnimId = animId;
	if (aas2.SetAnim(animId.GetName())) {
		return;
	}

	auto fallbackState = 0;

	while (true) {
		if (animId.ToFallback()) {
			if (aas2.SetAnim(animId.GetName())) {
				return;
			}
			continue;
		}

		if (fallbackState)
			break;
		fallbackState = 1;

		if (animId.IsWeaponAnim()) {
			// Try the unarmed_unarmed version of it
			animId = gfx::EncodedAnimId(animId.GetWeaponAnim());

			if (aas2.SetAnim(animId.GetName())) {
				return;
			}
			continue;
		}

		if (fallbackState != 1) {
			throw TempleException("Could not find fallback animation for {} in {}", animId.GetName(), skaFilename);
		}

		fallbackState = 2;
		animId = gfx::EncodedAnimId(gfx::NormalAnimType::ItemIdle);

		if (aas2.SetAnim(animId.GetName())) {
			return;
		}

	}

}

AasRunningAnim::AasRunningAnim(AasClass2& owner, SkaAnimHeader& animHeader)
	: owner(owner), mAnimHeader(animHeader) {

	if (animHeader.streamCount > 0) {

		auto& streamHeader = animHeader.streams[0];

		if (streamHeader.frames > 0) {
			mDuration = (streamHeader.frames - 1) / streamHeader.frameRate;
		}

		mStream = std::make_unique<AasRunningStream>();

	}

}

