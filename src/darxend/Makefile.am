# Copyright (C) 2008 Kevin Wells <kevin@darxen.org>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

#bindir=..
bin_PROGRAMS=darxend
darxend_SOURCES = darxendWebServiceImp.c \
	restapi.c restapi.h \
	Settings.c Settings.h \
	FtpDirList.c FtpDirList.h \
	Callbacks.c Callbacks.h \
	FtpConnection.c FtpConnection.h \
	RadarDataManager.c RadarDataManager.h \
	ClientManager.c ClientManager.h \
	Client.c Client.h \
	../soap/server/soapC.c \
	../soap/server/soapServer.c \
	soap.h \
	darxend.c

darxend_CFLAGS = -Wall $(glib_CFLAGS) $(gobject_CFLAGS) $(libxml_CFLAGS) $(gsoap_CFLAGS) $(curl_CFLAGS)
darxend_LDADD = $(glib_LIBS) $(gobject_LIBS) $(libxml_LIBS) $(gsoap_LIBS) $(curl_LIBS) $(net_LIBS) -lpthread -lmicrohttpd # -lwsock32
# ../shared/libdarxenshared.la

#AM_LDFLAGS = -export-dynamic

EXTRA_DIST = 

