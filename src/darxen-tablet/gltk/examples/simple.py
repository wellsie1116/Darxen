#!/usr/bin/python

import gltk
from gltkdriver import GlutWindowDriver as WindowDriver

class MyWindow(WindowDriver):
    def create_window(self):
        window = gltk.Window()
        screen = gltk.Screen()
        btn = gltk.Button("Close")
        btn.connect("click-event", lambda btn,data: screen.close_window())
        screen.set_root(btn)
        window.push_screen(screen)
        return window

def main():
    win = MyWindow()
    win.run()

if __name__ == '__main__':
    main()
