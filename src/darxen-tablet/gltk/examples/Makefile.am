# Copyright (C) 2011 Kevin Wells <kevin@darxen.org>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

bin_PROGRAMS=labels buttons lists scrollable

labels_SOURCES = labels.c common.c common.h ../libgltk.la
labels_CFLAGS = -Wall -g -I../../ $(json_LIBS) $(gtkglext_CFLAGS) $(libgrip_CFLAGS) -O0
labels_LDADD = $(json_LIBS) $(gtkglext_LIBS) $(libgrip_LIBS) ../libgltk.la 
labels_LDFLAGS = -g

buttons_SOURCES = buttons.c common.c common.h ../libgltk.la
buttons_CFLAGS = -Wall -g -I../../ $(json_LIBS) $(gtkglext_CFLAGS) $(libgrip_CFLAGS) -O0
buttons_LDADD = $(json_LIBS) $(gtkglext_LIBS) $(libgrip_LIBS) ../libgltk.la 
buttons_LDFLAGS = -g

lists_SOURCES = lists.c common.c common.h ../libgltk.la
lists_CFLAGS = -Wall -g -I../../ $(json_LIBS) $(gtkglext_CFLAGS) $(libgrip_CFLAGS) -O0
lists_LDADD = $(json_LIBS) $(gtkglext_LIBS) $(libgrip_LIBS) ../libgltk.la 
lists_LDFLAGS = -g

scrollable_SOURCES = scrollable.c common.c common.h ../libgltk.la
scrollable_CFLAGS = -Wall -g -I../../ $(json_LIBS) $(gtkglext_CFLAGS) $(libgrip_CFLAGS) -O0
scrollable_LDADD = $(json_LIBS) $(gtkglext_LIBS) $(libgrip_LIBS) ../libgltk.la 
scrollable_LDFLAGS = -g

AM_LDFLAGS = -export-dynamic

SUBDIRS = 
EXTRA_DIST = 

