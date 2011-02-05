/*
 * libdarxenCommon.h
 *
 *  Created on: Mar 2, 2010
 *      Author: wellska1
 */

#ifndef __LIBDARXENCOMMON_H__
#define __LIBDARXENCOMMON_H__

#include <math.h>
#include <time.h>

#include <glib/gtypes.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef G_OS_WIN32
#define G_EXPORT __declspec(dllexport)
#else
#define G_EXPORT
#endif

typedef float DarxenColor[4];
#define DARXEN_COLOR_COPY(from, to) to[0] = from[0]; to[1] = from[1]; to[2] = from[2]; to[3] = from[3];

#define DARXEN_DEG_TO_RAD(deg) (((deg)/180.0)*G_PI)
#define DARXEN_RAD_TO_DEG(rad) (((rad)/G_PI)*180.0)

#define degcos(deg) cos(DARXEN_DEG_TO_RAD(deg))
#define degsin(deg) sin(DARXEN_DEG_TO_RAD(deg))
#define degacos(deg) DARXEN_RAD_TO_DEG(acos(deg))

#ifdef G_OS_WIN32

#define nanosleep(req, rem) pthread_delay_np(req)

#endif



//gchar* darxen_temp(gchar* val);

#endif /* __LIBDARXENCOMMON_H__ */
