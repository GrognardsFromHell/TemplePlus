from toee import *
import tpgui

w=tpgui._add_root_container("pgwnd", 256,256)

a = tpgui._get_container("pgwnd")
a.x = 50; a.y = 50

# Add background image
w_img = a.add_image("art/splash/legal0322_1_1.tga")

# Add title text from mesline
title_txt = game.get_mesline("mes\\pc_creation.mes", 18013)
w_txt = a.add_text(title_txt); w_txt.y = 10
w_txt.set_style_by_id("priory-title") # get style from text_styles.json
#w_txt.style.point_size = 20 # this does not work on predefined fonts

# Add freeform text
w_txt2 = a.add_text("Body text"); w_txt2.y = 20
w_txt2.style.point_size = 18


b = tpgui._add_button("pgwnd", "btn1")
b.set_text("CLICK ME!"); b.x = 50; b.y = 40
b.set_style_id("chargen-button")


def butclick():
    # b = tpgui._get_button("btn1")
    b.x += 30
    b.x = b.x % 230
    return True

b.set_click_handler( butclick )


b2 = tpgui._add_button("pgwnd", "btn2")


def b2cb(): # updates the title text
    print(game.leader)
    w_txt.set_text(str(game.leader))
    return

b2.set_text("title"); b2.x = 50; b2.y = 70
b2.set_style_id("chargen-button")
b2.set_click_handler( b2cb )

c = tpgui._add_button("pgwnd", "shit_close_btn")
c.set_text("Close"); c.x = 200; c.y = 236
c.set_style_id("accept-button")
c.set_click_handler( lambda: a.hide() )
