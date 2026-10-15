from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import tpactions
import char_class_utils

###################################################
# Dragon Disciple (3.5 SRD). Class spec: rules/char_class/class023_dragon_disciple.py

def GetConditionName():
    return "Dragon Disciple"

print "Registering " + GetConditionName()

classEnum = stat_level_dragon_disciple
classSpecModule = __import__('class023_dragon_disciple')

breathWeaponActionEnum = 2302
heritageActionEnum = 2304
spellDDConeBreath = 3231

###################################################

#### standard callbacks - BAB and Save values
def OnGetToHitBonusBase(attachee, args, evt_obj):
    classLvl = attachee.stat_level_get(classEnum)
    babvalue = game.get_bab_for_class(classEnum, classLvl)
    evt_obj.bonus_list.add(babvalue, 0, 137) # untyped, description: "Class"
    return 0

def OnGetSaveThrowFort(attachee, args, evt_obj):
    value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Fortitude)
    evt_obj.bonus_list.add(value, 0, 137)
    return 0

def OnGetSaveThrowReflex(attachee, args, evt_obj):
    value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Reflex)
    evt_obj.bonus_list.add(value, 0, 137)
    return 0

def OnGetSaveThrowWill(attachee, args, evt_obj):
    value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Will)
    evt_obj.bonus_list.add(value, 0, 137)
    return 0

classSpecObj = PythonModifier(GetConditionName(), 0)
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())

def ddLevel(attachee):
    return attachee.stat_level_get(classEnum)

#### Ability boosts (SRD): Str +2 at 2nd, +2 at 4th (+4 more at 10th from apotheosis), Con +2 at 6th,
#### Int +2 at 8th, Cha +2 at 10th (apotheosis). Untyped (stacking) inherent-style bonuses.
def abilityBoost(attachee, args, evt_obj):
    stat = args.get_param(0)
    lvl = ddLevel(attachee)
    bonus = 0
    if stat == stat_strength:
        if lvl >= 2: bonus += 2
        if lvl >= 4: bonus += 2
        if lvl >= 10: bonus += 4
    elif stat == stat_constitution:
        if lvl >= 6: bonus += 2
    elif stat == stat_intelligence:
        if lvl >= 8: bonus += 2
    elif stat == stat_charisma:
        if lvl >= 10: bonus += 2
    if bonus:
        evt_obj.bonus_list.add(bonus, 0, "~Dragon Disciple Ability Boost~[TAG_CLASS_FEATURES_DRAGON_DISCIPLES_ABILITY_BOOST]")
    return 0

abilityBoostCond = PythonModifier("Dragon Disciple Ability Boost", 2) # feat conditions get (featEnum, arg)
abilityBoostCond.MapToFeat("Dragon Disciple Ability Boost")
abilityBoostCond.AddHook(ET_OnAbilityScoreLevel, EK_STAT_STRENGTH, abilityBoost, (stat_strength,))
abilityBoostCond.AddHook(ET_OnAbilityScoreLevel, EK_STAT_CONSTITUTION, abilityBoost, (stat_constitution,))
abilityBoostCond.AddHook(ET_OnAbilityScoreLevel, EK_STAT_INTELLIGENCE, abilityBoost, (stat_intelligence,))
abilityBoostCond.AddHook(ET_OnAbilityScoreLevel, EK_STAT_CHARISMA, abilityBoost, (stat_charisma,))

#### Natural armor: +1 at 1st, +2 at 4th, +3 at 7th, +4 at 10th (apotheosis). Natural armor bonus type (stacks with existing natural armor as an increase).
def naturalArmorACBonus(attachee, args, evt_obj):
    lvl = ddLevel(attachee)
    if lvl >= 10: bonus = 4
    elif lvl >= 7: bonus = 3
    elif lvl >= 4: bonus = 2
    else: bonus = 1
    evt_obj.bonus_list.add(bonus, 0, "~Dragon Disciple Natural Armor~[TAG_CLASS_FEATURES_DRAGON_DISCIPLES_NATURAL_ARMOR_INCREASE]") # 0: stacks with the base natural armor (SRD: "increase")
    return 0

naturalArmorInc = PythonModifier("Dragon Disciple Natural Armor", 2) # feat conditions get (featEnum, arg)
naturalArmorInc.MapToFeat("Dragon Disciple Natural Armor")
naturalArmorInc.AddHook(ET_OnGetAC, EK_NONE, naturalArmorACBonus, ())

#### Bonus spells per day: one extra spell per day at DD levels 1, 2, 3, 5, 6, 7 and 9, for the spontaneous
#### arcane class. Each goes into the highest spell level that class can cast. (Approximation: DD doesn't
#### advance casting, so the base class's highest level normally stays the same while taking DD levels.)
bonusSpellLevels = (1, 2, 3, 5, 6, 7, 9)

def ddBonusSpellCount(attachee):
    lvl = ddLevel(attachee)
    return len([l for l in bonusSpellLevels if l <= lvl])

def bonusSpellsPerDay(attachee, args, evt_obj):
    baseClass = attachee.highest_spontaneous_arcane_class
    if baseClass <= 0 or evt_obj.caster_class != baseClass:
        return 0
    if evt_obj.spell_level != attachee.arcane_spontaneous_spell_level_can_cast():
        return 0
    count = ddBonusSpellCount(attachee)
    if count:
        evt_obj.bonus_list.add(count, 0, "~Dragon Disciple Bonus Spells~[TAG_CLASS_FEATURES_DRAGON_DISCIPLES_BONUS_SPELLS]")
    return 0

bonusSpells = PythonModifier("Dragon Disciple Bonus Spells", 2) # feat conditions get (featEnum, arg)
bonusSpells.MapToFeat("Dragon Disciple Bonus Spells")
bonusSpells.AddHook(ET_OnGetSpellsPerDayMod, EK_NONE, bonusSpellsPerDay, ())

#### Draconic heritage (dragon colour). Picked once via the radial menu; stored in a persistent condition.
# colour index -> (name, damage type, breath shape per SRD)
DRAGON_COLOURS = [
    ("Black", D20DT_ACID, "line"), ("Blue", D20DT_ELECTRICITY, "line"), ("Green", D20DT_ACID, "cone"),
    ("Red", D20DT_FIRE, "cone"), ("White", D20DT_COLD, "cone"), ("Brass", D20DT_FIRE, "line"),
    ("Bronze", D20DT_ELECTRICITY, "line"), ("Copper", D20DT_ACID, "line"), ("Gold", D20DT_FIRE, "cone"),
    ("Silver", D20DT_COLD, "cone"),
]

def getHeritage(attachee):
    # 0 = not chosen yet, else colour index + 1
    return attachee.d20_query("PQ_Dragon_Disciple_Heritage")

def getHeritageInfo(attachee):
    h = getHeritage(attachee)
    if h < 1 or h > len(DRAGON_COLOURS):
        return None
    return DRAGON_COLOURS[h - 1]

def heritageRadial(attachee, args, evt_obj):
    info = getHeritageInfo(attachee)
    if info:
        return 0 # already chosen; shown in the breath weapon entry
    parent = tpdp.RadialMenuEntryParent("Choose Draconic Heritage")
    parentId = parent.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    for i, (name, dt, shape) in enumerate(DRAGON_COLOURS):
        e = tpdp.RadialMenuEntryPythonAction(name, D20A_PYTHON_ACTION, heritageActionEnum, i + 1, "TAG_CLASS_FEATURES_DRAGON_DISCIPLES_DRACONIC_HERITAGE")
        e.add_as_child(attachee, parentId)
    return 0

def heritagePerform(attachee, args, evt_obj):
    choice = evt_obj.d20a.data1
    if getHeritage(attachee) == 0 and 1 <= choice <= len(DRAGON_COLOURS):
        attachee.condition_add_with_args("Dragon Disciple Heritage Choice", choice, 0)
        name = DRAGON_COLOURS[choice - 1][0]
        attachee.float_text_line("{} dragon heritage".format(name))
        game.create_history_freeform("{} chooses the ~{} dragon~[TAG_CLASS_FEATURES_DRAGON_DISCIPLES_DRACONIC_HERITAGE] heritage\n\n".format(attachee.description, name))
    return 0

heritageFeat = PythonModifier("Dragon Disciple Heritage", 2) # feat conditions get (featEnum, arg)
heritageFeat.MapToFeat("Dragon Disciple Heritage")
heritageFeat.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, heritageRadial, ())
heritageFeat.AddHook(ET_OnD20PythonActionPerform, heritageActionEnum, heritagePerform, ())

def heritageQuery(attachee, args, evt_obj):
    evt_obj.return_val = args.get_arg(0)
    return 0

# persistent (added with condition_add, saved with the character): arg0 = colour index + 1
heritageChoice = PythonModifier("Dragon Disciple Heritage Choice", 2)
heritageChoice.AddHook(ET_OnD20PythonQuery, "PQ_Dragon_Disciple_Heritage", heritageQuery, ())

#### Claws and bite (2nd level), via the natural weapon blend (ET_OnGetExtraNaturalAttacks):
#### usable together with weapons (as secondary attacks), replace unarmed strikes when unarmed.
#### SRD damage: claws 1d3 (Small) / 1d4 (Medium) / 1d6 (Large); bite 1d4 / 1d6 / 1d8.
def clawsAndBite(attachee, args, evt_obj):
    size = attachee.get_size
    if size <= STAT_SIZE_SMALL:
        claw, bite = dice_new("1d3"), dice_new("1d4")
    elif size >= STAT_SIZE_LARGE:
        claw, bite = dice_new("1d6"), dice_new("1d8")
    else:
        claw, bite = dice_new("1d4"), dice_new("1d6")
    evt_obj.add_attack(NATURAL_ATTACK_CLAW, claw, D20DT_SLASHING, NAF_PRIMARY | NAF_NEEDS_FREE_HAND)
    evt_obj.add_attack(NATURAL_ATTACK_CLAW, claw, D20DT_SLASHING, NAF_PRIMARY | NAF_NEEDS_FREE_HAND)
    evt_obj.add_attack(NATURAL_ATTACK_BITE, bite, D20DT_PIERCING, NAF_PRIMARY | NAF_MOUTH)
    return 0

clawsBite = PythonModifier("Dragon Disciple Claws and Bite", 2) # feat conditions get (featEnum, arg)
clawsBite.MapToFeat("Dragon Disciple Claws and Bite")
clawsBite.AddHook(ET_OnGetExtraNaturalAttacks, EK_NONE, clawsAndBite, ())

#### Breath weapon (3rd level): 1/day, 2d8 (3rd), 4d8 (7th), 6d8 (10th), Reflex half, DC 10 + DD level + Con mod.
#### Always a 30 ft cone here (line-shaped colours are approximated as a cone); energy type by heritage.
#### Spell 3231 ("Dragon Disciple Cone Breath"), cone/save code shared with Frost Breath.
def breathRadial(attachee, args, evt_obj):
    info = getHeritageInfo(attachee)
    if not info:
        return 0 # choose a heritage first
    if args.get_arg(2):
        label = "Breath Weapon ({}, used today)".format(info[0])
    else:
        label = "Breath Weapon ({})".format(info[0])
    e = tpdp.RadialMenuEntryPythonAction(label, D20A_PYTHON_ACTION, breathWeaponActionEnum, 0, "TAG_CLASS_FEATURES_DRAGON_DISCIPLES_BREATH_WEAPON")
    spellData = tpdp.D20SpellData(spellDDConeBreath)
    spellData.set_spell_class(classEnum)
    spellData.set_spell_level(ddLevel(attachee))
    e.set_spell_data(spellData)
    e.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Class)
    return 0

def breathCheck(attachee, args, evt_obj):
    if args.get_arg(2):
        evt_obj.return_val = AEC_OUT_OF_CHARGES
    elif not getHeritageInfo(attachee):
        evt_obj.return_val = AEC_INVALID_ACTION
    return 0

def breathPerform(attachee, args, evt_obj):
    args.set_arg(2, 1) # used today
    currentSequence = tpactions.get_cur_seq()
    spellPacket = currentSequence.spell_packet
    spellPacket.caster_level = ddLevel(attachee)
    newSpellId = tpactions.get_new_spell_id()
    tpactions.register_spell_cast(spellPacket, newSpellId)
    evt_obj.d20a.spell_id = newSpellId
    currentSequence.spell_action.spell_id = newSpellId
    game.create_history_freeform("{} uses ~Breath Weapon~[TAG_CLASS_FEATURES_DRAGON_DISCIPLES_BREATH_WEAPON]\n\n".format(attachee.description))
    tpactions.trigger_spell_effect(newSpellId)
    return 0

def breathReset(attachee, args, evt_obj):
    args.set_arg(2, 0)
    return 0

breathWeapon = PythonModifier("Dragon Disciple Breath Weapon", 3) # arg0: featEnum, arg1: spare, arg2: used today
breathWeapon.MapToFeat("Dragon Disciple Breath Weapon")
breathWeapon.AddHook(ET_OnConditionAdd, EK_NONE, breathReset, ())
breathWeapon.AddHook(ET_OnNewDay, EK_NEWDAY_REST, breathReset, ())
breathWeapon.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, breathRadial, ())
breathWeapon.AddHook(ET_OnD20PythonActionCheck, breathWeaponActionEnum, breathCheck, ())
breathWeapon.AddHook(ET_OnD20PythonActionPerform, breathWeaponActionEnum, breathPerform, ())

#### Blindsense (5th: 30 ft, 10th: 60 ft). Approximation: within range the DD notices and can target
#### creatures it can't see (critterSys.CanSense via the "Blindsense Range" query), but concealment and
#### invisibility miss chances still apply (unlike the Scout's "Blindsight Range", which negates them).
def blindsenseRange(attachee, args, evt_obj):
    evt_obj.return_val = 60 if ddLevel(attachee) >= 10 else 30
    return 0

blindsense = PythonModifier("Dragon Disciple Blindsense", 2) # feat conditions get (featEnum, arg)
blindsense.MapToFeat("Dragon Disciple Blindsense")
blindsense.AddHook(ET_OnD20PythonQuery, "Blindsense Range", blindsenseRange, ())

#### Wings (9th): cosmetic only (ToEE has no flight). Darley's addmesh (vanilla art\meshes\armor\darley_addmesh),
#### mapped in rules/addmeshes/dragon_disciple_wings.mes.
ddWingsAddmeshId = 19500

def addWings(attachee, args, evt_obj):
    evt_obj.append(ddWingsAddmeshId)
    return 0

wings = PythonModifier("Dragon Disciple Wings", 2) # feat conditions get (featEnum, arg)
wings.MapToFeat("Dragon Disciple Wings")
wings.AddHook(ET_OnAddMesh, EK_NONE, addWings, ())

#### Dragon apotheosis (10th): +4 Str / +2 Cha and natural armor +4 are in the boosts above;
#### immunity to sleep, paralysis and the heritage's energy type. Darkvision / low-light vision: skipped
#### (no lighting-based vision rules in ToEE). Half-dragon type change: not implemented.
def apotheosisImmunities(attachee, args, evt_obj):
    if evt_obj.is_modifier("sp-Sleep"):
        evt_obj.return_val = 0
        attachee.float_mesfile_line('mes\\combat.mes', 5059, tf_red) # "Sleep Immunity"
    elif evt_obj.is_modifier("Paralyzed") or evt_obj.is_modifier("sp-Hold Person") or evt_obj.is_modifier("sp-Hold Monster"):
        evt_obj.return_val = 0
        attachee.float_text_line("Paralysis Immunity", tf_red)
    return 0

def apotheosisEnergyImmunity(attachee, args, evt_obj):
    info = getHeritageInfo(attachee)
    if info:
        evt_obj.damage_packet.add_mod_factor(0.0, info[1], 132) # damage.mes 132: Immunity
    return 0

def apotheosisParalysisQuery(attachee, args, evt_obj):
    evt_obj.return_val = 1
    return 0

apotheosis = PythonModifier("Dragon Disciple Dragon Apotheosis", 2) # feat conditions get (featEnum, arg)
apotheosis.MapToFeat("Dragon Disciple Dragon Apotheosis")
apotheosis.AddHook(ET_OnConditionAddPre, EK_NONE, apotheosisImmunities, ())
apotheosis.AddHook(ET_OnTakingDamage2, EK_NONE, apotheosisEnergyImmunity, ())
