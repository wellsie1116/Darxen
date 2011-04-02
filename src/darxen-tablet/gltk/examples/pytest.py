#!/usr/bin/python

import gltk
from gltkdriver import GlutWindowDriver

def create_screen():
    screen = gltk.Screen()
    vbox = gltk.VBox()

    btn1 = gltk.Button("Hello World")
    btn2 = gltk.Button("Button 2")

    vbox.append_widget(btn1, False, False)
    vbox.append_widget(btn2, False, False)

    screen.set_root(vbox)
    return screen


class MyScreen(gltk.Screen):
    def __init__(self):
        vbox = gltk.VBox()

        btn1 = gltk.Button("Hello World")
        btn2 = gltk.Button("Button 2")

        vbox.append_widget(btn1, False, False)
        vbox.append_widget(btn2, False, False)

        self.set_root(vbox)

class MyWindow(GlutWindowDriver):
    def create_window(self):
        window = gltk.Window()

        #self.screen = MyScreen()

        #window.push_screen(self.screen)
        window.push_screen(create_screen())

        return window


def main():
    win = MyWindow()
    win.run()

if __name__ == '__main__':
    main()
