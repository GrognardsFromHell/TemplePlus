#include "stdafx.h"

#include "tig_partsys.h"
#include "hashtable.h"

namespace legacypartsys {

	typedef ToEEHashtable<PartSysSpec> PartSysTable;
	ToEEHashtableSystem<PartSysSpec> partSysTableSys;

	void DumpPartSysProps() {

		auto partSysTable = *(PartSysTable**)0x10EEEA04;

		auto countSystems = partSysTableSys.HashtableNumItems(partSysTable);
		logger->info("Particle Systems: {}", countSystems);

		auto fh = fopen("partsysdump.txt", "wt");

		for (size_t i = 0; i < countSystems; ++i) {
			auto system = partSysTableSys.HashtableGetDataPtr(partSysTable, i);

			logger->trace("{}", system->name);
			for (int j = 0; j < system->emitterCount; ++j) {
				auto emitter = system->emitters[j];

				fprintf(fh, "%s|%d|%d|%f|%f\n", system->name, j, emitter->maxActiveParticles, emitter->particlesPerSec, emitter->particlesPerSecSecondary);
			}
		}

		fclose(fh);

	}

}
