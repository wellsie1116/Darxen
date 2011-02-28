# Copyright (C) 2011 Kevin Wells <kevin@darxen.org>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


lib_LTLIBRARIES = libgltk.la
libgltk_la_SOURCES = gltk.h \
			 gltkmarshal.c gltkmarshal.h \
		     gltkwindow.c gltkwindow.h \
			 gltkfonts.c gltkfonts.h \
		     gltkwidget.c gltkwidget.h \
			 gltkscrollable.c gltkscrollable.h \
			 gltkbox.c gltkbox.h \
			 gltkhbox.c gltkhbox.h \
			 gltkvbox.c gltkvbox.h \
			 gltkbin.c gltkbin.h \
			 gltklist.c gltklist.h \
			 gltklabel.c gltklabel.h \
			 gltkbutton.c gltkbutton.h \
			 gltkevents.c gltkevents.h

libgltk_la_LDFLAGS = -g -version-info 1:1:0
libgltk_la_CFLAGS = $(glib_CFLAGS) $(ftgl_CFLAGS) -Wall -O0
libgltk_la_LIBADD = $(glib_LIBS) $(ftgl_LIBS)

#AM_LDFLAGS = -export-dynamic

EXTRA_DIST = 

