from t import *


#################################
# TELEPORT SHORTCUTS		#
#################################





def shopmap():
	game.fade_and_teleport(0,0,0,5123,474,465)
	return




#################################
# THE KEEP						#
#################################

def bailey():
	game.fade_and_teleport(0,0,0,5001,461,563)
	return
def kotb():
	game.fade_and_teleport(0,0,0,5001,461,563)
	return
def southernbailey():
	game.fade_and_teleport(0,0,0,5001,461,563)
	return
def keep():
	game.fade_and_teleport(0,0,0,5001,461,563)
	return
def innerbailey():
	game.fade_and_teleport(0,0,0,5115,588,494)
	return
def inner_bailey():
	innerbailey()
	return
def fortressentrance():
	game.fade_and_teleport(0,0,0,5017,487,460)
	return
def innerfort():
	fortressentrance()
	return
def fortress():
	fortressentrance()
	return
def fort():
	fortressentrance()
	return
def quartermaster():
	game.fade_and_teleport(0,0,0,5017,447,435)
	return
	
def keepoutside():
	game.fade_and_teleport(0,0,0,5063,508,503)
	return
def outsidekeep():
	keepoutside()
	return
def maingate():
	keepoutside()
	return
def keepgate():
	keepoutside()
	return
def kotbgate():
	keepoutside()
	return
def gate():
	keepoutside()
	return
	
def smith():
	game.fade_and_teleport(0,0,0,5001,503,521)
	return

	
def marketplace():
	game.fade_and_teleport(0,0,0,5001,544,388)
	return
def market():
	marketplace()
	return
def bazar():
	marketplace()
	return
def reece():
	marketplace()
	return
def ricario():
	marketplace()
	return
def geoffrey():
	marketplace()
	return
	
def flay():
	game.fade_and_teleport(0,0,0,5045,524,499)
	return
def robin():
	game.fade_and_teleport(0,0,0,5045,524,499)
	return
def robingraves():
	robin()
	return
	
	
def graveyard():
	game.fade_and_teleport(0,0,0,5126,497,523)
	return
	
def inn():
	game.fade_and_teleport(0,0,0,5007,517,490)
	return
def keepinn():
	game.fade_and_teleport(0,0,0,5007,517,490)
	return

def animalseller():
	game.fade_and_teleport(0,0,0,5043,480,480)
	return	

def church():
	game.fade_and_teleport(0,0,0,5033,512,489)
	
def jayfie():
	if is_daytime():
		game.fade_and_teleport(0,0,0,5001,48,698)
	else:
		jayfienight()
	return

def wainwright():
	if is_daytime():
		game.fade_and_teleport(0,0,0,5016,494,487)
	else:
		inn()
	return

	
def gaolcell():
	game.fade_and_teleport(0,0,0,5125,473,525)
	return

#################################
# Northern Oak Woods			#
# 								#
#################################
	
def harpywoods():
	game.fade_and_teleport(0,0,0,5094,565,588)
	return
def northernoakwoods():
	harpywoods()
	return
def northernwoods():
	harpywoods()
	return
def oakwoods():
	harpywoods()
	return
def northwoods():
	harpywoods()
	return
def northwood():
	harpywoods()
	return
def harpies():
	game.fade_and_teleport(0,0,0,5094,499,536)
	return
def northernwoodsshrine():
	harpies()
	return
def northwoodsshrine():
	harpies()
	return
def harpyshrine():
	harpies()
	return
def hermit():
	game.fade_and_teleport(0,0,0,5094,400,476)
	return
	
	
def passagebeanththewoods():
	game.fade_and_teleport(0,0,0,5133,454,491)
	return
def underpass():
	passagebeanththewoods()
	return
def underdark():
	passagebeanththewoods()
	return
def passage():
	passagebeanththewoods()
	return


#################################
# Mad Wizard Tower	& Swamp		#
# 								#
#################################
	

def swamp():
	game.fade_and_teleport(0,0,0,5095,407,474)
	return
def lizards():
	swamp()
	return
def lizardswamp():
	swamp()
	return

def lizardchief():
	game.fade_and_teleport(0,0,0,5114,491,465)
	return
	
def madwizardoutside():
	game.fade_and_teleport(0,0,0,5069,515,480)
	return

def madwizard():
	madwizardoutside()
	return

def deepswamp():
	madwizardoutside()
	return
def thedeepswamp():
	madwizardoutside()
	return
	
def madwizardmainhall():
	game.fade_and_teleport(0,0,0,5130,477,490)
	return
def wizardtowermainhall():
	madwizardmainhall()
	return
def towermainhall():
	madwizardmainhall()
	return

def madwizardupperhall():
	game.fade_and_teleport(0,0,0,5131,482,472)
	return
def wizardtowerupperhall():
	madwizardupperhall()
	return
def towerupperhall():
	madwizardupperhall()
	return
	
def madwizardinnerparapet():
	game.fade_and_teleport(0,0,0,5132,480,478)
	return
def wizardtowerinnerparapet():
	madwizardinnerparapet()
	return
def towerinnerparapet():
	madwizardinnerparapet()
	return
	
	
#################################
# Spider Pine Woods				#
# 								#
#################################
	
def reynard():
	if game.quests[5].state != qs_completed:
		game.fade_and_teleport(0,0,0,5004,473,475)
	else:
		marketplace()
	return
	
def abandonedflet():
	game.fade_and_teleport(0,0,0,5004,473,475)
	return
	
def spiderwoods():
	game.fade_and_teleport(0,0,0,5002,620,525)
	return
def spiderwood():
	spiderwoods()
	return
def spiderpinewoods():
	spiderwoods()
	return
def spiders():
	spiderwoods()
	return
def southernpinewoods():
	spiderwoods()
	return
def spiderqueen():
	game.fade_and_teleport(0,0,0,5002,396,471)
	return

def insectmoundentrance():
	game.fade_and_teleport(0,0,0,5002,511,386)
	return

	
def thorp():
	game.fade_and_teleport(0,0,0,5062,483,526)
def thorpoflordaxer():
	thorp()
def thorpe():
	thorp()

	
#################################
# Raiders						#
# 								#
#################################
def raiders():
	game.fade_and_teleport(0,0,0,5068,598, 457)
	return
def raidercamp():
	game.fade_and_teleport(0,0,0,5068,598, 457)
	return

#################################
# Caves of Chaos				#
# 								#
#################################

def cavesofchaos():
	game.fade_and_teleport(0,0,0,5051,518,582)
	return
def caves():
	game.fade_and_teleport(0,0,0,5051,518,582)
	return	
def coc():
	game.fade_and_teleport(0,0,0,5051,518,582)
	return

def koboldcave():
	game.fade_and_teleport(0,0,0,5052,481,476)
	return
def koboldcaves():
	koboldcave()
	return
def koboldscave():
	koboldcave()
	return
def koboldscaves():
	koboldcave()
	return
def kobolds():
	koboldcave()
	return
def tarkin():
	game.fade_and_teleport(0,0,0,5052,450,446)
	return
	
def orccave():
	game.fade_and_teleport(0,0,0,5053,417,536)
	return
def orccaves():
	orccave()
	return
def orcs():
	orccave()
	return
def orcscave():
	orccave()
	return
def caveb():
	orccave()
	return
def CaveB():
	orccave()
	return
def orcleaderb():
	game.fade_and_teleport(0,0,0,5053,486,451)
	return
	
def bugbearcave():
	game.fade_and_teleport(0,0,0,5058,498,484)
	return
def bugbears():
	bugbearcave()
	return
def bugbearcaves():
	bugbearcave()
	return
def gnollcave():
	game.fade_and_teleport(0,0,0,5060,471,542)
def gnollcaves():
	gnollcave()
def hobgoblincaves():
	game.fade_and_teleport(0,0,0,5056,481,463)
def hobgoblincave():
	hobgoblincaves()
def hobgobcaves():
	hobgoblincaves()
def hobgobcave():
	hobgoblincaves()
def hobgoblinprison():
	game.fade_and_teleport(0,0,0,5056,503,485)
def hobgoblinprisoners():
	hobgoblinprison()
def labyrinth():
	game.fade_and_teleport(0,0,0,5059,509,463)
def labyrinthentrance():
	labyrinth()
def soec():
	game.fade_and_teleport(0,0,0,5061,500,500)
def shrineofevilchaos():
	game.fade_and_teleport(0,0,0,5061,500,500)
def templeofevilchaos():
	game.fade_and_teleport(0,0,0,5061,500,500)
def SoEC():
	game.fade_and_teleport(0,0,0,5061,500,500)

	
def minotaur():
	game.fade_and_teleport(0,0,0,5059,447,509)