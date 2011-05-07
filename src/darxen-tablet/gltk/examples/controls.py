#!/usr/bin/python

import gltk
from gltkdriver import GlutWindowDriver as WindowDriver

from OpenGL.GLU  import *
from OpenGL.GL   import *

class MyScreen(gltk.Screen):
    def __init__(self):
        gltk.Screen.__init__(self)

        self.vbox = gltk.VBox(5)

        model = gltk.SpinnerModel(1)
        for i in range(10):
            model.add_toplevel(str(i), "Item %d" % (i))

        lbl = gltk.Label("I am a label")
        lbl.set_draw_border(True)

        spinner = gltk.Spinner(model)

        entry = gltk.Entry("Entry widget")

        btn = gltk.Button("Close")

        self.append(lbl)
        self.append(spinner)
        self.append(entry)
        self.append(btn)

        btn.connect("click-event", self.btn_clicked)

        self.set_root(self.vbox)

    def append(self, widget):
        hbox = gltk.HBox(0)
        hbox.append_widget(widget, False, False)
        self.vbox.append_widget(hbox, False, False)

    def btn_clicked(self, btn, data):
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
