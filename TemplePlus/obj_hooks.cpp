
#include "stdafx.h"
#include "util/fixes.h"
#include "obj.h"
#include "critter.h"
#include <infrastructure/meshes.h>

static class ObjHooks : public TempleFix {
public:
	const char *name() {
		return "Object System Overrides";
	}
	void apply() override;

	static void UpdateModelEquipment(objHndl obj);
	static int GetAasHandle(objHndl obj);
	static int GetIdleAnim(objHndl obj);

} hooks;

void ObjHooks::apply()
{
	replaceFunction(0x1007E9D0, UpdateModelEquipment);
	replaceFunction(0x10021A40, GetAasHandle);
	replaceFunction(0x100167F0, GetIdleAnim);
}

void ObjHooks::UpdateModelEquipment(objHndl handle)
{
	critterSys.UpdateModelEquipment(handle);
}

int ObjHooks::GetAasHandle(objHndl obj)
{
	auto model(objects.GetAnimHandle(obj));
	if (model) {
		return model->GetHandle();
	}
	return 0;
}

int ObjHooks::GetIdleAnim(objHndl obj)
{
	return objects.GetIdleAnim(obj);
}
