# Copyright (C) 2008 Kevin Wells <kevin@darxen.org>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

bin_PROGRAMS=darxencgi
darxencgi_SOURCES= request.c \
	darxencgi.c \
	../libdarxen/libdarxen.la
darxencgi_CFLAGS = -Wall -I../libdarxen $(gobject_CFLAGS) $(glib_CFLAGS) $(gsoap_CFLAGS)
darxencgi_LDADD = $(gobject_LIBS) $(glib_LIBS) $(gsoap_LIBS) ../libdarxen/libdarxen.la -lpthread # -lwsock32
# ../libdarxen/libdarxen.la

#AM_LDFLAGS = -export-dynamic

EXTRA_DIST = 

