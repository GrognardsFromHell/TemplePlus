
#pragma once

namespace particles {

	class PartSysEmitter;
	
	void SimulateParticleAging(PartSysEmitter *emitter, float timeToSimulateSec);

	void SimulateParticleSpawn(PartSysEmitter *emitter, int particleIdx, float timeToSimulateSec);

	void SimulateParticleMovement(PartSysEmitter *emitter, float timeToSimulateSec);

}
