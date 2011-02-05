/*
 * libdarxenXml.h
 *
 *  Created on: Mar 6, 2010
 *      Author: wellska1
 */

#ifndef LIBDARXENXML_H_
#define LIBDARXENXML_H_

#include "libdarxenCommon.h"

#include <stdarg.h>

#include <libxml/xpath.h>

#include <glib/gdate.h>
#include <glib/gslist.h>

G_EXPORT void			darxen_xml_debug				(xmlXPathContext* xpath, const char* expr);

G_EXPORT gboolean		darxen_xml_try_get_boolean		(xmlXPathContext* xpath, gboolean defaultValue, const char* expr);
G_EXPORT double			darxen_xml_try_get_number		(xmlXPathContext* xpath, double defaultValue, const char* expr);
G_EXPORT int			darxen_xml_try_get_int			(xmlXPathContext* xpath, int defaultValue, const char* expr);
G_EXPORT char*			darxen_xml_try_get_string		(xmlXPathContext* xpath, const char* defaultValue, const char* expr);

G_EXPORT gboolean		darxen_xml_get_boolean			(xmlXPathContext* xpath, const char* expr);
G_EXPORT double			darxen_xml_get_number			(xmlXPathContext* xpath, const char* expr);
G_EXPORT int			darxen_xml_get_int				(xmlXPathContext* xpath, const char* expr);
G_EXPORT char*			darxen_xml_get_string			(xmlXPathContext* xpath, const char* expr);

G_EXPORT xmlNodeSet*	darxen_xml_get_nodes			(xmlXPathContext* xpath, const char* expr);
G_EXPORT char**			darxen_xml_get_strs				(xmlXPathContext* xpath, const char* expr, int* count);

G_EXPORT gboolean		darxen_xml_get_color			(xmlXPathContext* xpath, DarxenColor color, const char* expr);
G_EXPORT GDate*			darxen_xml_get_date				(xmlXPathContext* xpath, const char* expr);
G_EXPORT int			darxen_xml_get_time				(xmlXPathContext* xpath, const char* expr);

G_EXPORT gchar*			darxen_xml_path_format			(char* path, ...);


#endif /* LIBDARXENXML_H_ */
