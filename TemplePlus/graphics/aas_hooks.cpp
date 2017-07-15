#include "stdafx.h"
#include "util/fixes.h"

#include <temple/dll.h>
#include <infrastructure/meshes.h>
#include <graphics/math.h>
#include "config/config.h"

using DirectX::XMFLOAT4X3;

#pragma pack(push, 8)

using AasHandle = uint32_t;
struct AnimatedModel;
struct SkaAnimation;
struct SkmFile {
};

struct SkaFile {
};

struct IAasEventListener {
	virtual void Handle() = 0;
	virtual ~IAasEventListener() = 0;
};

// Pure virtual interface to generate a compatible vtable API for AasClass2
struct IAnimatedModel {
	virtual ~IAnimatedModel() = 0;
	virtual int SetSkaFile(SkaFile* skaData, int a3, int a4) = 0;
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
	virtual void SetWorldMatrix(const XMFLOAT4X3* worldMatrix) = 0;
	virtual void Method19() = 0;
	virtual void SetTime() = 0;
	virtual void Method21() = 0;
	virtual void GetSubmesh(int submeshIdx, int* vertexCountOut, float** posOut, float** normalsOut, float** uvOut, int* primCountOut, uint16_t** indicesOut) = 0;
	virtual void AddRunningAnim() = 0;
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

struct AnimPlayer : IAnimPlayer {
	AnimatedModel * ownerAnim;
	int boneIdx;
	int fieldC;
	float elapsedTimeRelated;
	float field14;
	int eventHandlingDepth;
	AnimPlayer * nextRunningAnim;
	AnimPlayer * prevRunningAnim;
	SkaAnimation * skaAnimation;
	int streamCount;
	void * streams[4];
	void * streamRelated[4];
	int field4C;
	int field50;
	void* streamStuff;
	float field58;
	float currentTime;
	float duration;
	float frameRate;
	float distancePerSecond;
	void * eventListener;
	int eventCount;
	void*events;
public:
};


struct VariationSelector {
	int variationId;
	float factor;
};
const int asdf = offsetof(AnimPlayer, field4C);

struct AasClothSphere {
	int boneId;
	int param1;
	XMFLOAT4X3 someMatrix;
	XMFLOAT4X3 someMatrix2;
	AasClothSphere* next;
};

struct AasClothCylinder {
	int boneId;
	int param1;
	int param2;
	XMFLOAT4X3 someMatrix1;
	XMFLOAT4X3 someMatrix2;
	AasClothCylinder* next;
};

struct AasClothStuff {
	uint32_t field_0;
	XMFLOAT4* clothVertexPos1;
	XMFLOAT4* clothVertexPos2;
	XMFLOAT4* clothVertexPos3;
	uint8_t* bytePerVertex;
	uint32_t field_14;
	uint32_t field_18;
	uint32_t field_1C;
	uint32_t field_20;
	uint32_t field_24;
	uint32_t field_28;
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

struct AasSubmesh {
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
	uint16_t countForOtherSkmFile_;
	uint16_t countForOtherSkmFile;
	SkmFile* otherSkmFile;
	int* otherMaterialId;
};

struct AnimatedModel : IAnimatedModel {
	AnimPlayer* runningAnimsHead;
	AnimPlayer* newestRunningAnim;
	bool submeshesValid;
	float scale;
	float scaleInv;
	SkaFile* skaData;
	uint32_t variationCount;
	VariationSelector variations[8];
	bool hasClothBones;
	uint32_t clothBoneId;
	uint32_t clothStuff1Count;
	AasClothStuff1* clothStuff1;
	AasClothSphere* clothSpheresHead;
	AasClothCylinder* clothCylindersHead;
	IAasEventListener* eventListener;
	uint16_t submeshCount;
	uint16_t field_7E;
	AasSubmesh* submeshes;
	float timeRel1;
	float timeRel2;
	float drivenDistance;
	float drivenRotation;
	XMFLOAT4X3 someMatrix;
	XMFLOAT4X3 worldMatrix;
	XMFLOAT4X3* boneMatrices;
	uint32_t field_F8;
	uint32_t field_FC;
};

struct AasAnimation {
	AasHandle id;
	gfx::EncodedAnimId animId = gfx::EncodedAnimId(gfx::WeaponAnim::None);
	BOOL freed;
	float floatconst;
	uint32_t timeLoaded;
	AnimatedModel* model;
	SkmFile* skmFile;
	SkaFile* skaFile;
	uint32_t addMeshCount;
	void* addMeshData[32];
	char* addMeshNames[32];
	char* skaFilename;
	char* skmFilename;
};

struct SkaAnimation
{
	char name[64];
	char driveType;
	char loopable;
	int16_t eventCount;
	int eventOffset;
	int streamCount;
	int16_t frameCount;
	int16_t variationId;
	float frameRate;
	float distancePerSecond;
	int dataOffset;
	char field5C[144];
};

static_assert(temple::validate_size<AasClothSphere, 0x6C>(), "AasClothSphere has the wrong size");
static_assert(temple::validate_size<AasClothCylinder, 0x70>(), "AasClothCylinder has the wrong size");
static_assert(temple::validate_size<AasSubmesh, 0x48>(), "AasSubmesh has the wrong size");
static_assert(temple::validate_size<AasAnimation, 0x12C>(), "AasAnimation has the wrong size");
static_assert(temple::validate_size<AasClothStuff1, 0x1C>(), "AasClothStuff1 has the wrong size");
static_assert(temple::validate_size<AasClothStuff, 0x38>(), "AasClothStuff has the wrong size");

#pragma pack(pop)

static class AasHooks : public TempleFix {
public:

	static constexpr int AAS_ERROR = 1;
	static constexpr int AAS_OK = 0;

	// 5000 anims max
	static constexpr size_t sMaxAnims = 5000;
	AasAnimation* mAnims;

	bool IsValidHandle(AasHandle handle) const {
		return handle < sMaxAnims && !mAnims[handle].freed;
	}

	void apply() override {
		static auto instance = this;

		mAnims = temple::GetPointer<AasAnimation>(0x10EFB900);

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
				auto &aasAnim = hooks.mAnims[handle];
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
			auto &aasAnim = hooks.mAnims[handle];
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

	}
} hooks;

