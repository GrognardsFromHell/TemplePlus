from utilities import *

# Editor's Helper Script version 6.0 - (C) 2005-2006 by Agetian
# Batch mod testing commands (C) 2005 by Cerulean the Blue
# Sector reader/writer routines are portions of ToEEWB foreported to Python 2.x
# (C) copyright 2005 by Michael "Agetian" Kamensky

# ==========================================================================
# PLEASE MODIFY TO SUIT YOUR NEEDS:
#    PATH_TO_SECTORS - Default path to sector files (where to load and save)
#    PATH_TO_INTEROP - Default path to the synchronizer (must be a same fol-
#                      der as specified in the ToEE World Builder 2.0.0)
#                      Note that you need a double slash as a path separator
#               here!

PATH_TO_SECTORS = """C:\Sectors"""
PATH_TO_INTEROP = "C:\\"

# ==========================================================================

# Critical constants. Do NOT modify!
TILE_IMPASSABLE = '\x02' + '\x00'*4 + '\xFE\x03' + '\x00'*9
TILE_FLYOVER = '\x02' + '\x00'*5 + '\xFC\x07' + '\x00'*8
TILE_FLYOVER_COVER = '\x02' + '\x00'*5 + '\xFC\x0F' + '\x00'*8
TILE_WATER = '\x24'*9
TILE_NO_WATER = '\x00'*9

def secloc():
    secID = _secloc(location_to_axis(game.party[0].location))
    return str(secID[0])+".sec  (SectorX="+str(secID[1])+", SectorY="+str(secID[2])+")"

def _secloc( coords_tuple ):
    sec_X, sec_Y = coords_tuple[0] / 64, coords_tuple[1] / 64
    return [((sec_Y * 4) << 24) + sec_X, sec_X, sec_Y]

def _createemptysector( sector_file ):
    f = open(sector_file, 'wb')
    f.write('\x00\x00\x00\x00')

    for i in range(0,4096):
        f.write('\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')

    f.write('\x01\x00\x00\x00\x04\x00\xAA')

    for j in range(0,45):
        f.write('\x00')
        
    f.close()

def _world2sector( coords_tuple ):
    sec_X, sec_Y = coords_tuple[0] / 64, coords_tuple[1] / 64
    return ((sec_Y * 4) << 24) + sec_X

def _world2sec_coords( coords_tuple ):
    sec_X, sec_Y = coords_tuple[0] / 64, coords_tuple[1] / 64
    return (sec_X, sec_Y)

def _sec_getminXY( sector_coords_tuple ):
    return (sector_coords_tuple[0]*64, sector_coords_tuple[1]*64)

def _sec_getmaxXY( sector_coords_tuple ):
    return (sector_coords_tuple[0]*64+63, sector_coords_tuple[1]*64+63)

def _sec_getXY( object_coords ):
    min_X, min_Y = _sec_getminXY(_world2sec_coords(object_coords))
    obj_X, obj_Y = object_coords[0]-min_X, object_coords[1]-min_Y
    return (obj_X, obj_Y)

def _sec_gettilepos( object_coords ):
    pos_X, pos_Y = _sec_getXY(object_coords)
    return (pos_Y * 64 + pos_X) * 16

def _sec_writetile( file, tile_data_string, tile_pos ):
    try:
        f = open(file, 'rb')
    except:
        _createemptysector(file)
        f = open(file, 'rb')
    data = f.read()
    f.close()
    tile_idx = data.index('\x01\x00\x00\x00\x04\x00\xAA\x00\x00\x00\x00')-65536
    tile_idx += tile_pos
    data = data[:tile_idx] + tile_data_string + data[tile_idx+16:]    
    f = open(file, 'wb')
    f.write(data)
    f.close()
    
def _sec_getfile():
    return PATH_TO_SECTORS + '\\' + str(_world2sector(location_to_axis(game.party[0].location))) + '.sec'

def _createemptyhsd( hsd_file ):
    f = open(hsd_file, 'wb')
    f.write('\x02\x00\x00\x00')
    for x in range(0, 4096):
        f.write('\x00'*9)
    f.close()

def _hsd_getpos( object_coords ):
    pos_X, pos_Y = _sec_getXY(object_coords)
    return 4 + ((pos_Y * 64 + pos_X) * 9)

def _hsd_writetile( file, hsd_data_string, tile_pos ):
    try:
        f = open(file, 'rb')
    except:
        _createemptyhsd(file)
        f = open(file, 'rb')
    data = f.read()
    f.close()
    data = data[:tile_pos] + hsd_data_string + data[tile_pos+9:]
    f = open(file, 'wb')
    f.write(data)
    f.close()

def _hsd_getfile():
    return PATH_TO_SECTORS + '\\hsd' + str(_world2sector(location_to_axis(game.party[0].location))) + '.hsd'

def _createemptysvb( svb_file ):
    f = open(svb_file, 'wb')
    f.write('\x00'*18432)
    f.close()

def _svb_getfile():
    return PATH_TO_SECTORS + '\\' + str(_world2sector(location_to_axis(game.party[0].location))) + '.svb'

def _svb_getfile_plus11():
    x,y = location_to_axis(game.party[0].location)
    x += 11
    return PATH_TO_SECTORS + '\\' + str(_world2sector((x,y))) + '.svb'

def _svb_getfile_plus22():
    x,y = location_to_axis(game.party[0].location)
    x += 22
    return PATH_TO_SECTORS + '\\' + str(_world2sector((x,y))) + '.svb'

def _svb_getfile_minus11():
    x,y = location_to_axis(game.party[0].location)
    x -= 11
    return PATH_TO_SECTORS + '\\' + str(_world2sector((x,y))) + '.svb'

def _svb_getpos( object_coords ):
    pos_X, pos_Y = _sec_getXY(object_coords)
    return int((pos_Y*64+pos_X)*4.5)

def _svb_writetile( file, pos_X, pos_Y ):
    try:
        f = open(file, 'rb')
    except:
        _createemptysvb(file)
        f = open(file, 'rb')
    data = f.read()
    f.close()
    # CHECK THE TILE DATA
    tloc = _svb_getpos((pos_X,pos_Y))
    type = 0
    if pos_X % 2 == 0:
        # second
        check_loc = ord(data[tloc+4])
        # print check_loc
        type = 1
    else:
        # first
        check_loc = ord(data[tloc])
        # print check_loc
        type = 2
    EXTRA_BYTE = 0
    if type == 1: # first
        #print 'first'
        if check_loc == 0:
            EXTRA_BYTE = 1
        elif check_loc == 1:
            EXTRA_BYTE = 1
        elif check_loc == 17:
            EXTRA_BYTE = 17
        elif check_loc == 16:
            EXTRA_BYTE = 17
        strdef = '\x11'*4 + chr(EXTRA_BYTE)
        #print repr(strdef)
    else: # second
        if check_loc == 0:
            EXTRA_BYTE = 16
        elif check_loc == 1:
            EXTRA_BYTE = 17
        elif check_loc == 16:
            EXTRA_BYTE = 17
        elif check_loc == 17:
            EXTRA_BYTE = 17
        strdef = chr(EXTRA_BYTE) + '\x11'*4
        #print repr(strdef)
    data = data[:tloc]+strdef+data[tloc+5:]
    f = open(file, 'wb')
    f.write(data)
    f.close()

def wtr():
    _hsd_writetile(_hsd_getfile(), TILE_WATER, _hsd_getpos(location_to_axis(game.party[0].location)))
    print(str(location_to_axis(game.party[0].location)) + " was marked WATER")
    return

def lnd():
    _hsd_writetile(_hsd_getfile(), TILE_NO_WATER, _hsd_getpos(location_to_axis(game.party[0].location)))
    print(str(location_to_axis(game.party[0].location)) + " was marked NON-WATER (LAND)")
    return
    
def blk():
    _sec_writetile(_sec_getfile(), TILE_IMPASSABLE, _sec_gettilepos(location_to_axis(game.party[0].location)))
    print(str(location_to_axis(game.party[0].location)) + " was marked IMPASSABLE")
    return

def fly():
    _sec_writetile(_sec_getfile(), TILE_FLYOVER, _sec_gettilepos(location_to_axis(game.party[0].location)))
    print(str(location_to_axis(game.party[0].location)) + " was marked IMPASSABLE, CAN FLY OVER")
    return

def cov():
    _sec_writetile(_sec_getfile(), TILE_FLYOVER_COVER, _sec_gettilepos(location_to_axis(game.party[0].location)))
    print(str(location_to_axis(game.party[0].location)) + " was marked IMPASSABLE, CAN FLY OVER, PROVIDES COVER")
    return

def tp( map, X, Y ):
    game.fade_and_teleport(0,0,0,map,X,Y)
    return RUN_DEFAULT

def tpl( X, Y ):
    game.fade_and_teleport(0,0,0,game.party[0].map,X,Y)
    return RUN_DEFAULT

def loc():
    return location_to_axis(game.party[0].location)

def objs():
    return game.obj_list_vicinity(game.party[0].location, OLC_ALL)
    
def locx( array, index ):
    return location_to_axis(array[index].location)

def inc_x():
    loc_x, loc_y = location_to_axis(game.party[0].location)
    game.fade_and_teleport(0,0,0,game.party[0].map,loc_x+1,loc_y)

def inc_y():
    loc_x, loc_y = location_to_axis(game.party[0].location)
    game.fade_and_teleport(0,0,0,game.party[0].map,loc_x,loc_y+1)

def dec_x():
    loc_x, loc_y = location_to_axis(game.party[0].location)
    game.fade_and_teleport(0,0,0,game.party[0].map,loc_x-1,loc_y)

def dec_y():
    loc_x, loc_y = location_to_axis(game.party[0].location)
    game.fade_and_teleport(0,0,0,game.party[0].map,loc_x,loc_y-1)    

# + paint IMPASSABLE on-off +
def blk_on():
    if game.global_flags[1000] == 1:
        print ("ALREADY PAINTING. PLEASE CANCEL THE PREVIOUS PAINT MODE.")
        return
    game.global_flags[1000] = 1
    print("Painting impassable tiles. Move your character to paint.")
    _blk_on_core()

def _blk_on_core():
    _sec_writetile(_sec_getfile(), TILE_IMPASSABLE, _sec_gettilepos(location_to_axis(game.party[0].location)))
    if game.global_flags[1000] == 1:
        game.timeevent_add(_blk_on_core, (), 10)
    return

def blk_off():
    game.global_flags[1000] = 0
    print("Stopped painting.")
# - paint IMPASSABLE on-off -


# + paint FLYOVER on-off +
def fly_on():
    if game.global_flags[1000] == 1:
        print ("ALREADY PAINTING. PLEASE CANCEL THE PREVIOUS PAINT MODE.")
        return
    game.global_flags[1000] = 1
    print("Painting fly-over tiles. Move your character to paint.")
    _fly_on_core()

def _fly_on_core():
    _sec_writetile(_sec_getfile(), TILE_FLYOVER, _sec_gettilepos(location_to_axis(game.party[0].location)))
    if game.global_flags[1000] == 1:
        game.timeevent_add(_fly_on_core, (), 10)
    return

def fly_off():
    game.global_flags[1000] = 0
    print("Stopped painting.")
# - paint FLYOVER on-off -


# + paint FLYOVER/COVER on-off +
def cov_on():
    if game.global_flags[1000] == 1:
        print ("ALREADY PAINTING. PLEASE CANCEL THE PREVIOUS PAINT MODE.")
        return
    game.global_flags[1000] = 1
    print("Painting fly-over/cover tiles. Move your character to paint.")
    _cov_on_core()

def _cov_on_core():
    _sec_writetile(_sec_getfile(), TILE_FLYOVER_COVER, _sec_gettilepos(location_to_axis(game.party[0].location)))
    if game.global_flags[1000] == 1:
        game.timeevent_add(_cov_on_core, (), 10)
    return

def cov_off():
    game.global_flags[1000] = 0
    print("Stopped painting.")
# - paint FLYOVER/COVER on-off -


# + paint WATER on-off +
def wtr_on():
    if game.global_flags[1000] == 1:
        print ("ALREADY PAINTING. PLEASE CANCEL THE PREVIOUS PAINT MODE.")
        return
    game.global_flags[1000] = 1
    print("Painting water tiles. Move your character to paint.")
    _wtr_on_core()

def _wtr_on_core():
    _hsd_writetile(_hsd_getfile(), TILE_WATER, _hsd_getpos(location_to_axis(game.party[0].location)))
    if game.global_flags[1000] == 1:
        game.timeevent_add(_wtr_on_core, (), 10)
    return

def wtr_off():
    game.global_flags[1000] = 0
    print("Stopped painting.")
# - paint WATER on-off -


# + paint LAND on-off +
def lnd_on():
    if game.global_flags[1000] == 1:
        print ("ALREADY PAINTING. PLEASE CANCEL THE PREVIOUS PAINT MODE.")
        return
    game.global_flags[1000] = 1
    print("Painting land (non-water) tiles. Move your character to paint.")
    _lnd_on_core()

def _lnd_on_core():
    _hsd_writetile(_hsd_getfile(), TILE_NO_WATER, _hsd_getpos(location_to_axis(game.party[0].location)))
    if game.global_flags[1000] == 1:
        game.timeevent_add(_lnd_on_core, (), 10)
    return

def lnd_off():
    game.global_flags[1000] = 0
    print("Stopped painting.")
# - paint LAND on-off -

# + Cerulean's batch commands +
def partyxpset( xp ):  # CB - sets entire groups experience points to xp
    pc = game.leader
    for obj in pc.group_list():
        curxp = obj.stat_level_get(stat_experience)
        newxp = curxp + xp
        obj.stat_base_set(stat_experience, newxp)
    return 1
    
def partylevelset(level):  # CB - sets entire groups xp to minimum necessary for level imputted
    pc = game.leader
    for obj in pc.group_list():
        newxp = (level * (500 * (level -1)))
        obj.stat_base_set(stat_experience, newxp)
    return 1
    
def partyabset (ab, score):  # CB - sets ability to score for entire group
    if (ab == 1):
        abstat = stat_strength
    elif (ab == 2):
        abstat = stat_dexterity
    elif (ab == 3):
        abstat = stat_constitution
    elif (ab == 4):
        abstat = stat_intelligence
    elif (ab == 5):
        abstat = stat_wisdom
    elif (ab == 6):
        abstat = stat_charisma
    else:
        return 0
    if ((score > 0) and (score < 41)):
        pc = game.leader
        for obj in pc.group_list():
            obj.stat_base_set(abstat, score)
    else:
        return 0
    return 1

def massabset(num, score):  # CB - sets all ability scores of specified pc to score
    num = num - 1
    pc = game.party[num]
    if (pc != OBJ_HANDLE_NULL):
        if ((score > 0) and (score < 41)):
            pc.stat_base_set(stat_strength, score)
            pc.stat_base_set(stat_dexterity, score)
            pc.stat_base_set(stat_constitution, score)
            pc.stat_base_set(stat_intelligence, score)
            pc.stat_base_set(stat_wisdom, score)
            pc.stat_base_set(stat_charisma, score)
        else:
            return 0
    else:
        return 0
    return 1
    
def partyhpset(hp):  # CB - sets max hp of entire party to specified value
    pc = game.leader
    for obj in pc.group_list():
        obj.stat_base_set(stat_hp_max, hp)
    return 1
# - Cerulean's batch commands -

# + INTEROPERABILITY LAYER FOR TOEEWB 2.X.X SERIES +

def locobj():  # LOCOBJ - put the current X/Y coords into the WB object editor
    f=open(PATH_TO_INTEROP + "\\wb200_il.lri","w")
    x,y = loc()
    f.write("OBJLOC "+str(x)+" "+str(y))
    f.close()

def locjmp():  # LOCJMP - put the current X/Y/map coords into the WB jump point editor
    f=open(PATH_TO_INTEROP + "\\wb200_il.lri","w")
    x,y = loc()
    amap = game.party[0].map
    f.write("JMPLOC "+str(x)+" "+str(y)+" "+str(amap))
    f.close()

def locday():  # LOCDAY - put the current X/Y/map coords into the WB day/night transition (day)
    f=open(PATH_TO_INTEROP + "\\wb200_il.lri","w")
    x,y = loc()
    amap = game.party[0].map
    f.write("DAYLOC "+str(x)+" "+str(y)+" "+str(amap))
    f.close()

def locngt():  # LOCNGT - put the current X/Y/map coords into the WB day/night transition (night)
    f=open(PATH_TO_INTEROP + "\\wb200_il.lri","w")
    x,y = loc()
    amap = game.party[0].map
    f.write("NGTLOC "+str(x)+" "+str(y)+" "+str(amap))
    f.close()

def pnd():  # PND - put the current X/Y coords into the WB pathnode generator
    f=open(PATH_TO_INTEROP + "\\wb200_il.lri","w")
    x,y = loc()
    f.write("PNDLOC "+str(x)+" "+str(y))
    f.close()

def way():  # WAY - put the current X/Y coords into the WB object waypoint entry
    f=open(PATH_TO_INTEROP + "\\wb200_il.lri","w")
    x,y = loc()
    f.write("WAYLOC "+str(x)+" "+str(y))
    f.close()

def light():  # LIGHT - put the current X/Y/map coords into the WB light editor
    f=open(PATH_TO_INTEROP + "\\wb200_il.lri","w")
    x,y = loc()
    amap = game.party[0].map
    f.write("LGTLOC "+str(x)+" "+str(y)+" "+str(amap))
    f.close()

def standday():  # STANDDAY - put the current X/Y/map coords into the WB day standpoint
    f=open(PATH_TO_INTEROP + "\\wb200_il.lri","w")
    x,y = loc()
    amap = game.party[0].map
    f.write("STDDLOC "+str(x)+" "+str(y)+" "+str(amap))
    f.close()

def standnight():  # STANDNIGHT - put the current X/Y/map coords into the WB night standpoint
    f=open(PATH_TO_INTEROP + "\\wb200_il.lri","w")
    x,y = loc()
    amap = game.party[0].map
    f.write("STDNLOC "+str(x)+" "+str(y)+" "+str(amap))
    f.close()

def standscout():  # STANDSCOUT - put the current X/Y/map coords into the WB scout standpoint
    f=open(PATH_TO_INTEROP + "\\wb200_il.lri","w")
    x,y = loc()
    amap = game.party[0].map
    f.write("STDSLOC "+str(x)+" "+str(y)+" "+str(amap))
    f.close()

def setrot(value): # SETROT - set the game.party[0].rotation
    game.party[0].rotation = value

def locobjr(): # LOCOBJR - return X/Y and the rotation
    f=open(PATH_TO_INTEROP + "\\wb200_il.lri","w")
    x,y = loc()
    a = str(game.party[0].rotation)[0:8]
    f.write("OBJLOCR "+str(x)+" "+str(y)+" "+a)
    f.close()

def wayr():  # WAYR - put the current X/Y coords into the WB object waypoint entry with rotation
    f=open(PATH_TO_INTEROP + "\\wb200_il.lri","w")
    x,y = loc()
    a = str(game.party[0].rotation)[0:8]
    f.write("WAYLOCR "+str(x)+" "+str(y)+" "+a)
    f.close()

# - INTEROPERABILITY LAYER FOR TOEEWB 2.X.X SERIES -