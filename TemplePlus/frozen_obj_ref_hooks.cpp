
#include "stdafx.h"
#include "util/fixes.h"
#include "frozen_obj_ref.h"

static class FrozenObjRefHooks : public TempleFix {
public:

	virtual void apply() override {
		
		// map_obj_ref_write
		replaceFunction<int(objHndl *, FrozenObjRef *, void *)>(0x10020280, [](objHndl *handle, FrozenObjRef *ref, void* file) {
			return FrozenObjRef::Save(*handle, ref, file) ? 1 : 0;
		});

		// map_obj_ref_load
		replaceFunction<int(objHndl *, FrozenObjRef*, void *)>(0x10020370, [](objHndl *handleOut, FrozenObjRef *ref, void *file) {
			return FrozenObjRef::Load(handleOut, ref, file) ? 1 : 0;
		});

		// map_obj_ref_create
		replaceFunction<void(objHndl, FrozenObjRef *)>(0x10020540, [](objHndl handle, FrozenObjRef *refOut) {
			*refOut = FrozenObjRef::Freeze(handle);
		});

		// map_obj_ref_unfreeze
		replaceFunction<int(objHndl*, FrozenObjRef *)>(0x10020610, [](objHndl *handleOut, FrozenObjRef *ref) {
			return FrozenObjRef::Unfreeze(*ref, handleOut) ? 1 : 0;
		});

	}

} hooks;
