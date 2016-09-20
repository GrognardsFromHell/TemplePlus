
#include "stdafx.h"

#include "common.h"
#include <temple/dll.h>
#include "description.h"
#include "util/fixes.h"

LegacyDescriptionSystem description;

class DescriptionHooks : public TempleFix{
	void apply() override {
		replaceFunction<const char*(int)>(0x100869D0, [](int descrIdx)->const char*	{
			return description.GetDescriptionString(descrIdx);
		});
	}
} descHooks;

const char* LegacyDescriptionSystem::getDisplayName(objHndl obj)
{
	if (obj)
		return _getDisplayName(obj, obj);
	else
		return "OBJ_HANDLE_NULL";
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

	MesLine line(descrIdx);
	if (mesFuncs.GetLine(descrMesExt, &line))
		return line.value;
	if (!mesFuncs.GetLine(mesHandle, &line)) {
		return nullptr;
	}
	return line.value;
}

BOOL LegacyDescriptionSystem::Init(GameSystemConf& conf){
	if (!mesFuncs.Open("mes\\description.mes", &descriptionMes)
		|| mesFuncs.Open("mes\\description_ext.mes", &descrMesExt)
		|| !mesFuncs.Open("mes\\long_description.mes", &longDescrMes))
		return FALSE;
	
	// get the highest description index
	auto numLines = mesFuncs.GetNumLines(descriptionMes);
	if (!numLines){
		*descrIdxMax = 0;
	} 
	else {
		MesLine line;
		mesFuncs.ReadLineDirect(descriptionMes, numLines - 1, &line);
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
