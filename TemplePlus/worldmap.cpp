#include "stdafx.h"
#include "common.h"
#include "util/fixes.h"
#include "maps.h"

// Worldmap extension by Spellslinger
class WorldmapFix: TempleFix
{
public:
	static int GetAreaFromMap(int mapId);
	static int CanAccessWorldmap();
	
	void apply() override {

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