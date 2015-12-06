
#include "stdafx.h"

#include <temple/dll.h>
#include "temple_enums.h"

#include <infrastructure/json11.hpp>
using namespace json11;

#pragma pack(push,1)
struct ProtoCol {
	int parseFunc;
	int arg1;
	int arg2;
	int arg3;
	int arg4;
	int field14;
};

struct ProtoNameSpec {

};
#pragma pack(pop)

#include <fstream>

std::vector<Json> GetStringList(uint32_t count, uint32_t stringsAddr) {
	char** strings = reinterpret_cast<char**>(stringsAddr);

	std::vector<Json> result;
	result.reserve(count);
	for (size_t i = 0; i < count; ++i) {
		if (!strings[i]) {
			result.push_back(nullptr);
		}
		else {
			result.push_back(strings[i]);
		}
	}
	return result;

}

void DumpProtos() {
	auto protoCol = temple::GetPointer<ProtoCol>(0x102AD564);
	auto protoColEnd = temple::GetPointer<ProtoCol>(0x102AF4C0);

	std::vector<Json> cols;

	int colIdx = 0;
	for (int colIdx = 0; colIdx < 334; ++colIdx) {
		auto obj = Json::object{};

		switch (protoCol->parseFunc) {
		case 0x10039380:
			obj["parser"] = "int32";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x10039560:
			obj["parser"] = "int32_flags";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			obj["flags"] = GetStringList(protoCol->arg2, protoCol->arg3);
			break;
		case 0x10039640:
			obj["parser"] = "char";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x10039480:
			obj["parser"] = "float";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x10039680:
			obj["parser"] = "int32_enum";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			obj["values"] = GetStringList(protoCol->arg2, protoCol->arg3);
			break;
		case 0x101f5850:
			obj["parser"] = "skip";
			break;
		case 0x100393c0:
			obj["parser"] = "int32_two_fields";
			obj["field1"] = GetObjectFieldName((obj_f)protoCol->arg1);
			obj["field2"] = GetObjectFieldName((obj_f)protoCol->arg2);
			break;
		case 0x100394c0:
			obj["parser"] = "float_radius";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x10039510:
			obj["parser"] = "float_height";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x10039410:
			obj["parser"] = "inven_src";
			break;
		case 0x1003aa90:
			obj["parser"] = "spell_charges_idx";
			break;
		case 0x100397d0:
			obj["parser"] = "d20_dmg_type";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x10039b80:
			obj["parser"] = "dice";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x10039ce0:
			obj["parser"] = "crit_range";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x1003a780:
			obj["parser"] = "armor_type";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			obj["values"] = GetStringList(protoCol->arg2, protoCol->arg3);
			break;
		case 0x1003a840:
			obj["parser"] = "helm_size";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			obj["values"] = GetStringList(protoCol->arg2, protoCol->arg3);
			break;
		case 0x10039d20:
			obj["parser"] = "int32_indexed";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			obj["idx"] = protoCol->arg2;
			break;
		case 0x10039b40:
			obj["parser"] = "race";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x100396f0:
			obj["parser"] = "gender";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x10039d60:
			obj["parser"] = "deity";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x10039c70:
			obj["parser"] = "dice_indexed";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			obj["idx"] = protoCol->arg2;
			break;
		case 0x10039760:
			obj["parser"] = "int32_enum_indexed";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			obj["idx"] = protoCol->arg4;
			obj["values"] = GetStringList(protoCol->arg2, protoCol->arg3);
			break;
		case 0x10039990:
			obj["parser"] = "hair_color";
			obj["values"] = GetStringList(8, 0x102E3CD8);
			break;
		case 0x10039a60:
			obj["parser"] = "hair_style";
			obj["values"] = GetStringList(8, 0x102E3CF8);
			break;
		case 0x1003a700:
			obj["parser"] = "factions";
			break;
		case 0x10039da0:
			obj["parser"] = "challenge_rating";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x10039bf0:
			obj["parser"] = "hit_dice";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			break;
		case 0x10039850:
			obj["parser"] = "monster_category";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			obj["values"] = GetStringList(protoCol->arg2, protoCol->arg3);
			break;
		case 0x100398c0:
			obj["parser"] = "monster_subtypes";
			obj["field"] = GetObjectFieldName((obj_f)protoCol->arg1);
			obj["flags"] = GetStringList(protoCol->arg2, protoCol->arg3);
			break;
		case 0x10039e80:
			obj["idx"] = protoCol->arg1;
			if (protoCol->arg2 == 0) {
				obj["parser"] = "cond_name";
			}
			else if (protoCol->arg2 == 1) {
				obj["parser"] = "cond_arg1";
			}
			else if (protoCol->arg2 == 2) {
				obj["parser"] = "cond_arg2";
			}
			else {
				obj["parser"] = "cond_unknown";
			}
			break;
		case 0x1003a430:
			obj["idx"] = protoCol->arg1;
			if (protoCol->arg2 == 3) {
				obj["parser"] = "classlevel_name";
			}
			else if (protoCol->arg2 == 4) {
				obj["parser"] = "classlevel_count";
			}
			else {
				obj["parser"] = "classlevel_unknown";
			}
			break;
		case 0x1003a530:
			obj["idx"] = protoCol->arg1;
			if (protoCol->arg2 == 5) {
				obj["parser"] = "skill_name";
			}
			else if (protoCol->arg2 == 6) {
				obj["parser"] = "skill_rank";
			}
			else {
				obj["parser"] = "skill_unknown";
			}
			break;
		case 0x1003a600:
			obj["parser"] = "feat";
			obj["idx"] = protoCol->arg1;
			break;
		case 0x1003a660:
			obj["parser"] = "script";
			obj["idx"] = protoCol->arg1;
			break;
		case 0x10039360:
			obj["parser"] = "id";
			break;
		case 0x1003a8c0:
			obj["parser"] = "spell_known";
			break;
		case 0x1003acc0:
			obj["parser"] = "critter_pad_i_4";
			break;
		case 0x1003ad20:
			obj["parser"] = "critter_strategy";
			break;
		default:
			obj["parser"] = fmt::format("0x{:x}", protoCol->parseFunc);
			obj["arg1"] = protoCol->arg1;
			obj["arg2"] = protoCol->arg2;
			obj["arg3"] = protoCol->arg3;
			obj["arg4"] = protoCol->arg4;
			break;
		}

		cols.push_back(obj);

		protoCol++;
	}

	std::ofstream o("proto_cols.json");
	o << Json(cols).dump();
	o.close();
	exit(0);
}
