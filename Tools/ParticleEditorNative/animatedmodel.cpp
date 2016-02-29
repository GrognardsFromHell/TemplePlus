#include "api.h"
#include <infrastructure/meshes.h>
#include <temple/meshes.h>
#include <infrastructure/stringutil.h>
#include <infrastructure/exception.h>
#include <graphics/math.h>

#include <iostream>

API gfx::AnimatedModelPtr* AnimatedModel_FromFiles(TempleDll* dll,
                                                   const wchar_t* skmFilename,
                                                   const wchar_t* skaFilename) {

	gfx::AnimatedModelParams params;

	auto model = dll->aasFactory.FromFilenames(ucs2_to_local(skmFilename),
	                                           ucs2_to_local(skaFilename),
	                                           gfx::EncodedAnimId(gfx::WeaponAnim::Idle),
	                                           params);

	auto headMdf(dll->mdfFactory.LoadMaterial("art/meshes/PCs/PC_Human_Male/Head.mdf"));
	auto chestMdf(dll->mdfFactory.LoadMaterial("art/meshes/PCs/PC_Human_Male/Chest.mdf"));
	auto glovesMdf(dll->mdfFactory.LoadMaterial("art/meshes/PCs/PC_Human_Male/Hands.mdf"));
	auto bootsMdf(dll->mdfFactory.LoadMaterial("art/meshes/PCs/PC_Human_Male/Feet.mdf"));

	model->AddReplacementMaterial(gfx::MaterialPlaceholderSlot::HEAD, headMdf);
	model->AddReplacementMaterial(gfx::MaterialPlaceholderSlot::CHEST, chestMdf);
	model->AddReplacementMaterial(gfx::MaterialPlaceholderSlot::GLOVES, glovesMdf);
	model->AddReplacementMaterial(gfx::MaterialPlaceholderSlot::BOOTS, bootsMdf);

	dll->currentModel = model;

	return new gfx::AnimatedModelPtr(model);

}

API void AnimatedModel_Free(gfx::AnimatedModelPtr* handle) {
	delete handle;
}

API void AnimatedModel_Render(TempleDll* dll, gfx::AnimatedModelPtr* handle, float w, float h, float scale) {

	auto &camera = dll->renderingDevice.GetCamera();
	camera.SetScale(scale);
	camera.SetScreenWidth(w, h);
	camera.CenterOn(0, 0, 0);

	auto model = (*handle).get();

	gfx::Light3d light;
	light.type = gfx::Light3dType::Directional;
	light.color = XMFLOAT4(1, 1, 1, 1);
	light.dir = XMFLOAT4(-0.7070000171661377f, -0.8659999966621399f, 0, 0);

	std::vector<gfx::Light3d> lights;
	lights.push_back(light);

	gfx::MdfRenderOverrides overrides;
	//overrides.ignoreLighting = true;
	
	dll->aasRenderer.Render(handle->get(), dll->animParams, lights, &overrides);

}

API void AnimatedModel_AdvanceTime(gfx::AnimatedModelPtr* handle, float time) {

	auto model = handle->get();

	gfx::AnimatedModelParams params;
	model->Advance(time, 0, 0, params);

}
