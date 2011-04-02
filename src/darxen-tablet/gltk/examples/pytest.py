#!/usr/bin/python

import gltk
from gltkdriver import GlutWindowDriver as WindowDriver

class MyScreen(gltk.Screen):
    def __init__(self):
        gltk.Screen.__init__(self)
        vbox = gltk.VBox()

        lbl = gltk.Label("This is a label")
        btn1 = gltk.Button("Hello World")
        btn2 = gltk.Button("Button 2")

        vbox.append_widget(lbl, False, False)
        vbox.append_widget(btn1, False, False)
        vbox.append_widget(btn2, False, False)

        self.set_root(vbox)

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
