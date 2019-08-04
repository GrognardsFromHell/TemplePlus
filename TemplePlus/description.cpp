
#include "stdafx.h"

#include "common.h"
#include <temple/dll.h>
#include "description.h"
#include "util/fixes.h"
#include "gamesystems/objects/objsystem.h"
#include <map>

LegacyDescriptionSystem description;
std::map<int, std::string> LegacyDescriptionSystem::descrOverrides;
std::map<int, std::string> LegacyDescriptionSystem::longDescrOverrides;


class DescriptionHooks : public TempleFix{
	void apply() override {

		// fix for crafted items not showing long descriptions (the long description will be outdated of course)
		replaceFunction<const char*(objHndl, objHndl)>(0x1001FB60, [](objHndl handle, objHndl observer){
			return description.GetLongDescription(handle, observer);
		});

		// GetDescriptionString
		replaceFunction<const char*(int)>(0x100869D0, [](int descrIdx)->const char*	{
			return description.GetDescriptionString(descrIdx);
		});
	}
} descHooks;

const char* LegacyDescriptionSystem::getDisplayName(objHndl obj)
{
	if (obj){
		auto result = _getDisplayName(obj, obj);
		if (!result)
			return "OBJ_HANDLE_NULL";
		return result;
	}
	else
		return "OBJ_HANDLE_NULL";
}

const char * LegacyDescriptionSystem::debugGetName(uint64_t handle){
	objHndl _hnd;
	_hnd.handle = handle;
	return getDisplayName(_hnd);
}

const char* LegacyDescriptionSystem::getDisplayName(objHndl obj, objHndl observer)
{
	return _getDisplayName(obj, observer);
}

const char* LegacyDescriptionSystem::GetDescriptionString(int descrIdx) const
{
	// check if custom name idx
	if (descrIdx & 0x40000000){
		auto _descrIdx = descrIdx & (~0x40000000);
		if (_descrIdx >= 0	&& _descrIdx  < (int)*customNamesCount)	{
			return (*customNamesArray)[_descrIdx];
		}
		return nullptr;
	}

	if (descrIdx < 0 || descrIdx > *descrIdxMax){
		return nullptr;
	}

	auto mesHandle = descriptionMes;
	if (descrIdx >= 30000 ){
		if (*gameDescMes == -1)
			return nullptr;
		mesHandle = *gameDescMes;
	}

	// look it up in the .mes extensions first
	auto findInExt = descrOverrides.find(descrIdx);
	if (findInExt != descrOverrides.end())
		return findInExt->second.c_str();

	// if not found, look it up in the normal .mes files
	MesLine line(descrIdx);
	if (mesFuncs.GetLine(descrMesExt, &line))
		return line.value;
	if (!mesFuncs.GetLine(mesHandle, &line)) {
		return nullptr;
	}
	return line.value;
}

BOOL LegacyDescriptionSystem::LongDescriptionHas(objHndl handle){

	if (!handle)
		return FALSE;

	auto descr = GetLongDescription(handle, handle);
	if (descr == nullptr)
		return FALSE;

	return TRUE;
}

const char * LegacyDescriptionSystem::GetLongDescription(objHndl handle, objHndl observer){
	if (!handle)
		return nullptr;

	auto obj = objSystem->GetObject(handle);
	if (obj->type == obj_t_key){
		auto keyId = obj->GetInt32(obj_f_key_key_id);
		return temple::GetRef<const char*(__cdecl)(int)>(0x100867E0)(keyId); // gets line from gamekeylog.mes
	}

	if (obj->IsPC()){
		return temple::GetRef<const char*(__cdecl)(objHndl, obj_f)>(0x1009E430)(handle, obj_f_pc_player_name);
	}

	if (obj->IsNPC()){
		if (!observer)
			observer = handle;
		auto descrId = temple::GetRef<int(__cdecl)(objHndl, objHndl)>(0x1007F670)(handle, observer);
		return GetLongDescriptionFromFile(descrId);
	}
	
	if (temple::GetRef<int>(0x10788098) || inventory.IsIdentified(handle)){  // is editor mode or is identified
		auto descrIdx = obj->GetInt32(obj_f_description);

		// check if custom name idx
		if ( (descrIdx & 0x40000000)
			|| descrIdx < 0 || descrIdx > *descrIdxMax) {
			auto protoId = objSystem->GetProtoId(handle);
			auto protoHndl = objSystem->GetProtoHandle(protoId);
			descrIdx = objSystem->GetObject(protoHndl)->GetInt32(obj_f_description);
		}
		return GetLongDescriptionFromFile(descrIdx);
	}

	// unidentified item
	auto descrId = obj->GetInt32(obj_f_item_description_unknown);
	return GetLongDescriptionFromFile(descrId);
}

BOOL LegacyDescriptionSystem::Init(const GameSystemConf& conf){

	if (!mesFuncs.Open("mes\\description.mes", &descriptionMes)
	    || !mesFuncs.Open("mes\\description_ext.mes", &descrMesExt)
		|| !mesFuncs.Open("mes\\long_description.mes", &longDescrMes))
		return FALSE;


	*descrIdxMax = 0;

	{
		TioFileList descrFlist;
		tio_filelist_create(&descrFlist, "mes\\description\\*.mes");

		for (auto i=0; i < descrFlist.count; i++){
			std::string combinedFname(fmt::format("mes\\description\\{}", descrFlist.files[i].name));
			mesFuncs.AddToMap(combinedFname, descrOverrides,descrIdxMax);
		}

		tio_filelist_destroy(&descrFlist);

	}

	{
		TioFileList longdescFlist;
		tio_filelist_create(&longdescFlist, "mes\\long_descr\\*.mes");

		for (auto i = 0; i < longdescFlist.count; i++) {
			std::string combinedFname(fmt::format("mes\\long_descr\\{}", longdescFlist.files[i].name));
			MesHandle mh;
			mesFuncs.Open(combinedFname.c_str(), &mh);
			auto numLines = mesFuncs.GetNumLines(mh);
			for (auto j = 0; j<numLines; j++) {
				MesLine line;
				mesFuncs.ReadLineDirect(mh, j, &line);
				longDescrOverrides[line.key] = line.value;
			}
			mesFuncs.Close(mh);
		}

		tio_filelist_destroy(&longdescFlist);

	}


	
	// get the highest description index
	auto numLines = mesFuncs.GetNumLines(descriptionMes);
	if (!numLines){
		*descrIdxMax = 0;
	} 
	else {
		MesLine line;
		mesFuncs.ReadLineDirect(descriptionMes, numLines - 1, &line);
		if ( static_cast<int>(line.key) > *descrIdxMax)
			*descrIdxMax = line.key;
	}

	*gameDescMes = -1;
	*customNamesArray = nullptr;
	*customNamesCount = 0;

	return TRUE;
}

void LegacyDescriptionSystem::Exit(){
	mesFuncs.Close(descriptionMes);
	mesFuncs.Close(descrMesExt);
	mesFuncs.Close(longDescrMes);
	if (*customNamesCount > 0){
		for (auto i = 0u; i < *customNamesCount; i++){
			free((*customNamesArray)[i]);
		}
		free(*customNamesArray);
	}
}

void LegacyDescriptionSystem::Reset(){
	if (*customNamesCount > 0) {
		for (auto i = 0u; i < *customNamesCount; i++) {
			free((*customNamesArray)[i]);
		}
		free(*customNamesArray);
		*customNamesArray = nullptr;
		*customNamesCount = 0;
	}
}

const char * LegacyDescriptionSystem::GetLongDescriptionFromFile(int idx){
	if (idx < 0 || idx > *descrIdxMax)
		return nullptr;

	auto findInExt = longDescrOverrides.find(idx);
	if (findInExt != longDescrOverrides.end())
		return findInExt->second.c_str();

	MesLine line(idx);
	auto result = mesFuncs.GetLine(longDescrMes, &line);
	return result != 0 ? line.value : nullptr;
}
