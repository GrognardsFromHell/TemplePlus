import tpgui

w=tpgui._add_root_container("shiat", 256,256)
a = tpgui._get_container("shiat")
a.add_image("art/splash/legal0322_1_1.tga"); a.x = 50; a.y = 50


b = tpgui._add_button("shiat", "btn1")
b.set_text("CLICK ME!"); b.x = 50; b.y = 30
b.set_style_id("chargen-button")


def butclick():
    # b = tpgui._get_button("btn1")
    b.x += 30
    return

b.set_click_handler( butclick )


c = tpgui._add_button("shiat", "shit_close_btn")
c.set_text("Close"); c.x = 200; c.y = 236
c.set_style_id("accept-button")
c.set_click_handler( lambda: a.hide() )
