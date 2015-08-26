
#include "meshes.h"
#include "mesparser.h"
#include "logging.h"

std::unique_ptr<MeshesManager> meshes;

MeshRef MeshesManager::Resolve(const std::string& name) {

	return MeshRef(); // Nullptr
}

void MeshesManager::LoadMapping(const std::string& filename) {
	auto meshesMapping = MesFile::ParseFile(filename);
	mMapping.insert(meshesMapping.begin(), meshesMapping.end());
	logger->debug("Loaded mapping for {} meshes from {}", meshesMapping.size(), filename);
}
