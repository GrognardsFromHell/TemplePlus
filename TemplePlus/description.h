#pragma once

#include "common.h"
#include "tig/tig_mes.h"
#include <map>


struct GameSystemConf;

struct LegacyDescriptionSystem : temple::AddressTable
{
//	MesHandle * descriptionMes;
	MesHandle *gameDescMes;
	uint32_t * customNamesCount;
	int *descrIdxMax; // the highest description.mes index (not to be confused with the number of description entries!!!)
	char*** customNamesArray;
	const char* getDisplayName(objHndl obj);
	const char* debugGetName(uint64_t handle);
	const char* getDisplayName(objHndl obj, objHndl observer);
	const char* GetDescriptionString(int descrIdx) const; // extends the description strings with Temple+ file description_ext.mes

	uint32_t(__cdecl *DescriptionIsCustom)(int32_t descrIdx);
	uint32_t(__cdecl *CustomNameNew)(const char *pString);
	void(__cdecl *CustomNameChange)(const char * pNewNameSource, uint32_t descrIdx);
	const char *(__cdecl *_getDisplayName)(objHndl obj, objHndl observer);
	BOOL LongDescriptionHas(objHndl handle);
	const char* GetLongDescription(objHndl handle, objHndl observer = objHndl::null);

	BOOL Init(const GameSystemConf& conf);
	void Exit();
	void Reset();



	LegacyDescriptionSystem()
	{
		rebase(customNamesCount, 0x10AB757C);
		rebase(customNamesArray, 0x10AB7578);
		rebase(descrIdxMax, 0x10AB7564);
	//	rebase(descriptionMes, 0x10BD023C);
		rebase(gameDescMes, 0x10AB7574);

		rebase(_getDisplayName, 0x1001FA80);
		rebase(DescriptionIsCustom, 0x100869B0);
		rebase(CustomNameNew, 0x10086A50);
		rebase(CustomNameChange, 0x10086AA0);
	};

protected:
	MesHandle descriptionMes =0;
	MesHandle longDescrMes =0;
	MesHandle descrMesExt = 0;
	static std::map<int, std::string> descrOverrides;
	static std::map<int, std::string> longDescrOverrides;
	const char *GetLongDescriptionFromFile(int idx);
};

extern LegacyDescriptionSystem description;
