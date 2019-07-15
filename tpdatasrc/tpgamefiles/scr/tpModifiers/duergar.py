from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import race_utils

###################################################

def GetConditionName():
	return "Duergar"


raceEnum = race_dwarf + (3 << 5)
raceSpecModule = __import__('race097_duergar')
###################################################

BONUS_MES_RACIAL_BONUS = 139
BONUS_MES_STABILITY    = 317

def OnGetFavoredClass(attachee, args, evt_obj):
	if evt_obj.data1 == stat_level_fighter:
		evt_obj.return_val = 1
	return 0

def ConditionImmunityOnPreAdd(attachee, args, evt_obj):
	val = evt_obj.is_modifier("Paralyzed")
	if val:
		evt_obj.return_val = 0
		attachee.float_text_line( "Paralysis Immunity", tf_red )
	return 0
def OnQueryReturnTrue(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0

def PhantasmImmunity(attachee, args, evt_obj):
	sp_pkt = evt_obj.spell_packet
	# Check if phantasm subschool
	spell_enum = sp_pkt.spell_enum
	if (spell_enum == 0):
		return 0
	spell_entry = tpdp.SpellEntry(spell_enum)
	if (spell_entry.spell_school_enum != Illusion or spell_entry.spell_subschool_enum != Phantasm):
		return 0
	
	evt_obj.return_val = 1
	
	return 0
	
def PhantasmImmunitySaveBonus(attachee, args, evt_obj):
	flags = evt_obj.flags
	if not (flags & (1<< (D20STD_F_SPELL_LIKE_EFFECT-1) ) ): # 0x10
		return 0
	
	sp_pkt = evt_obj.spell_packet
	# Check if phantasm subschool
	spell_enum = sp_pkt.spell_enum
	if (spell_enum == 0):
		return 0
	spell_entry = tpdp.SpellEntry(spell_enum)
	if (spell_entry.spell_school_enum != Illusion or spell_entry.spell_subschool_enum != Phantasm):
		return 0
	
	evt_obj.return_val = 1
	
	return 0
	
def OnGetMoveSpeedSetLowerLimit(attachee, args, evt_obj):
	# this sets the lower limit for dwarf move speed at 20, unless someone has already set it (e.g. by web/entangle)
	if evt_obj.bonus_list.flags & 2:
		return 0
	moveSpeedCapValue = 20
	capFlags = 2 # set lower limit
	capType  = 0 # operate on all bonus types
	bonusMesline = BONUS_MES_RACIAL_BONUS # racial ability
	evt_obj.bonus_list.set_overall_cap(capFlags, moveSpeedCapValue, capType, bonusMesline)
	return 0

def OnGetAppraiseSkill(attachee, args, evt_obj):
	# adds appraise bonus to metal or rock items
	item = evt_obj.obj
	if item == OBJ_HANDLE_NULL:
		return 0
	item_material = item.obj_get_int(obj_f_material)
	if (item_material == mat_stone or item_material == mat_metal):
		evt_obj.bonus_list.add(2, 0, BONUS_MES_RACIAL_BONUS)
	return 0

def OnGetToHitBonusVsOrcsAndGoblins(attachee, args, evt_obj):
	target = evt_obj.attack_packet.target
	if target == OBJ_HANDLE_NULL:
		return 0
	if target.is_category_subtype(mc_subtype_half_orc) or target.is_category_subtype(mc_subtype_goblinoid) or target.is_category_subtype(mc_subtype_orc):
		evt_obj.bonus_list.add(1, 0, BONUS_MES_RACIAL_BONUS)
	return 0

def OnGetArmorClassBonusVsGiants(attachee, args, evt_obj):
	attacker = evt_obj.attack_packet.attacker
	if attacker == OBJ_HANDLE_NULL:
		return 0
	if attacker.is_category_type(mc_type_giant):
		evt_obj.bonus_list.add(4, 8, BONUS_MES_RACIAL_BONUS)
	return 0

def OnAbilityModCheckStabilityBonus(attachee, args, evt_obj):
	flags = evt_obj.flags
	if (flags & 1) and (flags & 2): # defender bonus
		evt_obj.bonus_list.add(4, 22, BONUS_MES_STABILITY)
	return 0

def CasterLevelRacialSpell(attachee, args, evt_obj):
	# Set caster level for racial spells to 2x character level (minimum 3)
	sp_pkt = evt_obj.get_spell_packet()
	if sp_pkt.spell_class != domain_special:
		return 0
	caster_level = max(3, 2 * attachee.stat_level_get(stat_level))
	if evt_obj.return_val < caster_level:
		evt_obj.return_val = caster_level
	return 0
##########################################################

raceSpecObj = PythonModifier(GetConditionName(), 0)
race_utils.AddAbilityModifierHooks(raceSpecObj, raceSpecModule)
race_utils.AddSaveBonusVsEffectType(raceSpecObj, D20STD_F_SPELL_LIKE_EFFECT, 2)
race_utils.AddSkillBonuses(raceSpecObj, {skill_listen: 1, skill_spot: 1, skill_move_silently: 4})
race_utils.AddBaseMoveSpeed(raceSpecObj, 20) # note: dwarven move speed with heavy armor or when medium/heavy encumbered is already handled in Encumbered Medium, Encumbered Heavy condition callbacks
race_utils.AddPoisonImmunity(raceSpecObj)
race_utils.AddFavoredClassHook(raceSpecObj, stat_level_fighter)

raceSpecObj.AddHook(ET_OnGetSkillLevel,   EK_SKILL_APPRAISE, OnGetAppraiseSkill, ())
raceSpecObj.AddHook(ET_OnGetMoveSpeed,    EK_NONE,           OnGetMoveSpeedSetLowerLimit, ())
raceSpecObj.AddHook(ET_OnConditionAddPre, EK_NONE,           ConditionImmunityOnPreAdd, ()) # paralysis immunity (affects normal Paralysis condition)
raceSpecObj.AddHook(ET_OnD20Query,        EK_Q_Critter_Is_Immune_Paralysis, OnQueryReturnTrue, ()) # paralysis immunity query - used in spell paralysis effects such as Hold Person/Monster
raceSpecObj.AddHook(ET_OnSpellImmunityCheck,      EK_NONE,   PhantasmImmunity, ()) # phantasm immunity
raceSpecObj.AddHook(ET_OnSaveThrowLevel    ,      EK_NONE,   PhantasmImmunitySaveBonus, ()) # phantasm immunity via save throw - incomplete approach since you can fail on a natural 1
raceSpecObj.AddHook(ET_OnToHitBonus2,     EK_NONE,           OnGetToHitBonusVsOrcsAndGoblins, ())
raceSpecObj.AddHook(ET_OnGetAC,           EK_NONE,           OnGetArmorClassBonusVsGiants, ())
raceSpecObj.AddHook(ET_OnGetAbilityCheckModifier, EK_NONE,   OnAbilityModCheckStabilityBonus, ())
raceSpecObj.AddHook(ET_OnGetCasterLevelMod,       EK_NONE,   CasterLevelRacialSpell, ())