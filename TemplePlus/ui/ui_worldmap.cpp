#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include "maps.h"

#include "ui_worldmap.h"
#include "ui_systems.h"

#include <temple/dll.h>

//*****************************************************************************
//* Worldmap-UI
//*****************************************************************************

class UiWorldmapImpl {
	friend class UiWorldmap;
	friend class WorldmapFix;
public:
	UiWorldmapImpl();

	std::vector<int> areaToLocId;
};

UiWorldmapImpl *wmImpl = nullptr;

UiWorldmap::UiWorldmap(int width, int height) {
	struct LgcyConf {
		int x;
		int width;
		int height;
	} lgcyConf;
	lgcyConf.width = width;
	lgcyConf.height = height;

	auto startup = temple::GetPointer<int(const LgcyConf*)>(0x10160470);
	if (!startup(&lgcyConf)) {
		throw TempleException("Unable to initialize game system Worldmap-UI");
	}

	mImpl = std::make_unique<UiWorldmapImpl>();
	wmImpl = mImpl.get();
}
UiWorldmap::~UiWorldmap() {
	auto shutdown = temple::GetPointer<void()>(0x1015e060);
	shutdown();
}
void UiWorldmap::ResizeViewport(const UiResizeArgs& resizeArg) {
	auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x101599c0);
	resize(&resizeArg);
}
void UiWorldmap::Reset() {
	auto reset = temple::GetPointer<void()>(0x101597b0);
	reset();
}
bool UiWorldmap::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x101598b0);
	return save(file) == 1;
}
bool UiWorldmap::LoadGame(const UiSaveFile &save) {
	auto load = temple::GetPointer<int(const UiSaveFile*)>(0x1015e0f0);
	return load(&save) == 1;
}
const std::string &UiWorldmap::GetName() const {
	static std::string name("Worldmap-UI");
	return name;
}

void UiWorldmap::Show(int mode)
{
	static auto ui_show_worldmap = temple::GetPointer<void(int mode)>(0x1015f140);
	ui_show_worldmap(mode);
}

void UiWorldmap::TravelToArea(int area)
{
	static auto ui_worldmap_travel_by_dialog = temple::GetPointer<void(int)>(0x10160450);
	ui_worldmap_travel_by_dialog(area);
}

bool UiWorldmap::NeedToClearEncounterMap(){
	static auto uiWorldmapNeedToClearEncounterMap = temple::GetRef<BOOL(__cdecl)()>(0x10159790);
	return (uiWorldmapNeedToClearEncounterMap() != FALSE) ? true: false;
}

// UI crash at exit fix
class UiExitCrashFix : TempleFix
{
public: 
	void apply() override 
	{
		int asdf;
		read(0x1015B3E8 + 2, &asdf, sizeof(int));
		
		write(0x1015B3F5 + 2, &asdf, sizeof(int));// need to free the "centered on party" button; Spellslinger forgot to change this
		//read(0x1015B3F5 + 2, &asdf, sizeof(int));

	}
} uiExitCrashFix;

UiWorldmap &ui_worldmap() {
	return uiSystems->GetWorldmap();
}

void UiWorldmapMakeTripWrapper();

// Worldmap extension by Spellslinger
class WorldmapFix : TempleFix
{
public:
	static int GetAreaFromMap(int mapId);
	static int CanAccessWorldmap();

	void apply() override {
		replaceFunction<void(int)>(0x10160450, [](int destArea) {
			auto &uiWorldmapDestLocId = temple::GetRef<int>(0x102FB3E8);
			//auto &uiWorldmapAreaToLocId_Table = temple::GetRef<int[]>(0x11EA3610); // this is the Co8 location
			
			uiWorldmapDestLocId = wmImpl->areaToLocId[destArea];

			ui_worldmap().Show(2);
		});

		// This fixes an out of bounds write caused by spell slinger hacks
		writeNoops(0x10159ABC);

		//replaceFunction<void()>(0x1015EA20, UiWorldmapMakeTripWrapper);
	}

} worldmapFix;

int WorldmapFix::GetAreaFromMap(int mapId)
{ // this might be co8 specific so not sure if to hook this
	int result;
	switch (mapId)
	{
	case 5000:
	case 5001:
	case 5006:
	case 5007:
	case 5008:
	case 5009:
	case 5010:
	case 5011:
	case 5012:
	case 5013:
	case 5014:
	case 5015:
	case 5016:
	case 5017:
	case 5018:
	case 5019:
	case 5020:
	case 5021:
	case 5022:
	case 5023:
	case 5024:
	case 5025:
	case 5026:
	case 5027:
	case 5028:
	case 5029:
	case 5030:
	case 5031:
	case 5032:
	case 5033:
	case 5034:
	case 5035:
	case 5036:
	case 5037:
	case 5038:
	case 5039:
	case 5040:
	case 5041:
	case 5042:
	case 5043:
	case 5044:
	case 5045:
	case 5046:
	case 5047:
	case 5048:
	case 5049:
	case 5063:
	case 5096:
	case 5097:
	case 5098:
	case 5099:
	case 5100:
	case 5101:
	case 5102:
	case 5103:
	case 5104:
	case 5115:
		result = 1;
		break;
	case 5002:
	case 5003:
	case 5004:
	case 5005:
		result = 2;
		break;
	case 5051:
	case 5052:
	case 5053:
	case 5054:
	case 5055:
	case 5056:
	case 5057:
	case 5058:
	case 5059:
	case 5060:
	case 5061:
	case 5085:
	case 5086:
	case 5087:
	case 5088:
		result = 3;
		break;
	case 5062:
	case 5064:
	case 5065:
	case 5066:
	case 5067:
	case 5078:
	case 5079:
	case 5080:
	case 5081:
	case 5082:
	case 5083:
	case 5084:
	case 5106:
		result = 4;
		break;
	case 5094:
		result = 5;
		break;
	case 5068:
		result = 6;
		break;
	case 5092:
	case 5093:
		result = 7;
		break;
	case 5091:
		result = 8;
		break;
	case 5095:
	case 5114:
		result = 9;
		break;
	case 5069:
		result = 10;
		break;
	case 5112:
		result = 11;
		break;
	case 5113:
		result = 12;
		break;
	case 5121:
		result = 14;
		break;
	case 5132:
		result = 15;
		break;
	case 5108:
		result = 16;
		break;
	default:
		result = 0;
		break;
	}

	return result;
}

int WorldmapFix::CanAccessWorldmap()
{
	signed int result;
	int v1;

	v1 = maps.GetCurrentMapId() - 5001;
	switch (v1)
	{
	case 120:
		result = 1;
		break;
	case 131:
		result = 1;
		break;
	case 107:
		result = 1;
		break;
	default:
		switch (v1)
		{
		case 0:
		case 1:
		case 50:
		case 61:
		case 67:
		case 68:
		case 90:
		case 92:
		case 93:
		case 94:
		case 111:
		case 112:
			result = 1;
			break;
		default:
			result = 0;
			break;
		}
		break;
	}
	return result;
}

void UiWorldmapMakeTripCdecl(int fromId, int toId) {
	auto asdf = 1;
}

UiWorldmapImpl::UiWorldmapImpl(){

	// Todo read this from file to support new modules
	areaToLocId.resize(64, 0);
	int i = 0;
	for (auto it : {
	-1,
	9,
	1, // Area 2 - Moathouse
	3,
	6,
	0,
	4,
	7,
	2,
	5,
	8,
	12,
	13,
	0,
	14,
	15,
	16,
		}){
		areaToLocId[i++] = it;
	}
}

void __declspec(naked) UiWorldmapMakeTripWrapper() {
	macAsmProl; // esp = esp0 - 16
	__asm {
		push eax; // esp = esp0 - 20
		push ecx; // esp = esp0 - 24
		mov esi, UiWorldmapMakeTripCdecl;
		call esi;
		add esp, 8;
	}
	macAsmEpil;
	__asm retn;
};
