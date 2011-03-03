# Copyright (C) 20118 Kevin Wells <kevin@darxen.org>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


bin_PROGRAMS=darxen-tablet
darxen_tablet_SOURCES = darxen-tablet.c darxen-tablet.h \
						darxengui.c \
						darxenconfig.c darxenconfig.h \
						darxenmainview.c darxenmainview.h \
						darxenview.c darxenview.h \
						darxenpanelmanager.c darxenpanelmanager.h \
						darxensitelist.c darxensitelist.h \
						../soap/client/soapC.c \
						../soap/client/soapClient.c \
						../libdarxen/libdarxen.la

darxen_tablet_CFLAGS = -Wall -g -I../libdarxen $(gtkglext_CFLAGS) $(libgrip_CFLAGS) $(gsoap_CFLAGS) -O0
darxen_tablet_LDADD = $(gtkglext_LIBS) $(libgrip_LIBS)  $(gsoap_LIBS) ../libdarxen/libdarxen.la gltk/libgltk.la -lpthread $(libshp_LIBS) -lshp # -lwsock32
darxen_tablet_LDFLAGS = -g

AM_LDFLAGS = -export-dynamic

SUBDIRS = gltk
EXTRA_DIST = 

