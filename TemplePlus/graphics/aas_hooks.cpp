#include "stdafx.h"
#include "util/fixes.h"

#include <temple/dll.h>
#include <infrastructure/meshes.h>
#include <graphics/math.h>
#include "config/config.h"
#include "common.h"
#include "tig/tig_mes.h"
#include "temple/meshes.h"
#include "infrastructure/binaryreader.h"
#include "infrastructure/vfs.h"
#include "tio/tio.h"

using DirectX::XMFLOAT4X3;

#pragma region Structs

#pragma pack(push, 8)

struct Matrix3x4 {
	float r00;
	float r01;
	float r02;
	float t0;
	float mr0;
	float r11;
	float r12;
	float t1;
	float r20;
	float r21;
	float r22;
	float t2;
};

using AasHandle = uint32_t;
struct AnimatedModel;
struct SkaAnimation;
struct LgcySkaFile;
struct AnimPlayer;


struct SkmFile {
	int boneCount;
	void* boneDataStart;
	int variationCount;
	void* variationDataStart;
	int animationCount;
	void* animationDataStart;
	int data;
};


struct SkaFileEntry
{
	char filename[260];
	int refCount;
	LgcySkaFile * fileContents;
};

struct SkaEvent{
	int16_t frameId;
	char type[48];
	char action[128];
};

struct IAasEventListener {
	virtual void Handle() = 0;
	virtual ~IAasEventListener() = 0;
};

// Pure virtual interface to generate a compatible vtable API for AasClass2
struct IAnimatedModel {
	virtual void IAnimatedModelDtor() = 0;
	virtual int SetSkaFile(LgcySkaFile* skaData, int a3, int a4) = 0;
	virtual void SetScale(float scale) = 0;
	virtual int GetBoneCount() = 0;
	virtual char* GetBoneName(int idx) = 0;
	virtual int GetBoneParentId(int idx) = 0;
	virtual int SetSkmFile(SkmFile* skmData, void* matResolver, void* matResolverArg) = 0;
	virtual int FreeAddMeshStuffMaybe(int) = 0;
	virtual int SetEventListener(IAasEventListener* eventListener) = 0;
	virtual void ResetSubmeshes() = 0;
	virtual void Method11() = 0;
	virtual void SetClothFlagSth() = 0;
	virtual void GetSubmeshes(int* submeshCountCount, int* submeshMaterialsOut) = 0;
	virtual void Method14() = 0;
	virtual int HasAnimation(char* anim) = 0;
	virtual int SetAnimIdx(int animIdx) = 0;
	virtual void Advance(const XMFLOAT4X3* worldMatrix, float deltaTime, float deltaDistance, float deltaRotation) = 0;
	virtual void SetWorldMatrix(const Matrix3x4* worldMatrix) = 0;
	virtual void Method19() = 0; // SetBoneMatrices maybe
	virtual void SetTime() = 0;
	virtual void Method21() = 0;
	virtual void GetSubmesh(int submeshIdx, int* vertexCountOut, float** posOut, float** normalsOut, float** uvOut, int* primCountOut, uint16_t** indicesOut) = 0;
	virtual int AddRunningAnim(AnimPlayer* ra, int boneCountSthg) = 0;
	virtual void RemoveRunningAnim() = 0;
	virtual void Method25() = 0;
	virtual void Method26() = 0;
	virtual void HasBone() = 0;
	virtual void AddReplacementMaterial() = 0;
	virtual double GetDistPerSec() = 0;
	virtual void GetRotationPerSec() = 0;
};

struct IAnimPlayer {
	virtual ~IAnimPlayer() = 0;
	virtual void GetDistPerSec(float* speedOut) = 0;
};
#pragma pack(push, 1)
struct AnimPlayerStreamHeader {
	int field0;
	int field1;
	int field2;
	int8_t pad[3];
};

struct AnimPlayerStreamBone
{
	XMFLOAT4 prevScale;
	XMFLOAT4 scale;
	XMFLOAT4 prevRotation;
	XMFLOAT4 rotation;
	XMFLOAT4 prevTranslation;
	XMFLOAT4 translation;
	int16_t scaleFrame;
	int16_t scaleNextFrame;
	int16_t rotationFrame;
	int16_t rotationNextFrame;
	int16_t translationFrame;
	int16_t translationNextFrame;
	int field6C;
};

struct AnimPlayerStream {
	AnimPlayerStreamHeader* header;
	LgcySkaFile* skaFile;
	SkaAnimation* anim;
	int variationId;
	float scaleFactor;
	float translationFactor;
	float currentFrame;
	void* keyframePtr;
	AnimPlayerStreamBone bones[1]; //set according to skafile bone count

	void SetFrame(float frame);
	void ResetBones(SkaAnimation * anim, int variationId);
};

struct AnimPlayerStreamWhole
{
	AnimPlayerStreamHeader header;
	AnimPlayerStream body;
};

const auto asdf = sizeof AnimPlayerStreamWhole;
#pragma pack(pop)

struct AnimPlayer : IAnimPlayer {
	AnimatedModel * ownerAnim;
	int boneIdx;
	char fieldC[4];
	float elapsedTimeRelated;
	float field14;
	int eventHandlingDepth;
	AnimPlayer * nextRunningAnim;
	AnimPlayer * prevRunningAnim;
	SkaAnimation * skaAnimation;
	int streamCount;
	AnimPlayerStream * streams[4];
	float streamRelated[4]; // maybe fps
	int8_t streamVariationIndices[8]; // indices into streamVariationIds
	int8_t streamVariationIds[4];
	float field58; // time related
	float currentTime;
	float duration;
	float frameRate;
	float distancePerSecond;
	void * eventListener;
	int8_t eventCount[4];
	SkaEvent*events;
public:
};


struct VariationSelector {
	int variationId;
	float factor;
};

struct AasClothSphere {
	int boneId;
	float param1;
	XMFLOAT4X3 someMatrix;
	XMFLOAT4X3 someMatrix2;
	AasClothSphere* next;
};

struct AasClothCylinder {
	int boneId;
	float param1;
	float param2;
	XMFLOAT4X3 someMatrix1;
	XMFLOAT4X3 someMatrix2;
	AasClothCylinder* next;
};

struct BoneDistance {
	int16_t vertexInd[2];
	float dist;
};

struct AasClothStuff {
	uint32_t clothVertexCount;
	XMFLOAT4* clothVertexPos1;
	XMFLOAT4* clothVertexPos2;
	XMFLOAT4* clothVertexPos3;
	uint8_t* bytePerVertex;
	uint32_t boneDistancesCount;
	uint32_t boneDistancesCountDelta;
	BoneDistance* boneDistances;
	uint32_t boneDistances2Count;
	uint32_t boneDistances2CountDelta;
	BoneDistance* boneDistances2;
	AasClothSphere* spheres;
	AasClothCylinder* cylinders;
	uint32_t field_34;
};

struct AasClothStuff1 {
	SkmFile* skmFile;
	uint32_t clothVertexCount;
	uint16_t* vertexIdxForClothVertexIdx;
	uint8_t* bytePerClothVertex;
	uint8_t* bytePerClothVertex2;
	AasClothStuff* clothStuff;
	uint32_t* field_18;
};

struct SkmMaterialPair
{
	SkmFile* skmData;
	int materialId;
};

struct AasSubmeshWithMaterial {
	int materialId;
	void* matResolver;
	uint16_t field_8;
	uint16_t vertexCount;
	uint16_t primCount;
	uint16_t field_E;
	uint32_t field_10;
	uint32_t field_14;
	uint32_t field_18;
	XMFLOAT2* uv;
	XMFLOAT4* positions;
	XMFLOAT4* normals;
	uint16_t field_28;
	uint16_t field_2A;
	uint32_t field_2C;
	uint32_t field_30;
	uint32_t field_34;
	uint16_t* indices;
	uint16_t countForOtherSkmFile;
	uint16_t skmMatPairAllocationCount;
	union
	{
		SkmFile* skmFile;
		SkmMaterialPair* skmMatPairs;
	} skmData;
	int otherMaterialId;
};

struct AnimatedModel : IAnimatedModel { // aas_class2
	AnimPlayer* runningAnimsHead;
	AnimPlayer* newestRunningAnim;
	bool submeshesValid;
	float scale;
	float scaleInv;
	LgcySkaFile* skaData;
	uint32_t variationCount;
	VariationSelector variations[8];
	bool hasClothBones;
	uint32_t normalBoneCount;
	uint32_t clothStuff1Count;
	AasClothStuff1* clothStuff1;
	AasClothSphere* clothSpheresHead;
	AasClothCylinder* clothCylindersHead;
	IAasEventListener* eventListener;
	uint16_t submeshCount;
	uint16_t submeshCount2;
	AasSubmeshWithMaterial* submeshes;
	float timeRel1;
	float timeRel2;
	float drivenDistance;
	float drivenRotation;
	Matrix3x4 someMatrix;
	Matrix3x4 worldMatrix;
	Matrix3x4* boneMatrices;
	uint32_t field_F8;
	uint32_t field_FC;

	AnimatedModel();
	~AnimatedModel();

	// interface
	virtual void IAnimatedModelDtor(){};
	virtual int SetSkaFile(LgcySkaFile* skaData, int a3, int a4) { return 0; };
	virtual void SetScale(float scale){};
	virtual int GetBoneCount() { return 0; };
	virtual char* GetBoneName(int idx) { return nullptr; };
	virtual int GetBoneParentId(int idx) { return 0; };
	virtual int SetSkmFile(SkmFile* skmData, void* matResolver, void* matResolverArg) { return 0; };
	virtual int FreeAddMeshStuffMaybe(int) { return 0; };
	virtual int SetEventListener(IAasEventListener* eventListener) { return 0; };
	virtual void ResetSubmeshes(){};
	virtual void Method11(){};
	virtual void SetClothFlagSth(){};
	virtual void GetSubmeshes(int* submeshCountCount, int* submeshMaterialsOut){};
	virtual void Method14(){};
	virtual int HasAnimation(char* anim) { return 0; };
	virtual int SetAnimIdx(int animIdx) { return 0; };
	virtual void Advance(const XMFLOAT4X3* worldMatrix, float deltaTime, float deltaDistance, float deltaRotation){};
	virtual void SetWorldMatrix(const Matrix3x4* worldMatrix){};
	virtual void Method19(){}; // SetBoneMatrices maybe
	virtual void SetTime(){};
	virtual void Method21(){};
	virtual void GetSubmesh(int submeshIdx, int* vertexCountOut, float** posOut, float** normalsOut, float** uvOut, int* primCountOut, uint16_t** indicesOut){};
	virtual int AddRunningAnim(AnimPlayer* ra, int boneCountSthg) { return 0; };
	virtual void RemoveRunningAnim(){};
	virtual void Method25(){};
	virtual void Method26(){};
	virtual void HasBone(){};
	virtual void AddReplacementMaterial(){};
	virtual double GetDistPerSec() { return 0.0; };
	virtual void GetRotationPerSec(){};
};
const int testSizeofAas2 = offsetof( AnimatedModel, clothStuff1Count);
struct AasAnimation {
	AasHandle id;
	gfx::EncodedAnimId animId = gfx::EncodedAnimId(gfx::WeaponAnim::None);
	BOOL freed;
	float floatconst;
	uint32_t timeLoaded;
	AnimatedModel* model;
	SkmFile* skmFile;
	LgcySkaFile* skaFile;
	uint32_t addMeshCount;
	void* addMeshData[32];
	char* addMeshNames[32];
	char* skaFilename;
	char* skmFilename;
};

struct SkaAnimStream {
	uint16_t frames;
	int16_t variationId;
	float frameRate;
	float dps;
	// Offset to the start of the key-frame stream in relation to SkaAnimation start
	uint32_t dataOffset;
};

struct SkaAnimStreamData;

struct SkaAnimHeader {
	char name[64];
	uint8_t driveType;
	uint8_t loopable;
	uint16_t eventCount;
	uint32_t eventOffset;
	uint16_t streamCount;
	uint16_t unk;
	SkaAnimStream streams[10];

	SkaEvent* GetEventData(){
		return (SkaEvent*)(((char*)this) + eventOffset);
	}

	SkaAnimStreamData* GetSkaAnimStreamData(int variationId = 0){
		return (SkaAnimStreamData*)(((char*)this) + streams[variationId].dataOffset);
	}
};

struct SkaAnimStreamFrameData{
	int16_t boneId; 
	int16_t scaleX; 
	int16_t scaleY; 
	int16_t scaleZ; 
	int16_t rotationX; 
	int16_t rotationY; 
	int16_t rotationZ; 
	int16_t rotationW; 
	int16_t translationX; 
	int16_t translationY; 
	int16_t translationZ; 
};

struct SkaAnimStreamData{
	float scaleFactor; 
	float translationFactor; 
	SkaAnimStreamFrameData frameData[1]; // array for initial positions, terminated by boneId = -1; after that the keyframe data begins
};

struct SkaAnimation : SkaAnimHeader{
};

struct SkaBone{
	int16_t flags;
	int16_t parentId;
	char name[40];
	int field_2C;
	float field_30;
	XMFLOAT4 scale;
	XMFLOAT4 rotation;
	XMFLOAT4 translation;
};
const int testSizeofSkaBone = sizeof SkaBone; // 100 (0x64)

struct LgcySkaFile {
	int boneCount;
	int boneDataStart;
	int variationCount;
	int variationDataStart;
	int animationcCount;
	int animationDataStart;
	int unks[19];

	SkaBone* GetSkaBoneData(){
		return (SkaBone*)( ( (char*)this ) + boneDataStart);
	};

	SkaAnimation* GetSkaAnimationData(){
		return (SkaAnimation*)(((char*)this) + animationDataStart);
	};

};

static_assert(temple::validate_size<AasClothSphere, 0x6C>(), "AasClothSphere has the wrong size");
static_assert(temple::validate_size<AasClothCylinder, 0x70>(), "AasClothCylinder has the wrong size");
static_assert(temple::validate_size<AasSubmeshWithMaterial, 0x48>(), "AasSubmeshWithMaterial has the wrong size");
static_assert(temple::validate_size<AasAnimation, 0x12C>(), "AasAnimation has the wrong size");
static_assert(temple::validate_size<AasClothStuff1, 0x1C>(), "AasClothStuff1 has the wrong size");
static_assert(temple::validate_size<AasClothStuff, 0x38>(), "AasClothStuff has the wrong size");

#pragma pack(pop)

#pragma endregion

static class AasHooks : public TempleFix {
public:

	static constexpr int AAS_FILE_NOT_FOUND = 4; 
	static constexpr int AAS_ERROR = 1;
	static constexpr int AAS_OK = 0;

	// 5000 anims max
	static constexpr size_t sMaxAnims = 5000;
	AasAnimation* mAnims;

	bool IsValidHandle(AasHandle handle) const {
		return handle < sMaxAnims && !mAnims[handle].freed;
	}

	static int CreateModelByIds(int skmId, int skaId, int idleAnimId, temple::AasAnimParams* animState, int* handleOut);

		static int GetSkaFileContents(LgcySkaFile**, const char*);
		static int GetSkmFileContents(SkmFile**, const char*);

		static void SkaFileEntryRelease(LgcySkaFile *);
		static int __stdcall SetSubmesh(AasSubmeshWithMaterial* submesh, SkmFile* skmData, int materialIdx);

		static int SetSkaFile(LgcySkaFile* skaFile, int, int);

		static int SetAnimIdx_Impl(AnimatedModel* aasObj, int a3, int animIdx, IAasEventListener* evtLisnr);
		static void SetStreamFrame(AnimPlayerStream* stream, float frame);


	static int sub_10268910_naked();
	//static Matrix3x4* sub_10268910(Matrix3x4* transformationMat, char* boneName, AnimatedModel* aasObj);

	void apply() override {

		static auto instance = this;
		mAnims = temple::GetPointer<AasAnimation>(0x10EFB900);


		replaceFunction(0x10266810, SetSkaFile);
		replaceFunction(0x102665E0, SetSubmesh);
		replaceFunction(0x1026A680, SetAnimIdx_Impl);
		replaceFunction(0x1026B740, SetStreamFrame);
		//AasCreateModelByIds
		replaceFunction(0x102641B0, CreateModelByIds);



		// AasAnimatedModelGetSubmeshes
		replaceFunction<int(AasHandle, int**, int*)>(0x10263a50, [](AasHandle handle, int** submeshMaterials, int* submeshCountOut) {
			    if (!instance->IsValidHandle(handle)) {
				    return AAS_ERROR;
			    }

			    *submeshCountOut = 25;
			    static int sSubmeshMaterials[25];
			    auto& anim = instance->mAnims[handle];
			    anim.model->GetSubmeshes(submeshCountOut, &sSubmeshMaterials[0]);
			    *submeshMaterials = &sSubmeshMaterials[0];

			    return AAS_OK;
		    });

		// AasAdvance
		static int(__cdecl*orgAasAdvance)(AasHandle, float, float, float, void*, void*)	
		= replaceFunction<int (__cdecl)(AasHandle, float, float, float, void*, void*)>(0x10262C10,
			[](AasHandle handle, float timeAmt, float distAmt, float rotAmt, void* animParams, void* a6){
			
			if (distAmt > 0){
				auto dummy = 1;
			}
			if (rotAmt != 0.0f)
			{
				auto dumy = 1;
			}

			if (handle >= 0 && handle < sMaxAnims){
				auto &aasAnim = aasHooks.mAnims[handle];
				auto model = aasAnim.model;
				if (model){
					AnimPlayer* runningAnim = model->runningAnimsHead;

					auto dummy = 1;
				}
			}

			return orgAasAdvance(handle, timeAmt, distAmt, rotAmt, animParams, a6);
		});

		// GetDistPerSec
		static double (__cdecl*orgGetDistPerSec)(AasHandle) = replaceFunction<double(AasHandle)>(0x10263DA0, [](AasHandle handle)->double
		{
			if (!handle || handle >= sMaxAnims)
				return 0.0;
			auto &aasAnim = aasHooks.mAnims[handle];
			if (aasAnim.freed & 1){
				return 0.0;
			}

			auto model = aasAnim.model;
			auto val = model->GetDistPerSec();

			auto speed = 0.0f;
			AnimPlayer* runningAnim = model->runningAnimsHead;
			while (runningAnim){
				auto nextRunningAnim = runningAnim->nextRunningAnim;
				//runningAnim->GetDistPerSec(&speed);

				if (runningAnim->skaAnimation->driveType == 1 && runningAnim->field14 > 0.0){

					auto isWalk = strstr(runningAnim->skaAnimation->name, "walk");
					auto isSneak = strstr(runningAnim->skaAnimation->name, "sneak");

					if (config.equalizeMoveSpeed){ // *config.speedupFactor; // disregard scaling, equalize across different models

						// walking animations - leave as is
						if (isWalk){
							speed = runningAnim->distancePerSecond;
						}
						else if (isSneak){
							if (config.fastSneakAnim)
								speed = 190;
							else
								speed = runningAnim->distancePerSecond;
						}
						else{ // should be just running animations now
							if (runningAnim->distancePerSecond < 180)
								speed = 190; // to equalize with summoned monsters	
							else
								speed = runningAnim->distancePerSecond;
						}								
					}
					else{
						if (isSneak && config.fastSneakAnim){
							speed = runningAnim->ownerAnim->scale * 190;
						} 
						else
							speed = runningAnim->ownerAnim->scale * runningAnim->distancePerSecond;
	
					}
						
				}

				runningAnim = nextRunningAnim;
			}

			// return val;
			if (speed > 0)
				return speed;
			return val; // failsafe!
		});

		/*static void(__thiscall*orgGetDistPerSec_Class2_Impl)() = replaceFunction<void(__thiscall)()>(0x1026AB10, [](){
			return orgGetDistPerSec_Class2_Impl();
		});*/

		/*static BOOL(__cdecl*orgAas_10263400)(uint32_t, void**, void*) =	replaceFunction<BOOL(__cdecl)(uint32_t , void** , void* )>(0x10263400, 
				[](uint32_t aasHndl, void** aasUnk1C, void* animState){
			return orgAas_10263400(aasHndl, aasUnk1C, animState);
		});*/

		//replaceFunction(0x10268910, sub_10268910_naked); // causes crashes, unknown reason

		replaceFunction<BOOL(int, char*)>(0x100041E0, [](int id, char* fileOut)
		{
			MesLine line(id);
			
			auto mesFile = temple::GetRef<MesHandle>(0x10307168);
			if (mesFuncs.GetLine(mesFile, &line))
			{
				_snprintf(fileOut, 260, "art\\meshes\\%s.skm", line.value);
				return FALSE;
			}
			return TRUE;
		});


	}
} aasHooks;


Matrix3x4*  sub_10268910(Matrix3x4* transformationMat, char* boneName, AnimatedModel* aasObj) {
	
	auto skaData = aasObj->skaData;
	auto boneIdx = -1;

	auto gfds = skaData->GetSkaBoneData();
	for (auto i=0; i < skaData->boneCount; i++){
		auto asdf = skaData[i];
		auto &bData = gfds[i];

		auto boneNameIt = (const char*)&skaData[i].boneDataStart + skaData->boneDataStart;
		if (!_stricmp(boneName, boneNameIt)){
			boneIdx = i;
			break;
		}
	}
	if (boneIdx == -1){
		auto defaultTransMat = temple::GetRef<Matrix3x4>(0x11069E58);
		*transformationMat = defaultTransMat;
		return transformationMat;
	}

	aasObj->Method19();
	*transformationMat = aasObj->boneMatrices[boneIdx];
	return transformationMat;
}

int AasHooks::CreateModelByIds(int skmId, int skaId, int idleAnimId, temple::AasAnimParams * animState, int * handleOut){

	auto animHandle = 1;
	auto foundHandle = false;

	auto animations = aasHooks.mAnims;

	for (auto i = 1; i < sMaxAnims; i++) {
		if (animations[i].freed & 1) {
			animHandle = i;
			foundHandle = true;
			break;
		}
	}
	if (!foundHandle) {
		logger->error("AAS: SEVERE ERROR: max number of animations exceeded ({}).", sMaxAnims);
		return AAS_ERROR;
	}

	AasAnimation &anim = animations[animHandle];

	animations[animHandle].freed = 0;
	animations[animHandle].animId = gfx::EncodedAnimId(gfx::WeaponAnim::None);
	animations[animHandle].floatconst = 6.3940001f;
	animations[animHandle].addMeshCount = 0;
	animations[animHandle].id = animHandle;

	// Get SKA data
	{
		auto getSkaFile = temple::GetRef<BOOL(__cdecl*)(int, const char*)>(0x11069C60);
		char skaFilename[260];
		if (getSkaFile(skaId, skaFilename)) {
			logger->error("AAS: Could not build anim file for meshes.mes entry {}", skaId);
			return AAS_ERROR;
		}

		auto result = GetSkaFileContents(&anim.skaFile, skaFilename);
		if (result != AAS_OK)
			return result;
		anim.skaFilename = _strdup(skaFilename);
	}

	// Get SKM data
	{
		auto getSkmFile = temple::GetRef<BOOL(__cdecl*)(int, const char*)>(0x11069C74);
		char skmFilename[260];
		if (getSkmFile(skmId, skmFilename))
		{
			logger->error("AAS: Could not build model file for meshes.mes entry {}", skmId);
			SkaFileEntryRelease(anim.skaFile);
			return AAS_ERROR;
		}
		if (GetSkmFileContents(&anim.skmFile, skmFilename) != AAS_OK)
		{
			SkaFileEntryRelease(anim.skaFile);
			return AAS_ERROR;
		}
		anim.skmFilename = _strdup(skmFilename);
	}

	anim.model = new AnimatedModel;
	anim.model->SetSkaFile(anim.skaFile, 0, 0);
	anim.model->SetSkmFile(anim.skmFile, temple::GetRef<int(__cdecl*)(const char*)>(0x10EFB8F4), 0);
	anim.model->SetEventListener(temple::GetRef<IAasEventListener*>(0x11069C70));
	anim.timeLoaded = timeGetTime();

	auto aasAnimSet = temple::GetRef<void(__cdecl)(int, int)>(0x10262540);
	aasAnimSet(anim.id, idleAnimId);

	int asdf;
	auto unk = temple::GetRef<void(__cdecl)(int, int, int, int, temple::AasAnimParams*, int*)>(0x10262C10);
	unk(anim.id, 0, 0, 0, animState, &asdf);

	/*std::vector<AasSubmeshWithMaterial> asdf2;
	for (auto i=0; i<anim.model->submeshCount; i++){
	asdf2.push_back(anim.model->submeshes[i]);
	}*/

	//asdf.push_back(anim.model->submeshes[0]);
	*handleOut = anim.id;

	return AAS_OK;

}

int AasHooks::GetSkaFileContents(LgcySkaFile **skaFile , const char *filename)
{
	if (!skaFile)
		return AAS_ERROR;

	// look up in cache
	auto &skaEntries = temple::GetRef<SkaFileEntry*>(0x11069C68);
	auto &skaEntriesCount = temple::GetRef<int>(0x11069C64);
	
	auto skaEntry = (SkaFileEntry*)bsearch(filename, skaEntries, skaEntriesCount, sizeof SkaFileEntry, [](const void*a, const void*b)
	{
		return _stricmp((const char*)a,(const char*) b);
	} );

	if (skaEntry){
		skaEntry->refCount++;
		*skaFile = skaEntry->fileContents;
		return AAS_OK;
	}

	// not in cache - load file
	try{
		auto fh = tio_fopen(filename, "rb");
		auto fileLen = tio_filelength(fh);
		auto fcont = (LgcySkaFile*)malloc(fileLen);
		tio_fread(fcont, 1,fileLen , fh);
		tio_fclose(fh);
		*skaFile = fcont;

		skaEntries=(SkaFileEntry*)realloc(skaEntries, (skaEntriesCount + 1) * sizeof(SkaFileEntry));
		strncpy(skaEntries[skaEntriesCount].filename, filename, 260);
		skaEntries[skaEntriesCount].refCount = 1;
		skaEntries[skaEntriesCount].fileContents = fcont;
		// sort array
		auto arraySort = temple::GetRef<void(__cdecl)(void*, int, size_t, int(__cdecl*)(void*, void*))>(0x10254750);
		arraySort(skaEntries, skaEntriesCount+1, sizeof(SkaFileEntry), [](void*a, void*b){
			return _stricmp((const char*)a,(const char*)b);
		});
		skaEntriesCount++;
		
		return AAS_OK;
	}
	catch (TempleException e){
		*skaFile = nullptr;
		logger->error("AAS: Animation file {}not found", filename);
		return AAS_FILE_NOT_FOUND;
	}
	
	return AAS_OK;
}

int AasHooks::GetSkmFileContents(SkmFile ** skmFile, const char * filename)
{
	if (!skmFile)
	{
		return AAS_FILE_NOT_FOUND;
	}
	auto fh = tio_fopen(filename, "rb");
	if (!fh)
	{
		return AAS_FILE_NOT_FOUND;
	}
	auto fileLen = tio_filelength(fh);
	auto fcont = (SkmFile*)malloc(fileLen);
	tio_fread(fcont, 1, fileLen, fh);
	tio_fclose(fh);
	*skmFile = fcont;

	return AAS_OK;
}

void AasHooks::SkaFileEntryRelease(LgcySkaFile *skaFile){
	auto &skaEntries = temple::GetRef<SkaFileEntry*>(0x11069C68);
	auto &skaEntriesCount = temple::GetRef<int>(0x11069C64);

	if (!skaEntriesCount)
		return;

	for (auto i =0; i < skaEntriesCount;i++)
	{
		auto skaEntry = &skaEntries[i];
		if (skaFile != skaEntry->fileContents)
			continue;

		if (skaEntry->refCount-- <= 1){
			free(skaEntry->fileContents);
			*skaEntry = skaEntries[skaEntriesCount - 1];
			skaEntriesCount--;

			// sort array
			auto arraySort = temple::GetRef<void(__cdecl)(void*, int, size_t, int(__cdecl*)(void*, void*))>(0x10254750);
			arraySort(skaEntries, skaEntriesCount + 1, sizeof(SkaFileEntry), [](void*a, void*b) {
				return _stricmp((const char*)a, (const char*)b);
			});
			return;
		}
	}
}


int __stdcall AasHooks::SetSubmesh(AasSubmeshWithMaterial * submesh, SkmFile * skmData, int materialIdx){
	if (!submesh->skmMatPairAllocationCount){
		if (!submesh->countForOtherSkmFile){
			submesh->countForOtherSkmFile = 1; // as far as I can tell it only every executes this section since it's only executed after initializing a submesh -SA
			submesh->skmData.skmFile = skmData;
			submesh->otherMaterialId = materialIdx;
			return 0;
		}
		if (submesh->skmData.skmFile == skmData && submesh->otherMaterialId){
			return 0;
		}

		auto newSkmPair = new SkmMaterialPair[2];
		if (!newSkmPair)
			return -1;
		newSkmPair[0].skmData = submesh->skmData.skmFile;
		newSkmPair[0].materialId = submesh->otherMaterialId;
		newSkmPair[1].skmData = skmData;
		newSkmPair[1].materialId = materialIdx;

		submesh->skmData.skmMatPairs = newSkmPair;
		submesh->skmMatPairAllocationCount = 2;
		return submesh->countForOtherSkmFile++;
	}

	// search existing ones first
	for (auto i=0; i <= submesh->countForOtherSkmFile; i++){
		if (submesh->skmData.skmFile == skmData && submesh->materialId == materialIdx){
			return i;
		}
	}

	// if none found, allocate new entry
	auto count = submesh->countForOtherSkmFile;
	auto newCount = count + 2;
	auto newSkmPairs = new SkmMaterialPair[newCount];
	if (!newSkmPairs)
		return -1;
	memcpy(newSkmPairs, submesh->skmData.skmMatPairs, sizeof(SkmMaterialPair)*count);
	newSkmPairs[count].skmData = skmData;
	newSkmPairs[count].materialId = materialIdx;
	free(submesh->skmData.skmMatPairs);
	submesh->skmMatPairAllocationCount = newCount;
	submesh->skmData.skmMatPairs = newSkmPairs;
	submesh->countForOtherSkmFile++;
	return count;


	return 0;
}

int __cdecl SetSkaFile_(AnimatedModel* aasObj, LgcySkaFile* skaFile){
	
	if (aasObj->skaData)
		return -1;

	aasObj->skaData = skaFile;
	aasObj->scale = 1.0f;
	aasObj->scaleInv = 1.0f;
	aasObj->variationCount = 1;
	aasObj->variations[0].variationId = -1;
	aasObj->variations[0].factor = 1.0f;
	aasObj->normalBoneCount = 0;
	aasObj->boneMatrices = new Matrix3x4[skaFile->boneCount + 1];

	auto skaBoneData = skaFile->GetSkaBoneData();
	// count normal bones
	for (auto i=0; i < skaFile->boneCount; i++){
		
		if (!_stricmp("#ClothBone", skaBoneData[i].name)){
			break;
		}
		aasObj->normalBoneCount++;
	}
	aasObj->hasClothBones = skaFile->boneCount > aasObj->normalBoneCount;
	aasObj->clothStuff1Count = 0;

	if (!aasObj->hasClothBones)
		return 0;

	for (auto i = 0; i < skaFile->boneCount; i++) {

		auto namePos = skaBoneData[i].name;
		if (!_strnicmp("#Sphere", skaBoneData[i].name, 7)){
			while (*namePos) {
				if (*namePos == '{')
					break;
				namePos++;
			}
			auto param1 = 0.0f;
			if (*namePos == '{') {
				param1 = atof(namePos + 1);
			}

			auto newClothSphere = new AasClothSphere;
			newClothSphere->param1 = param1;
			newClothSphere->next = nullptr;
			newClothSphere->boneId = i;
			auto sphereHead = aasObj->clothSpheresHead;
			if (!sphereHead) {
				aasObj->clothSpheresHead = newClothSphere;
			}
			else //append to end of list
			{
				for (auto k = sphereHead->next; k; k = k->next)
					sphereHead = k;
				sphereHead->next = newClothSphere;
			}
			continue;
		}

		if (!_strnicmp("#Cylinder", skaBoneData[i].name, 9)) {
			while (*namePos){
				if (*namePos == '{')
					break;
				namePos++;
			}
			auto cylinderParam1 = 0.0f;
			auto cylinderParam2 = 0.0f;
			if (*namePos == '{'){
				cylinderParam1 = atof(namePos + 1);
				while (*namePos){
					if (*namePos == ',')
						break;
					namePos++;
				}
				if (*namePos == ',')
					cylinderParam2 = atof(namePos + 1);
			}

			auto newClothCyl = new AasClothCylinder;
			newClothCyl->param1 = cylinderParam1;
			newClothCyl->param2 = cylinderParam2;
			newClothCyl->next = nullptr;
			newClothCyl->boneId = i;
			auto cylHead = aasObj->clothCylindersHead;
			if (!cylHead){
				aasObj->clothCylindersHead = newClothCyl;
			}
			else //append to end of list
			{
				for (auto k = cylHead->next; k; k = k->next)
					cylHead = k;
				cylHead->next = newClothCyl;
			}
			continue;
		}
	}

	return 0;
};

int __declspec(naked) AasHooks::SetSkaFile(LgcySkaFile * skaFile, int, int)
{
	__asm {
		push[esp + 4]; // skaFile  (note: the other 2 args aren't used)
		mov eax, ecx; // AnimatedModel*
		push eax;
		mov eax, SetSkaFile_;
		call eax;
		add esp, 8;
		ret 0xC; // due to __stdcall convention
	}
}


AnimPlayerStream * CreateAnimPlayerStream(LgcySkaFile* skaFile){
	auto wholeResult = (AnimPlayerStreamWhole*)malloc(sizeof(AnimPlayerStreamBone) * skaFile->boneCount + 47);
	if (!wholeResult)
		return nullptr;

	auto result = &wholeResult->body;

	result->skaFile = skaFile;
	result->header = &wholeResult->header;
	result->anim = nullptr;
	
	return result;
}


int SetAnimIdx_Impl_(AnimPlayer *ra, AnimatedModel * aasObj, int boneCountSthg, int animIdx, IAasEventListener * evtLisnr) {

	if (ra->streamCount || ra->ownerAnim || !aasObj){
		return -1;
	}
	
	auto skaData = aasObj->skaData;
	if (!skaData || boneCountSthg < -1 || boneCountSthg >= skaData->boneCount || animIdx < 0 || animIdx >= skaData->animationcCount)
		return -1;

	auto skaAnim = &skaData->GetSkaAnimationData()[animIdx];

	ra->skaAnimation = skaAnim;
	ra->distancePerSecond = skaAnim->streams[0].dps;
	ra->frameRate = skaAnim->streams[0].frameRate;

	ra->eventListener = evtLisnr;
	ra->eventCount[0] = skaAnim->eventCount;
	if(ra->eventCount[0]){
		ra->events = skaAnim->GetEventData();
	}
	else{
		ra->events = nullptr;
	}

	auto variationCount = aasObj->variationCount;
	ra->streamCount = 0;


	//create the streams according to the variations
	const int sStreamVariationsMax = 4;
	int streamVariations[sStreamVariationsMax];

	for (auto i = 0; i <variationCount; i++){
		auto variationSel = aasObj->variations[i];
		auto variationId = -1;

		for (auto j = 0; j < skaAnim->streamCount; j++){
			if (skaAnim->streams[j].variationId == variationSel.variationId 
				|| variationId < 0  && skaAnim->streams[j].variationId == -1){
				variationId = j;
			}
		}

		ra->streamVariationIndices[i] = 4; // default value?

		if (variationId < 0){
			continue;
		}

		auto variationIdIndex = 0;
		for (; variationIdIndex < ra->streamCount; variationIdIndex++){
			if (streamVariations[variationIdIndex] == variationId){
				break;
			}
		}
		ra->streamVariationIndices[i] = variationIdIndex;

		// if not found in stream variations
		if (variationIdIndex == ra->streamCount && ra->streamCount < 4){ 
			streamVariations[variationIdIndex] = variationId;
			
			auto newRunningAnimStream = CreateAnimPlayerStream(skaData);
			ra->streams[variationIdIndex] = newRunningAnimStream;
			if (!newRunningAnimStream)
				return -1;
			auto animPlayerStreamInit = temple::GetRef<void(__cdecl)(AnimPlayerStream*, SkaAnimation*, int)>(0x1026B110);
			animPlayerStreamInit(newRunningAnimStream, skaAnim, variationId);
			ra->streamRelated[variationIdIndex] = 0;
			ra->streamVariationIds[variationIdIndex] = skaAnim->streams[variationIdIndex].variationId;
			ra->streamCount++;
		}

	}


	// set the running anim duration
	for (auto i=0; i < variationCount; i++){

		auto streamSelect = ra->streamVariationIndices[i];
		auto factor = aasObj->variations[i].factor;

		if (streamSelect < sStreamVariationsMax){
			auto frames = ra->skaAnimation->streams[streamVariations[streamSelect]].frames;
			auto frameRate = ra->skaAnimation->streams[streamVariations[streamSelect]].frameRate;
			if (frames > 1 && frameRate > 0.0){
				ra->duration = (frames - 1) / frameRate * factor + (1.0 - factor) * ra->duration;
			}
		}
	}

	// set stream fps
	if (ra->duration > 0.0){
		for (auto i=0; i < ra->streamCount; i++){
			auto frames = ra->skaAnimation->streams[streamVariations[i]].frames;
			if (frames > 1){
				ra->streamRelated[i] = (frames - 1) / ra->duration;
			}
		}
	}

	// add running anim
	if (aasObj->AddRunningAnim(ra, boneCountSthg))
		return -1;

	// que??
	if (boneCountSthg == -1){
		ra->fieldC[1] = 1;
	}

	return 0;
}

int __declspec(naked) AasHooks::SetAnimIdx_Impl(AnimatedModel * aasObj, int a3, int animIdx, IAasEventListener * evtLisnr){

	__asm {
		push[esp + 16];
		push[esp + 16];
		push[esp + 16];
		push[esp + 16];
		mov eax, ecx; // AasRunningAnim*
		push eax;
		mov eax, SetAnimIdx_Impl_;
		call eax;
		add esp, 20;
		ret 0x10; // due to __stdcall convention
	}
}

void AasHooks::SetStreamFrame(AnimPlayerStream * stream, float frame){
	stream->SetFrame(frame);
}


int __declspec(naked) AasHooks::sub_10268910_naked(){

	macAsmProl; // esp + 16, arg1 is esp+20
	__asm {
		push ecx; // AasClass2 *     // arg1 is esp +24
		mov eax, [esp + 28];
		push eax;      // arg1 is esp +28
		mov eax, [esp + 28];   
		push eax;
		mov edi, sub_10268910;
		call edi;
		add esp, 12;

	};

	macAsmEpil;
	__asm retn;
};

AnimatedModel::AnimatedModel(){

	this->skaData = nullptr;
	this->submeshesValid = 0;
	this->submeshCount = 0;
	this->submeshCount2 = 0;
	this->submeshes = 0;
	this->newestRunningAnim = 0;
	this->runningAnimsHead = 0;
	this->worldMatrix = temple::GetRef<Matrix3x4>(0x11069E58); 
	this->boneMatrices = 0;
	this->eventListener = 0;
	this->hasClothBones = 0;
	this->normalBoneCount = 0;
	this->clothStuff1Count = 0;
	this->clothSpheresHead = 0;
	this->clothCylindersHead = 0;
	this->timeRel1 = 0;
	this->timeRel2 = 0;
	this->drivenDistance = 0;
	this->drivenRotation = 0;

	// override the virtual table with ToEE's
	*(int**)(this) = temple::GetPointer<int>(0x102A8DC8);

}

AnimatedModel::~AnimatedModel()
{
}

void AnimPlayerStream::SetFrame(float frame){
	auto frameRounded = floor(frame);
	if (floor(this->currentFrame) == frameRounded) {
		this->currentFrame = frame;
		return;
	}

	if (frame < this->currentFrame) {
		SetFrame(32766.0); // run to end
		ResetBones(this->anim, this->variationId);
	}

	// set currentFrame
	this->currentFrame = frame;
	if (frameRounded >= 32767)
		frameRounded = 32766;

	auto sFactor = this->scaleFactor;
	auto tFactor = this->translationFactor;
	const auto rFactor = 1.0 / 32767.0;

	int16_t* keyframeData = (int16_t*)this->keyframePtr;
	auto keyframeFrame = (*(uint16_t*)keyframeData)  / 2;
	
	// advance the frames
	while (keyframeFrame <= frameRounded){
		
		keyframeData++;
		auto flags = *(uint16_t*)keyframeData;
		while (flags & 1){

			auto boneId = flags >> 4;
			auto &tBone = this->bones[boneId];
			keyframeData++;

			if (!(flags & 0xE)){
				flags = *(uint16_t*)keyframeData;
				continue;
			}

			if (flags & 8) // scale
			{
				tBone.scaleFrame = keyframeFrame;
				tBone.scaleNextFrame = *keyframeData;
				tBone.prevScale = tBone.scale;

				keyframeData++;
				tBone.scale.x = (*keyframeData) * sFactor;
				keyframeData++;
				tBone.scale.y = (*keyframeData) * sFactor;
				keyframeData++;
				tBone.scale.z = (*keyframeData) * sFactor;

				auto one_over_frame_delta = 0.0;
				if (keyframeFrame < tBone.scaleNextFrame)
					one_over_frame_delta = 1.0 / (tBone.scaleNextFrame - keyframeFrame);
				tBone.prevScale.w = one_over_frame_delta;

				keyframeData++;
			}

			if (flags & 4) // rotation
			{
				tBone.rotationFrame = keyframeFrame;
				tBone.rotationNextFrame = *keyframeData;
				tBone.prevRotation = tBone.rotation;
				
				keyframeData++;
				tBone.rotation.x = (*keyframeData) * rFactor;
				keyframeData++;
				tBone.rotation.y = (*keyframeData) * rFactor;
				keyframeData++;
				tBone.rotation.z = (*keyframeData) * rFactor;
				keyframeData++;
				tBone.rotation.w = (*keyframeData) * rFactor;

				auto one_over_frame_delta = 0.0;
				if (keyframeFrame < tBone.rotationNextFrame)
					one_over_frame_delta = 1.0 / (tBone.rotationNextFrame - keyframeFrame);
				tBone.scale.w = one_over_frame_delta;

				keyframeData++;
			}

			if (flags & 2) // translation
			{
				tBone.translationFrame = keyframeFrame;
				tBone.translationNextFrame = *keyframeData;
				tBone.prevTranslation.x = tBone.translation.x;
				tBone.prevTranslation.y = tBone.translation.y;
				tBone.prevTranslation.z = tBone.translation.z;

				keyframeData++;
				int transX = (*keyframeData);
				tBone.translation.x = transX * tFactor;
				keyframeData++;
				int transY = (*keyframeData);
				tBone.translation.y = transY * tFactor;
				keyframeData++;
				int transZ = (*keyframeData);
				tBone.translation.z = transZ * tFactor;

				auto one_over_frame_delta = 0.0;
				if (keyframeFrame < tBone.translationNextFrame)
					one_over_frame_delta = 1.0 / (tBone.translationNextFrame - keyframeFrame);
				tBone.translation.w = one_over_frame_delta;

				keyframeData++;
			}

			flags = *(uint16_t*)keyframeData;
		}

		this->keyframePtr = keyframeData;
		keyframeFrame = (*(uint16_t*)keyframeData) / 2;
	}

}

void AnimPlayerStream::ResetBones(SkaAnimation * anim, int variationId){

	this->anim = anim;
	this->variationId = variationId;

	auto skaFile = this->skaFile;
	auto boneCount = skaFile->boneCount;
	auto skaBones = skaFile->GetSkaBoneData();
	auto streamBones = this->bones;

	for (auto i=0; i <boneCount; i++){
		auto &sBone = streamBones[i];
		auto &fBone = skaBones[i];
		sBone.scale = fBone.scale;
		sBone.prevScale = sBone.scale;

		sBone.rotation = fBone.rotation;
		sBone.prevRotation = sBone.rotation;

		sBone.translation = fBone.translation;
		sBone.prevTranslation = sBone.translation;

		sBone.scaleFrame = sBone.scaleNextFrame = sBone.translationFrame = sBone.translationNextFrame = 0;
		sBone.rotationFrame = sBone.rotationNextFrame = 0;
		sBone.field6C = 0;
	}

	auto streamData = anim->GetSkaAnimStreamData(variationId);

	auto sFactor = streamData->scaleFactor;
	auto tFactor = streamData->translationFactor;
	auto rFactor = 1.0/32767.0;
	this->scaleFactor = sFactor;
	this->translationFactor = tFactor;
	auto frameData = streamData->frameData;
	for (; frameData->boneId >= 0; frameData++) {
		auto boneId = frameData->boneId;
		auto &sBone = streamBones[boneId];
		sBone.scale.x = sFactor * frameData->scaleX;
		sBone.scale.y = sFactor * frameData->scaleY;
		sBone.scale.z = sFactor * frameData->scaleZ;
		sBone.prevScale.x = sBone.scale.x;
		sBone.prevScale.y = sBone.scale.y;
		sBone.prevScale.z = sBone.scale.z;

		sBone.translation.x = tFactor * frameData->translationX;
		sBone.translation.y = tFactor * frameData->translationY;
		sBone.translation.z = tFactor * frameData->translationZ;
		sBone.prevTranslation.x = sBone.translation.x;
		sBone.prevTranslation.y = sBone.translation.y;
		sBone.prevTranslation.z = sBone.translation.z;

		sBone.rotation.x = rFactor * frameData->rotationX;
		sBone.rotation.y = rFactor * frameData->rotationY;
		sBone.rotation.z = rFactor * frameData->rotationZ;
		sBone.rotation.w = rFactor * frameData->rotationW;
		sBone.prevRotation.x = sBone.rotation.x;
		sBone.prevRotation.y = sBone.rotation.y;
		sBone.prevRotation.z = sBone.rotation.z;
	}

	this->currentFrame = -1.0;
	this->keyframePtr = ((int16_t*)frameData) + 1;
	SetFrame(0.0);

	// temple::GetRef<void(__cdecl)(AnimPlayerStream*, SkaAnimation *,int)>(0x1026B110)(this, anim, variationId);
}
