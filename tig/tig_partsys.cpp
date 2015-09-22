#include "stdafx.h"

#include "tig_partsys.h"
#include "hashtable.h"

namespace legacypartsys {

	typedef ToEEHashtable<PartSysSpec> PartSysTable;
	ToEEHashtableSystem<PartSysSpec> partSysTableSys;

	void DumpPartSysProps() {

		typedef int(__cdecl *partsys_param_parse_keyframe_countmaybe)(char *paramValue, float lifespan, int *countOut);
		typedef int(__cdecl *partsys_param_parse_adjust_frameStartfn)(char *paramValue, float *sthOut, float lifespan);
		typedef int(__cdecl *partsys_param_parse_keyframe)(char *paramValue, int a3, PartSysParamKeyframe **pOut, float lifespan);
		typedef char *(__cdecl *partsys_param_parse_get_next_frame)(char *a1);

		auto countKf = (partsys_param_parse_keyframe_countmaybe)0x101F8260;
		auto parseKfV = (partsys_param_parse_adjust_frameStartfn)0x101F7D90;
		auto parseKf = (partsys_param_parse_keyframe)0x101F8500;
		auto nextKf = (partsys_param_parse_get_next_frame)0x101F7E20;

		char *paramVal = "255(2),255(3),197";
		float lifetime = 0.5f;
		int frameCount, result;
		result = countKf(paramVal, lifetime, &frameCount);
		std::vector<float> starts(frameCount), values(frameCount);
		starts[0] = 0;

		if (sscanf(paramVal, "%f", &values[0]) != 1) {
			throw new TempleException("ERROR");
		}

		float start = 1.0f;
		if (parseKfV(paramVal, &start, lifetime)) {
			throw new TempleException("ERROR");
		}

		char *fd = paramVal;
		if (start != 0) {
			fd = nextKf(fd);
		}
		
		float timePerFrame = lifetime / (long double)(frameCount - 1);
		int frameIdx = 1;
		float curStart = 0.0f;

		while (1)
		{
			curStart += timePerFrame;
			starts[frameIdx] = curStart;
			if (parseKfV(fd, &starts[frameIdx], lifetime))
				throw new TempleException("BLAHFASEL");
			values[frameIdx] = values[frameIdx - 1];
			if (*fd)
			{
				if (sscanf(fd, "%f", &values[frameIdx]) != 1)
					throw new TempleException("BLAHFASEL");
			}
			fd = nextKf(fd);
			++frameIdx;
			if (frameIdx >= frameCount)
			{
				break;
			}
		}

		PartSysParamKeyframe *kf;
		result = parseKf("254(2),255(3),197", 0, &kf, 0.5f);
		for (int i = 0; i < kf->frameCount; ++i) {
			float start = kf->startTimes[i];
			float value = kf->values[i];
			printf("\n");
		}
		printf("\n");
		
		/*
		float startOut = 1.0f;
		parseKf("255(2),255(3),197", &startOut, 1.0f);

		startOut = 0;
		parseKf("255(2),255(3),197", &startOut, 1.0f);

		startOut = 0;
		parseKf("255(3),197", &startOut, 1.0f);*/

		auto partSysTable = *(PartSysTable**)0x10EEEA04;

		auto countSystems = partSysTableSys.HashtableNumItems(partSysTable);
		logger->info("Particle Systems: {}", countSystems);

		auto fh = fopen("partsysdump.txt", "wt");
		auto fh2 = fopen("keyframedump.txt", "wt");

		for (size_t i = 0; i < countSystems; ++i) {
			auto system = partSysTableSys.HashtableGetDataPtr(partSysTable, i);

			logger->trace("{}", system->name);
			for (int j = 0; j < system->emitterCount; ++j) {
				auto emitter = system->emitters[j];

				for (int k = 0; k <= 0x2C; ++k) {
					auto param = emitter->params[k];
					if (param && param->type == PSPT_KEYFRAMES) {
						auto keyframes = (PartSysParamKeyframe*)param;
						std::string niceDump;
						std::string dump;
						for (int l = 0; l < keyframes->frameCount; ++l) {
							if (!dump.empty()) {
								dump.push_back('|');
							}
							// Last frame has no delta
							float delta = (l + 1 < keyframes->frameCount) ? keyframes->deltaPerTick[l] : 0;
							dump += fmt::format("{:X};{:X};{:X}", *(uint32_t*)&keyframes->startTimes[l], *(uint32_t*)&keyframes->values[l], *(uint32_t*)&delta);

							if (!niceDump.empty()) {
								niceDump.append(" -> ");
							}
							
							niceDump += fmt::format("{}@{}", keyframes->values[l], keyframes->startTimes[l]);
						}

						fprintf(fh2, "%s|%d|%d: %s\n", system->name, j, k, niceDump.c_str());
						fprintf(fh2, "%s|%d|%d|%d|%s\n", system->name, j, k, keyframes->frameCount, dump.c_str());
					}				
				}

				fprintf(fh, "%s|%d|%d|%f|%f\n", system->name, j, emitter->maxActiveParticles, emitter->particlesPerSec, emitter->particlesPerSecSecondary);				
				fprintf(fh2, "\n");
			}
		}

		fclose(fh);
		fclose(fh2);

	}

}
