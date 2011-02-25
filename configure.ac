# Copyright (C) 2008 Kevin Wells <kevin@darxen.org>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


## Initializations ##
AC_INIT([darxen],
        [0.0.1],
        [Kevin Wells kevin@darxen.org],
        [darxen])
AC_PREREQ(2.61)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src/darxend/darxend.c)
AC_CONFIG_MACRO_DIR(m4)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE([-Wall dist-bzip2])


## Check arguments ##
AC_ARG_ENABLE(darxen-cli,
              AC_HELP_STRING([--disable-darxencli], [do not include darxen-cli in build]),
              [BUILD_DARXENCLI=$enableval],
              [BUILD_DARXENCLI=yes])

AC_ARG_ENABLE(darxen,
              AC_HELP_STRING([--disable-darxen], [do not include darxen (GUI) in build]),
              [BUILD_DARXEN=$enableval],
              [BUILD_DARXEN=yes])


## Checks for programs ##
AC_PROG_CC
AM_PROG_CC_C_O
#AC_PROG_CXX
AC_PROG_INSTALL
#AC_ PROG_ RANLIB
AC_AIX
#AC_MINIX
AC_ISC_POSIX
LT_INIT([disable-shared])
# check for gtk-doc
GTK_DOC_CHECK([1.14],[--flavour no-tmpl])

## Checks for libraries ##

AC_CHECK_LIB([shp], [SHPOpen],
  [libshp_LIBS=-lshp],
  AC_CHECK_LIB([shapelib], [SHPOpen],
    [libshp_LIBS=-lshapelib],
    [AC_MSG_WARN([Package requirements (libshp) were not met])]))
AC_SUBST([libshp_LIBS])

#OpenGL
AC_CHECK_HEADERS([GL/gl.h GL/glu.h], [HAVE_GL=yes], [HAVE_GL=no])
if test x$HAVE_GL = xyes; then
  GL_LIBS='-lGL -lGLU'
  GL_CFLAGS=
else
  AC_MSG_ERROR([OpenGL development libraries not found])
fi
AC_SUBST([GL_LIBS])
AC_SUBST([GL_CFLAGS])


## Checks for header files ##
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netinet/in.h stdlib.h string.h sys/socket.h])
AC_CHECK_HEADERS(shapefil.h)

## Checks for typedefs, structures, and compiler characteristics ##
AC_C_CONST

## Checks for library functions ##
AC_FUNC_MALLOC
AC_FUNC_STRTOD
#AC_CHECK_FUNCS([socket])

PKG_CHECK_MODULES(ftgl, ftgl >= 2.1.3)
PKG_CHECK_MODULES(glib, glib-2.0 >= 2.0.0)
PKG_CHECK_MODULES(gobject, gobject-2.0 >= 2.0.0)
PKG_CHECK_MODULES(pango, pango >= 1.0.0)
PKG_CHECK_MODULES(gtk, gtk+-2.0 >= 2.16.0)
PKG_CHECK_MODULES(libglade, libglade-2.0 >= 2.0.0)
PKG_CHECK_MODULES(gtkglext, gtkglext-1.0 >= 1.0.5)
PKG_CHECK_MODULES(libxml, libxml-2.0 >= 2.6.27)
PKG_CHECK_MODULES(gsoappp, gsoap++ >= 2.7)
PKG_CHECK_MODULES(gsoap, gsoap >= 2.7)
PKG_CHECK_MODULES(libgrip, libgrip-0.1 >= 0.1.2)

PKG_CHECK_MODULES(curl, libcurl >= 7.18)

#net_LIBS='-lwsock32'
net_LIBS=
AC_SUBST(net_LIBS)

#FIXME
#curl_CFLAGS=
#curl_LIBS='-lcurl'
#'-lcurl -Wl,-Bsymbolic-functions -lidn -lldap -lrt -L/usr/lib -g -O2 -Wl,-Bsymbolic-functions -lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err -lgssapi_krb5 -lz -lgnutls'
#AC_SUBST(curl_CFLAGS)
#AC_SUBST(curl_LIBS)


## Output ##
AC_CONFIG_FILES([
   Makefile
   README
   doc/Makefile
   doc/reference/gltk/Makefile
   m4/Makefile
   src/Makefile
   src/darxend/Makefile
   src/libdarxen/Makefile
   src/darxen-cli/Makefile
   src/darxen-cgi/Makefile
   src/darxen/Makefile
   src/darxen-tablet/Makefile
   src/darxen-tablet/gltk/Makefile
   src/soap/Makefile
])

AC_OUTPUT
