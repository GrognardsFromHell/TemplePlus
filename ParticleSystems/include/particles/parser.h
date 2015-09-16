#pragma once

#include <string>
#include <unordered_map>
#include <stringutil.h>

#include "spec.h"

class TabFileRecord;
class gfx::MaterialManager;

namespace particles {

	class PartSysParser {
	public:

		explicit PartSysParser(gfx::MaterialManager& materials);

		typedef std::unordered_map<std::string, PartSysSpecPtr> SpecMap;

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
		gfx::MaterialManager& mMaterials;

		void ParseLifespan(const TabFileRecord& record, PartSysEmitterSpecPtr emitter);
		void ParseParticleLifespan(const TabFileRecord& record, PartSysEmitterSpecPtr emitter);
		void ParseParticleRate(const TabFileRecord& record, PartSysEmitterSpecPtr emitter);
		void ParseEmitterNodeName(const TabFileRecord& record, PartSysEmitterSpecPtr emitter);
		void ParseMaterial(const TabFileRecord& record, PartSysEmitterSpecPtr emitter);
		void ParseMesh(const TabFileRecord& record, PartSysEmitterSpecPtr emitter);
		void ParseEmitter(const TabFileRecord& record);
	};

}
