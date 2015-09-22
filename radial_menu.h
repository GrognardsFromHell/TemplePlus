#include <temple/dll.h>
//#include "obj.h"
#include "common.h"

// use radialmenu.h instead of this weaksauce

struct RadialMenuStruct
{
	void * field0;
	uint32_t field4;
	uint32_t field8;
	uint32_t fieldC;
	uint32_t field10;
	uint32_t field14;
	uint32_t field18;
	uint32_t field1C;
	int32_t field20;
	uint32_t field24;
	uint32_t field28;
	uint16_t field2C;
	uint16_t field2E;
	uint8_t field30;
	uint8_t field31;
	uint8_t field32;
	uint8_t field33;
	
	uint32_t field34;
	void * field38;
	uint32_t field3C;
	uint32_t field40;
	uint32_t field44;

};


void RadialMenuStructInit(RadialMenuStruct * radmenu);


struct RadialFuncs : temple::AddressTable
{
	uint32_t(__cdecl * D20ASthg_sub_100F0110)(objHndl objHnd, void *);
	uint32_t(__cdecl * RadialMenuArgMap_sub_100F12B0)(uint32_t);
	uint32_t(__cdecl * RadialMenuCreateEntry)(objHndl objHnd, RadialMenuStruct* radmenu, uint32_t);
	void(__cdecl * sub_100F0A70)(objHndl objHnd);
	RadialFuncs()
	{
		rebase(D20ASthg_sub_100F0110, 0x100F0110);
		rebase(RadialMenuArgMap_sub_100F12B0, 0x100F12B0);
		rebase(RadialMenuCreateEntry, 0x100F0670);
		rebase(sub_100F0A70, 0x100F0A70);
	}
};




// const auto TestSizeOfRadialMenuStruct = sizeof(RadialMenuStruct); // shoud be 72

extern RadialFuncs radialFuncs;


void _RadialMenuUpdate(objHndl objHnd);