from toee import *
import tpdp
import tpactions
from warlock import EldritchBlastEssenceModifier, verifyEldritchBlastAction
from spell_utils import SpellPythonModifier

print "Registering sp-Brimstone Blast"

########## Python Action ID's ##########
brimstoneBlastEnum = 3310
########################################

def secondaryEffect(attachee, args, evt_obj):
    currentSequence = tpactions.get_cur_seq()
    spellId = currentSequence.spell_action.spell_id
    spellPacket = tpdp.SpellPacket(spellId)
    if verifyEldritchBlastAction(spellPacket.spell_enum):
        spellDc = spellPacket.dc
        saveType = D20_Save_Reflex
        saveDescriptor = D20STD_F_SPELL_DESCRIPTOR_FIRE
        spellTarget = evt_obj.attack_packet.target
        if spellTarget.saving_throw_spell(spellDc, saveType, saveDescriptor, spellPacket.caster, spellId): #success
            spellTarget.float_mesfile_line("mes\\spell.mes", 30001)
            game.particles("Fizzle", spellTarget)
        else:
            spellTarget.float_mesfile_line("mes\\spell.mes", 30002)
            duration = int(spellPacket.caster_level / 5)
            spellTarget.condition_add_with_args("Brimstone Burn", spellId, duration, 0)
    return 0

brimstoneBlast = EldritchBlastEssenceModifier("Brimstone Blast") #spellEnum, empty
brimstoneBlast.ModifyDamageType(D20DT_FIRE)
brimstoneBlast.AddQuerySecondaryTrue()

### Secondary Burn Effect ###

def burnDamage(attachee, args, evt_obj):
    spellId = args.get_arg(0)
    spellPacket = tpdp.SpellPacket(spellId)
    spellDamageDice = dice_new("1d6")
    spellDamageDice.number = 2
    attachee.float_text_line("Burning")
    game.create_history_freeform("{} takes ~Brimstone Blast~[TAG_SPELLS_BRIMSTONE_BLAST] burn damage".format(attachee.description))
    spellTarget.obj.spell_damage(spellPacket.caster, D20DT_FIRE, spellDamageDice, D20DAP_UNSPECIFIED, D20A_CAST_SPELL, spellId)
    return 0

def radialExtinguishFlames(attachee, args, evt_obj):
    radialName = "Extinguish Flames"
    radialHelpTag = "TAG_SPELLS_BRIMSTONE_BLAST"
    radialId = tpdp.RadialMenuEntryPythonAction(radialName, D20A_PYTHON_ACTION, brimstoneBlastEnum, 0, radialHelpTag)
    radialId.add_child_to_standard(attachee, tpdp.RadialMenuStandardNode.Options)
    return 0

def performExtinguishFlames(attachee, args, evt_obj):
    attachee.float_text_line("Flames Extinguished", tf_white)
    args.remove_spell_mod()
    args.remove_spell()
    return 0

brimstoneEffect = SpellPythonModifier("Brimstone Blast Effect") #spellId, duration, empty
brimstoneEffect.AddHook(ET_OnBeginRound, EK_NONE, burnDamage, ())
brimstoneEffect.AddHook(ET_OnBuildRadialMenuEntry, EK_NONE, radialExtinguishFlames, ())
brimstoneEffect.AddHook(ET_OnD20PythonActionPerform, brimstoneBlastEnum, performExtinguishFlames, ())
brimstoneEffect.AddSpellNoDuplicate()
