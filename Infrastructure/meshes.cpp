
#include "meshes.h"
#include "mesparser.h"
#include "logging.h"

std::unique_ptr<MeshesManager> meshes;

MeshRef MeshesManager::Resolve(int meshId) {
	auto it = mMapping.find(meshId);
	if (it == mMapping.end()) {
		// return an invalid mesh
		return std::make_shared<Mesh>(fmt::format("<invalid:{}>", meshId), false);
	}

	auto result = std::make_shared<Mesh>(it->second, true);
	result->SetLegacyId(meshId);
	return result;
}

MeshRef MeshesManager::Resolve(const std::string& name) {

	// paths that contain backslashes are considered "absolute"
	auto isFilename = (name.find('\\') == std::string::npos);

	// find the native key
	auto legacyId = -1;
	for (const std::pair<int, std::string> &entry : mMapping) {		
		if (!isFilename) {
			if (!_stricmp(entry.second.c_str(), name.c_str())) {
				legacyId = entry.first;
				break;
			}
		} else {
			const char *path = entry.second.c_str();
			// Skip everything before the filename
			int lastSlashPos = entry.second.rfind('\\');
			if (lastSlashPos != std::string::npos) {
				path += lastSlashPos + 1;
			}

			if (!_stricmp(path, name.c_str())) {
				legacyId = entry.first;
				break;
			}
		}
	}
	
	return Resolve(legacyId);
}

void MeshesManager::LoadMapping(const std::string& filename) {
	auto meshesMapping = MesFile::ParseFile(filename);
	mMapping.insert(meshesMapping.begin(), meshesMapping.end());
	logger->debug("Loaded mapping for {} meshes from {}", meshesMapping.size(), filename);
}
