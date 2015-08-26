
#include "particles/spec.h"
#include "elfhash.h"

PartSysSpec::PartSysSpec(const std::string & name) : mName(name), mNameHash(ElfHash::Hash(mName))
{
}

PartSysEmitterSpecPtr PartSysSpec::CreateEmitter(const std::string & name)
{
	auto emitter = std::make_shared<PartSysEmitterSpec>(*this, name);
	mEmitters.push_back(emitter);
	return emitter;
}
