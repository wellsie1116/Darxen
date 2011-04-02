#!/usr/bin/python

import gltk
from gltkdriver import GlutWindowDriver as WindowDriver

from OpenGL.GLU  import *
from OpenGL.GL   import *

class CustomWidget(gltk.Widget):
    __gsignals__ = {"render": "override",
                    "size-request": "override"}

    def do_size_request(self, size):
        #FIXME size is a gpointer
        #print "size request"
        pass

    def do_render(self):
        glColor3f(1.0, 0.0, 0.0)
        glBegin(GL_LINES)
        glVertex2i(0, 0)
        glVertex2i(100, 100)
        glEnd()


class MyScreen(gltk.Screen):
    def __init__(self):
        gltk.Screen.__init__(self)
        vbox = gltk.VBox()

        self.lbl = gltk.Label("This is a label")
        self.lbl.set_draw_border(True)
        btn1 = gltk.Button("Push Me")
        btn2 = gltk.Button("Close")

        vbox.append_widget(self.lbl, False, False)
        vbox.append_widget(btn1, False, False)
        vbox.append_widget(btn2, False, False)
        vbox.append_widget(CustomWidget(), True, True)

        btn1.connect("click-event", self.btn1_clicked)
        btn2.connect("click-event", self.btn2_clicked)

        self.set_root(vbox)

    def btn1_clicked(self, btn, data):
        self.lbl.set_text("Thanks")
    
    def btn2_clicked(self, btn, data):
        self.lbl.set_text("Goodbye")
        self.close_window()

class MyWindow(WindowDriver):
    def create_window(self):
        window = gltk.Window()

        self.screen = MyScreen()

        window.push_screen(self.screen)

        return window

def main():
    win = MyWindow()
    win.run()

if __name__ == '__main__':
    main()
