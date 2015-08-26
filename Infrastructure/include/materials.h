
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Material {
public:
	Material(const std::string &name, bool valid) : mName(name), mValid(valid) {
	}

	std::string GetName() const {
		return mName;
	}
	
	bool IsValid() const {
		return mValid;
	}
	
private:
	Material(Material&) = delete;
	Material& operator=(Material&) = delete;

	const std::string mName;
	const bool mValid;
};

typedef std::shared_ptr<Material> MaterialRef;

class MaterialManager {
public:
	MaterialManager();
	~MaterialManager();

	MaterialRef Resolve(const std::string& materialName);
private:
	std::unordered_map<std::string, MaterialRef> mMaterials;
};

extern std::unique_ptr<MaterialManager> materials;
