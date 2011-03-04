# Copyright (C) 2008 Kevin Wells <kevin@darxen.org>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


lib_LTLIBRARIES = libdarxen.la
libdarxen_la_SOURCES = libdarxen.h \
	darxenpalettes.c darxenpalettes.h \
	DarxenConversions.c DarxenConversions.h \
	DarxenDataStructs.c DarxenDataStructs.h \
	DarxenParser.c DarxenParser.h \
	DarxenParserLevel3.c DarxenParserLevel3.h \
	libdarxenPoller.c \
	libdarxenRestfulClient.c libdarxenRestfulClient.h \
	libdarxenFileSupport.c libdarxenFileSupport.h \
	libdarxeninit.c libdarxeninit.h \
	libdarxenRenderingCommon.c libdarxenRenderingCommon.h \
	libdarxenShapefileRenderer.c libdarxenShapefileRenderer.h \
	libdarxenShapefiles.c libdarxenShapefiles.h \
	libdarxenXml.c libdarxenXml.h \
	libdarxenRadarSites.c libdarxenRadarSites.h \
	darxenrenderer.c darxenrenderer.h \
	libdarxenCommon.h
libdarxen_la_LDFLAGS = -g -version-info 1:1:0
libdarxen_la_CFLAGS = $(glib_CFLAGS) $(pango_CFLAGS) $(libxml_CFLAGS) $(gsoap_CFLAGS) $(gtkglext_CFLAGS) $(json_CFLAGS)
libdarxen_la_LIBADD = $(glib_LIBS) $(pango_LIBS) $(libxml_LIBS) $(gsoap_LIBS) $(gtkglext_LIBS) $(json_LIBS)

#../shared/libdarxenshared.la

#AM_LDFLAGS = -export-dynamic

EXTRA_DIST = 

bin_PROGRAMS=libdarxen_test
libdarxen_test_SOURCES = libdarxen_test.c \
			 TestRestfulClient.c \
	  		 TestingCommon.c TestingCommon.h \
			 libdarxen.la

libdarxen_test_CFLAGS = -g -Wall $(glib_CFLAGS) $(gobject_CFLAGS) $(libxml_CFLAGS) $(gsoap_CFLAGS) $(curl_CFLAGS) $(json_CFLAGS)
libdarxen_test_LDADD = $(glib_LIBS) $(gobject_LIBS) $(libxml_LIBS) $(gsoap_LIBS) $(curl_LIBS) $(net_LIBS) $(json_LIBS) libdarxen.la -lpthread # -lwsock32
# ../shared/libdarxenshared.la

tests: libdarxen_test
	echo "Running Tests"
	-gtester -k -o testlog.xml ./libdarxen_test
	gtester-report testlog.xml > testlog.html
