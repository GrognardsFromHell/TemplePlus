#pragma once

#include <EASTL/hash_map.h>

#include <string>
#include <infrastructure/stringutil.h>

#include "spec.h"

class TabFileRecord;

namespace gfx {
	class MdfMaterialFactory;
}

namespace eastl {
	template<>
	struct hash<std::string> {
		size_t operator()(const std::string& str) const {
			return std::hash<std::string>()(str);
		}
	};
}

namespace particles {

	class PartSysParser {
	public:

		typedef eastl::hash_map<std::string, PartSysSpecPtr> SpecMap;

		void ParseFile(const std::string& filename);
		void ParseString(const std::string& spec);

		PartSysSpecPtr GetSpec(const std::string& name) const {
			auto it = mSpecs.find(tolower(name));
			if (it != mSpecs.end()) {
				return it->second;
			}
			return PartSysSpecPtr();
		}

		SpecMap::const_iterator begin() const {
			return mSpecs.cbegin();
		}

		SpecMap::const_iterator end() const {
			return mSpecs.cend();
		}

	private:
		SpecMap mSpecs;

		// To speed up loading
		std::unordered_map<std::string, std::string> mTextureNameCache;
		
		void ParseLifespan(const TabFileRecord& record, PartSysEmitterSpecPtr emitter);
		void ParseParticleLifespan(const TabFileRecord& record, PartSysEmitterSpecPtr emitter);
		void ParseParticleRate(const TabFileRecord& record, PartSysEmitterSpecPtr emitter);
		void ParseEmitterNodeName(const TabFileRecord& record, PartSysEmitterSpecPtr emitter);
		void ParseMaterial(const TabFileRecord& record, PartSysEmitterSpecPtr emitter);
		void ParseMesh(const TabFileRecord& record, PartSysEmitterSpecPtr emitter);
		void ParseEmitter(const TabFileRecord& record);
	};

}
