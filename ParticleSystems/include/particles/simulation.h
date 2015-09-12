
#pragma once

class PartSysEmitter;

namespace particles {
	
	void SimulateParticleAging(PartSysEmitter *emitter, float timeToSimulateSec);

	void SimulateParticleSpawn(PartSysEmitter *emitter, int particleIdx, float timeToSimulateSec);

	void SimulateParticleMovement(PartSysEmitter *emitter, float timeToSimulateSec);

}
