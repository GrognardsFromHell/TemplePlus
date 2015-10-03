
#include <infrastructure/tabparser.h>
#include <infrastructure/logging.h>
#include <infrastructure/materials.h>

#include "particles/parser.h"
#include "particles/params.h"
#include "particles/parser_params.h"

namespace particles {

	static const std::map<std::string, PartSysCoordSys> CoordSysMapping = {
		{"Cartesian", PartSysCoordSys::Cartesian},
		{"Polar", PartSysCoordSys::Polar}
	};

	static const std::map<std::string, PartSysEmitterSpace> EmitterSpaceMapping = {
		{"World", PartSysEmitterSpace::World},
		{"Object Pos", PartSysEmitterSpace::ObjectPos},
		{"Object YPR", PartSysEmitterSpace::ObjectYpr},
		{"Node Pos", PartSysEmitterSpace::NodePos},
		{"Node YPR", PartSysEmitterSpace::NodeYpr},
		{"Bones", PartSysEmitterSpace::Bones}
	};

	static const std::map<std::string, PartSysParticleType> ParticleTypeMapping = {
		{"Point", PartSysParticleType::Point},
		{"Sprite", PartSysParticleType::Sprite},
		{"Disc", PartSysParticleType::Disc},
		{"Billboard", PartSysParticleType::Billboard},
		{"Model", PartSysParticleType::Model}
	};

	static const std::map<std::string, PartSysBlendMode> BlendModeMapping = {
		{"Add", PartSysBlendMode::Add},
		{"Blend", PartSysBlendMode::Blend},
		{"Multiply", PartSysBlendMode::Multiply},
		{"Subtract", PartSysBlendMode::Subtract}
	};

	static const std::map<std::string, PartSysParticleSpace> ParticleSpaceMapping = {
		{"World", PartSysParticleSpace::World},
		{"Emitter YPR", PartSysParticleSpace::EmitterYpr},
		{"Same as Emitter", PartSysParticleSpace::SameAsEmitter}
	};

	enum PartSysColumns : uint32_t {
		COL_PARTSYS_NAME = 0,
		COL_EMITTER_NAME,
		COL_DELAY,
		COL_EmitType,
		COL_LIFESPAN,
		COL_PARTICLE_RATE,
		COL_BoundingRadius,
		COL_EMITTER_SPACE,
		COL_EMITTER_NODE_NAME,
		COL_EMITTER_COORD_SYS,
		COL_EMITTER_OFFSET_COORD_SYS,
		COL_PARTICLE_TYPE,
		COL_PARTICLE_SPACE,
		COL_PARTICLE_POS_COORD_SYS,
		COL_PARTICLE_VELOCITY_COORD_SYS,
		COL_MATERIAL,
		COL_PART_LIFESPAN,
		COL_BLEND_MODE,
		COL_Bounce,
		COL_AnimSpeed,
		COL_MODEL,
		COL_Animation,
		COL_BB_LEFT = 67,
		COL_BB_TOP = 68,
		COL_BB_RIGHT = 69,
		COL_BB_BOTTOM = 70,
		COL_PARTICLE_RATE_SECONDARY = 71
	};

	static void ParseOptionalFloat(const TabFileRecord& record, int col, const char* name, const std::function<void(float)>& setter) {
		float value;
		auto column = record[col];
		if (column) {
			if (column.TryGetFloat(value)) {
				setter(value);
			} else {
				logger->warn("Emitter on line {} has invalid {}: '{}'",
				             record.GetLineNumber(), name, column);
			}
		}
	}

	template <typename T>
	static void ParseOptionalEnum(const TabFileRecord& record,
	                              int col,
	                              const char* name,
	                              const std::map<std::string, T>& mapping,
	                              const std::function<void(T)>& setter) {
		T value;
		auto column = record[col];
		if (column) {
			if (column.TryGetEnum(mapping, value)) {
				setter(value);
			} else {
				logger->warn("Emitter on line {} has invalid {}: '{}'",
				             record.GetLineNumber(), name, column);
			}
		}
	}

	void PartSysParser::ParseLifespan(const TabFileRecord& record, PartSysEmitterSpecPtr emitter) {
		auto colLifespan = record[COL_LIFESPAN];
		float lifespan;
		if (!colLifespan || colLifespan.EqualsIgnoreCase("perm")) {
			emitter->SetPermanent(true);
		} else if (colLifespan.TryGetFloat(lifespan)) {
			if (lifespan == 0) {
				emitter->SetInstant(true);
			}
			emitter->SetLifespan(lifespan / 30.0f);
		} else {
			logger->warn("Emitter on line {} has invalid lifespan: '{}'",
			             record.GetLineNumber(), colLifespan);
		}
	}

	void PartSysParser::ParseParticleLifespan(const TabFileRecord& record, PartSysEmitterSpecPtr emitter) {
		auto colLifespan = record[COL_PART_LIFESPAN];
		float lifespan;
		if (!colLifespan || colLifespan.EqualsIgnoreCase("perm")) {
			emitter->SetPermanentParticles(true);
			emitter->SetParticleLifespan(1.0f);
		} else if (colLifespan.TryGetFloat(lifespan)) {
			emitter->SetParticleLifespan(lifespan / 30.0f);
		} else {
			logger->warn("Emitter on line {} has invalid particle lifespan: '{}'",
			             record.GetLineNumber(), colLifespan);
		}
	}

	void PartSysParser::ParseParticleRate(const TabFileRecord& record, PartSysEmitterSpecPtr emitter) {

		auto rate = emitter->GetParticleRate();
		auto maxParticles = emitter->GetMaxParticles();
		if (!record[COL_PARTICLE_RATE].TryGetFloat(rate) || rate == 0) {
			logger->warn("Emitter on line {} has invalid particle rate: '{}'",
			             record.GetLineNumber(), record[COL_PARTICLE_RATE]);
		} else if (emitter->IsPermanent()) {
			// For a permanent emitter, the max. number of particles is how many spawn per time unit multiplied 
			// by how many time units they will exist
			maxParticles = static_cast<int>(emitter->GetParticleLifespan() * rate) + 1;
		} else if (emitter->IsInstant() || emitter->IsPermanentParticles()) {
			maxParticles = static_cast<int>(rate) + 1;
		} else {
			// If the emitter lifespan limits the number of particles more, use that to calculate the max. particles
			auto lifespan = std::min<float>(emitter->GetLifespan(), emitter->GetParticleLifespan());
			maxParticles = static_cast<int>(rate * lifespan) + 1;
		}
		emitter->SetMaxParticles(maxParticles);
		emitter->SetParticleRate(rate);
		emitter->SetParticleRateSecondary(rate);

		// Not sure what this is for at the moment
		float rateSecondary;
		if (record[COL_PARTICLE_RATE_SECONDARY].TryGetFloat(rateSecondary)) {
			emitter->SetParticleRateSecondary(rateSecondary);
		} else {
			logger->warn("Emitter on line {} has invalid secondary particle rate: '{}'",
			             record.GetLineNumber(), record[COL_PARTICLE_RATE_SECONDARY]);
		}
	}

	void PartSysParser::ParseEmitterNodeName(const TabFileRecord& record, PartSysEmitterSpecPtr emitter) {
		auto col = record[COL_EMITTER_NODE_NAME];
		if (col) {
			emitter->SetNodeName(col.AsString());
		}
	}

	void PartSysParser::ParseMaterial(const TabFileRecord& record, PartSysEmitterSpecPtr emitter) {
		auto colMaterial = record[COL_MATERIAL];
		if (colMaterial) {
			auto material = gfx::gMdfMaterialFactory->LoadMaterial(colMaterial.AsString());
			if (!material->IsValid()) {
				logger->warn("Emitter on line {} has invalid material: '{}'",
				             record.GetLineNumber(), colMaterial);
			}
			emitter->SetMaterial(material);
		}
	}

	void PartSysParser::ParseMesh(const TabFileRecord& record, PartSysEmitterSpecPtr emitter) {

		// This only applies to emitters that emit 3D particles
		if (emitter->GetParticleType() != PartSysParticleType::Model) {
			return;
		}

		// The model filename is usually just the filename without path + extension
		auto meshRef = mMeshes->Resolve(record[COL_MODEL].AsString());

		if (!meshRef->IsValid()) {
			logger->warn("Emitter on line {} has invalid mesh: '{}'",
			             record.GetLineNumber(), record[COL_MODEL]);
		}

		emitter->SetMesh(meshRef);

	}

	void PartSysParser::ParseEmitter(const TabFileRecord& record) {
		auto systemName = record[COL_PARTSYS_NAME].AsString();

		auto& system = mSpecs[tolower(systemName)];
		// Create it on demand
		if (!system) {
			system = std::make_shared<PartSysSpec>(systemName);
		}

		// Add the emitter
		auto emitter = system->CreateEmitter(record[COL_EMITTER_NAME].AsString());

		ParseOptionalFloat(record, COL_DELAY, "Delay", [&] (float value) {
			                   emitter->SetDelay(value);
		                   });

		ParseLifespan(record, emitter);

		ParseParticleLifespan(record, emitter);

		ParseParticleRate(record, emitter);

		ParseOptionalEnum<PartSysEmitterSpace>(record, COL_EMITTER_SPACE, "emitter space", EmitterSpaceMapping, [&](auto space) {
			                                       emitter->SetSpace(space);
		                                       });

		ParseEmitterNodeName(record, emitter);

		ParseOptionalEnum<PartSysCoordSys>(record, COL_EMITTER_COORD_SYS, "emitter coord sys", CoordSysMapping, [&](auto coordSys) {
			                                   emitter->SetCoordSys(coordSys);
		                                   });

		ParseOptionalEnum<PartSysCoordSys>(record, COL_EMITTER_OFFSET_COORD_SYS, "emitter offset coord sys", CoordSysMapping, [&](auto coordSys) {
			                                   emitter->SetOffsetCoordSys(coordSys);
		                                   });

		ParseOptionalEnum<PartSysParticleType>(record, COL_PARTICLE_TYPE, "particle type", ParticleTypeMapping, [&](PartSysParticleType type) {
			                                       emitter->SetParticleType(type);
		                                       });

		ParseOptionalEnum<PartSysBlendMode>(record, COL_BLEND_MODE, "blend mode", BlendModeMapping, [&](auto mode) {
			                                    emitter->SetBlendMode(mode);
		                                    });

		ParseMaterial(record, emitter);

		ParseOptionalEnum<PartSysCoordSys>(record, COL_PARTICLE_POS_COORD_SYS, "particle pos coord sys", CoordSysMapping, [&](auto coordSys) {
			                                   emitter->SetParticlePosCoordSys(coordSys);
		                                   });
		ParseOptionalEnum<PartSysCoordSys>(record, COL_PARTICLE_VELOCITY_COORD_SYS, "particle velocity coord sys", CoordSysMapping, [&](auto coordSys) {
			                                   emitter->SetParticleVelocityCoordSys(coordSys);
		                                   });
		ParseOptionalEnum<PartSysParticleSpace>(record, COL_PARTICLE_SPACE, "particle space", ParticleSpaceMapping, [&](auto space) {
			                                        emitter->SetParticleSpace(space);
		                                        });

		ParseMesh(record, emitter);

		// Parse the bounding box
		ParseOptionalFloat(record, COL_BB_LEFT, "bb left", [&](float val) {
			                   emitter->SetBoxLeft(val);
		                   });
		ParseOptionalFloat(record, COL_BB_TOP, "bb top", [&](float val) {
			                   emitter->SetBoxTop(val);
		                   });
		ParseOptionalFloat(record, COL_BB_RIGHT, "bb right", [&](float val) {
			                   emitter->SetBoxRight(val);
		                   });
		ParseOptionalFloat(record, COL_BB_BOTTOM, "bb bottom", [&](float val) {
			                   emitter->SetBoxBottom(val);
		                   });

		for (int paramId = 0; paramId <= part_attractorBlend; paramId++) {
			int colIdx = 22 + paramId;
			auto col = record[colIdx];
			if (col) {
				bool success;
				std::unique_ptr<PartSysParam> param(ParserParams::Parse((PartSysParamId) paramId,
				                                                        col.AsString(),
				                                                        emitter->GetLifespan(),
				                                                        emitter->GetParticleLifespan(),
				                                                        success));
				if (success) {
					emitter->SetParam((PartSysParamId)paramId, param);
				} else {
					logger->warn("Unable to parse particle system param {} for particle system {} and emitter {} with value {}",
					             paramId, systemName, emitter->GetName(), col.AsString());
				}
			}
		}

	}

	PartSysParser::PartSysParser(gfx::MeshesManagerPtr meshes) : mMeshes(meshes) {
	}

	void PartSysParser::ParseFile(const std::string& filename) {
		TabFile::ParseFile(filename, [this](const TabFileRecord& record) {
			                   ParseEmitter(record);
		                   });
	}

	void PartSysParser::ParseString(const std::string& spec) {
		TabFile::ParseString(spec, [this](const TabFileRecord& record) {
			                     ParseEmitter(record);
		                     });
	}

}
