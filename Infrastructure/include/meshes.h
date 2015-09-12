
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Mesh {
public:
	Mesh(const std::string& name, bool valid)
		: mName(name),
		  mValid(valid) {
	}

	bool IsValid() const {
		return mValid;
	}
	
	std::string GetName() const {
		return mName;
	}
	
	int GetLegacyId() const {
		return mLegacyId;
	}

	void SetLegacyId(int legacyId) {
		mLegacyId = legacyId;
	}

private:
	const std::string mName;
	const bool mValid;
	int mLegacyId = -1;
};

using MeshRef = std::shared_ptr<Mesh>;

class MeshesManager {
public:	

	MeshRef Resolve(int meshId);

	MeshRef Resolve(const std::string &name);

	void LoadMapping(const std::string &filename);

private:

	std::unordered_map<std::string, MeshRef> mMeshesByName;

	// This is the mapping loaded from meshes.mes
	std::unordered_map<int, std::string> mMapping;

};

extern std::unique_ptr<MeshesManager> meshes;
