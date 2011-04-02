#!/usr/bin/python

import gltk

from OpenGL.GLUT import *
from OpenGL.GLU  import *
from OpenGL.GL   import *
import sys

class WindowDriver:
    def __init__(self):
        self.initialize_gui()
        self.window = self.create_window()
        self.window.connect("request-render", self.request_render)
        self.window.connect("close", self.close)

    def expose(self):
        glClear(GL_COLOR_BUFFER_BIT)
        glLoadIdentity()

        self.window.render()

    def configure(self, width, height):
        self.window.set_size(width, height)

        glViewport(0, 0, width, height)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluOrtho2D(0.0, width, height, 0.0)

        glMatrixMode(GL_MODELVIEW)

        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

    def mouse(self, state, x, y):
        e = gltk.Event(gltk.TOUCH)
        e.id = -1
        e.touchType = gltk.TOUCH_BEGIN if state == GLUT_DOWN else gltk.TOUCH_END
        e.positions = ((x, y),)
        self.window.send_event(e)

    def motion(self, x, y):
        e = gltk.Event(gltk.TOUCH)
        e.id = -1
        e.touchType = gltk.TOUCH_MOVE
        e.positions = ((x, y),)
        self.window.send_event(e)

    def create_window(self):
        raise NotImplemented

    def run(self):
        raise NotImplemented

    def request_render(self, data):
        raise NotImplemented

    def close(self, data):
        raise NotImplemented

#class GtkWindowDriver(WindowDriver):
#    def initialize_gui(self):
#        #setup our window
#        self.window = gtk.Window(gkt.WINDOW_TOPLEVEL)
#        self.window.connect("destroy", self.destroy)
#        #self.window.set_title("asdf")
#        self.window.set_default_size(800, 600)
#
#        #add our only widget
#        self.darea = gtk.DrawingArea()

#FIXME: We need a glib main loop!

class GlutWindowDriver(WindowDriver):
    def initialize_gui(self):
        glutInit(sys.argv)
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
        glutInitWindowSize(800, 600)
        glutCreateWindow("Example")

        glutDisplayFunc(self._display)
        glutReshapeFunc(self._reshape)
        glutMouseFunc(self._mouse)
        glutMotionFunc(self._motion)
        #glutIdleFunc(self._idle)

    def _display(self):
        self.expose()
        glutSwapBuffers()

    def _reshape(self, width, height):
        self.configure(width, height)

    def _mouse(self, button, state, x, y):
        self.mouse(state, x, y)

    def _motion(self, x, y):
        self.motion(x, y)

    def _idle(self):
        pass
    
    def run(self):
        glutMainLoop()

    def request_render(self, data):
        glutPostRedisplay()

    def close(self, data):
        glutDestroyWindow(glutGetWindow())
 
