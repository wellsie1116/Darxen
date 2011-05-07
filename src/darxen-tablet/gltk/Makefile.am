# Copyright (C) 2011 Kevin Wells <kevin@darxen.org>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AUTOMAKE_OPTIONS=1.7

lib_LTLIBRARIES = libgltk.la
libgltk_la_SOURCES = gltk.h \
			 gltkmarshal.h gltkmarshal.c \
			 gltkstructs.c gltkstructs.h \
		     gltkwindow.c gltkwindow.h \
			 gltkscreen.c gltkscreen.h \
			 gltkminiscreen.c gltkminiscreen.h \
			 gltkdialog.c gltkdialog.h \
			 gltkfonts.c gltkfonts.h \
		     gltkwidget.c gltkwidget.h \
		     gltkspacer.c gltkspacer.h \
			 gltkscrollable.c gltkscrollable.h \
			 gltkbox.c gltkbox.h \
			 gltkhbox.c gltkhbox.h \
			 gltkvbox.c gltkvbox.h \
			 gltktable.c gltktable.h \
			 gltkbin.c gltkbin.h \
			 gltklist.c gltklist.h \
			 gltkspinnermodel.c gltkspinnermodel.h \
			 gltkspinner.c gltkspinner.h \
			 gltklabel.c gltklabel.h \
			 gltkbutton.c gltkbutton.h \
			 gltktogglebutton.c gltktogglebutton.h \
			 gltkkeyboard.c gltkkeyboard.h \
			 gltkentry.c gltkentry.h \
			 gltkslidebutton.c gltkslidebutton.h \
			 gltkconfigbutton.c gltkconfigbutton.h \
			 gltkevents.c gltkevents.h

libgltk_la_LDFLAGS = -g -version-info 1:1:0
libgltk_la_CFLAGS = $(glib_CFLAGS) $(ftgl_CFLAGS) -Wall -O0
libgltk_la_LIBADD = $(glib_LIBS) $(ftgl_LIBS)

gltkmarshal.h: gltkmarshal.list
	glib-genmarshal --header gltkmarshal.list >gltkmarshal.h
	glib-genmarshal --body gltkmarshal.list >gltkmarshal.c

pyexec_LTLIBRARIES = gltk.la
gltk_la_SOURCES = libgltk.la \
 				gltk.c \
 				gltkmodule.c
gltk_la_LDFLAGS = -g -avoid-version -module -shared
gltk_la_CFLAGS = $(pygobject_CFLAGS) $(ftgl_CFLAGS) -I.. -I/usr/include/python2.6 -Wall -fPIC
gltk_la_LIBADD = $(pygobject_LIBS) $(ftgl_LIBS) libgltk.la

gltk-base.defs: libgltk.la gltk-types.defs
	/usr/share/pygobject/2.0/codegen/h2def.py -f gltk-types.defs *.h > gltk-base.defs

gltk.c: gltk.defs gltk-base.defs gltk-types.defs gltk.override
	python /usr/share/pygobject/2.0/codegen/codegen.py \
		--override gltk.override \
		--prefix gltk gltk.defs > gen-gltk.c \
		&& mv gen-gltk.c gltk.c

.PHONY: 

#AM_LDFLAGS = -export-dynamic

SUBDIRS = . examples
EXTRA_DIST = gltkmarshal.list gltk.defs gltk-types.defs gltk.override

