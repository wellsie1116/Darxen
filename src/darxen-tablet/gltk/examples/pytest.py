#!/usr/bin/python

import gltk
from gltkdriver import GlutWindowDriver as WindowDriver

class MyScreen(gltk.Screen):
    def __init__(self):
        gltk.Screen.__init__(self)
        vbox = gltk.VBox()

        self.lbl = gltk.Label("This is a label")
        btn1 = gltk.Button("Hello World")
        btn2 = gltk.Button("Button 2")

        vbox.append_widget(self.lbl, False, False)
        vbox.append_widget(btn1, False, False)
        vbox.append_widget(btn2, False, False)

        btn1.connect("click-event", self.btn1_clicked)

        self.set_root(vbox)

    def btn1_clicked(self, btn, data):
        self.lbl.set_text("Hi again")

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
