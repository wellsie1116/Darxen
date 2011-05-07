#!/usr/bin/python

import gltk
from gltkdriver import GlutWindowDriver as WindowDriver

from OpenGL.GLU  import *
from OpenGL.GL   import *

class MyScreen(gltk.Screen):
    def __init__(self):
        gltk.Screen.__init__(self)

        vbox = gltk.VBox()

        model = gltk.SpinnerModel(1)
        for i in range(10):
            model.add_toplevel(str(i), "Item %d" % (i))



        self.lbl = gltk.Label("Spinner Test")
        self.lbl.set_draw_border(True)
        spinner = gltk.Spinner(model)
        btn2 = gltk.Button("Close")

        vbox.append_widget(self.lbl, False, False)
        vbox.append_widget(btn2, False, False)
        vbox.append_widget(spinner, False, False)

        btn2.connect("click-event", self.btn2_clicked)

        self.set_root(vbox)

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
