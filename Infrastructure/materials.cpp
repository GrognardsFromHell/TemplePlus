
#include "materials.h"
#include "stringutil.h"

std::unique_ptr<MaterialManager> materials;

MaterialManager::MaterialManager() {
}

MaterialManager::~MaterialManager() {
}

MaterialRef MaterialManager::Resolve(const std::string& materialName) {
	auto key = tolower(materialName);
	
	auto it = mMaterials.find(key);
	if (it != mMaterials.end()) {
		return it->second;
	}

	bool valid = true; // TODO: Validate

	auto ref = std::make_shared<Material>(materialName, valid);
	mMaterials[key] = ref;
	return ref;
}
