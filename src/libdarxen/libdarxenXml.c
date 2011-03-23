/*
 * libdarxenXml.c
 *
 *  Created on: Mar 6, 2010
 *      Author: wellska1
 */

#include "libdarxenXml.h"

#include <glib.h>

#include <string.h>

static xmlXPathObject* evalXPath(xmlXPathContext* xpath, const char* expr);

static char*	get_xpath_str(xmlXPathObject* xres);
static int		get_xpath_int(xmlXPathObject* xres);
static double	get_xpath_num(xmlXPathObject* xres);
static gboolean	get_xpath_bool(xmlXPathObject* xres);


/********************
 * Public Functions *
 ********************/

void
darxen_xml_debug(xmlXPathContext* xpath, const char* expr)
{
	printf("Eval XPath expression: %s\n", expr);

	xmlXPathObject* xres;
	xres = evalXPath(xpath, expr);
	int i;

	switch (xres->type)
	{
	case XPATH_NODESET:
		if (xres->nodesetval)
		{
			printf("Type: Nodeset (%i nodes)\n", xres->nodesetval->nodeNr);
			for (i = 0; i < xres->nodesetval->nodeNr; i++)
			{
				xmlNode* node = xres->nodesetval->nodeTab[i];
				printf("Node %i (type %i):", i + 1, node->type);
				xmlBuffer* buf = xmlBufferCreate();
				xmlNodeDump(buf, node->doc, node, 1, 1);
				xmlBufferDump(stdout, buf);
				xmlBufferFree(buf);
				printf("\n");
			}
		}
		else
		{
			printf("Type: Nodeset (null!!!)\n");
			xmlError* err = xmlGetLastError();
			if (err)
				printf("Error message: %s\n", err->message);
		}
		break;
	case XPATH_BOOLEAN:
		printf("Type: Boolean\n");
		printf("Value: %i\n", xmlXPathCastToBoolean(xres));
		break;
	case XPATH_NUMBER:
		printf("Type: Number\n");
		printf("Value: %f\n", xmlXPathCastToNumber(xres));
		break;
	case XPATH_STRING:
	{
		xmlChar* res = xmlXPathCastToString(xres);
		printf("Type: String\n");
		printf("Value: %s\n", res);
		xmlFree(res);
	} break;
	default:
		printf("Type: unknown\n");
		break;
	}
	printf("\n");
	fflush(stdout);

	xmlXPathFreeObject(xres);
}

gboolean
darxen_xml_try_get_boolean(xmlXPathContext* xpath, gboolean defaultValue, const char* expr)
{
	xmlXPathObject* xres;
	xres = evalXPath(xpath, expr);

	if (!xres || (xres->type == XPATH_NODESET && (!xres->nodesetval || !xres->nodesetval->nodeNr)))
		return defaultValue;
	return get_xpath_bool(xres);
}

double
darxen_xml_try_get_number(xmlXPathContext* xpath, double defaultValue, const char* expr)
{
	xmlXPathObject* xres;
	xres = evalXPath(xpath, expr);

	if (!xres || (xres->type == XPATH_NODESET && (!xres->nodesetval || !xres->nodesetval->nodeNr)))
		return defaultValue;
	return get_xpath_num(xres);
}

int
darxen_xml_try_get_int(xmlXPathContext* xpath, int defaultValue, const char* expr)
{
	xmlXPathObject* xres;
	xres = evalXPath(xpath, expr);

	if (!xres || (xres->type == XPATH_NODESET && (!xres->nodesetval || !xres->nodesetval->nodeNr)))
		return defaultValue;
	return get_xpath_int(xres);
}

/**
 * Note: the returned string must be freed, regardless of whether or not the default value was used
 */
char*
darxen_xml_try_get_string(xmlXPathContext* xpath, const char* defaultValue, const char* expr)
{
	xmlXPathObject* xres;
	xres = evalXPath(xpath, expr);

	if (!xres || (xres->type == XPATH_NODESET && (!xres->nodesetval || !xres->nodesetval->nodeNr)))
	{
		if (!defaultValue)
			return NULL;
		return strdup(defaultValue);
	}

	return get_xpath_str(xres);
}

gboolean
darxen_xml_get_boolean(xmlXPathContext* xpath, const char* expr)
{
	xmlXPathObject* xres;
	xres = evalXPath(xpath, expr);

	return get_xpath_bool(xres);
}

double
darxen_xml_get_number(xmlXPathContext* xpath, const char* expr)
{
	xmlXPathObject* xres;
	xres = evalXPath(xpath, expr);

	return get_xpath_num(xres);
}

inline int
darxen_xml_get_int(xmlXPathContext* xpath, const char* expr)
{
	return (int)darxen_xml_get_number(xpath, expr);
}

char*
darxen_xml_get_string(xmlXPathContext* xpath, const char* expr)
{
	xmlXPathObject* xres;
	xres = evalXPath(xpath, expr);

	return get_xpath_str(xres);
}


xmlNodeSet*
darxen_xml_get_nodes(xmlXPathContext* xpath, const char* expr)
{
	xmlXPathObject* xres;
	xres = evalXPath(xpath, expr);
	g_assert(xres->type == XPATH_NODESET);

	xmlNodeSet* res = xres->nodesetval;

	xmlXPathFreeNodeSetList(xres);

	return res;
}

char**
darxen_xml_get_strs(xmlXPathContext* xpath, const char* expr, int* count)
{
	xmlXPathObject* xres;
	xres = evalXPath(xpath, expr);

	g_assert(xres->type == XPATH_NODESET && xres->nodesetval);

	if (count)
		*count = xres->nodesetval->nodeNr;
	char** res = calloc(xres->nodesetval->nodeNr, sizeof(char*));

	int i;
	for (i = 0; i < xres->nodesetval->nodeNr; i++)
	{
		xmlNode* node = xres->nodesetval->nodeTab[i];
		g_assert(node->type == XML_TEXT_NODE || node->type == XML_ELEMENT_NODE || node->type == XML_ATTRIBUTE_NODE);
		xmlChar* content = xmlNodeGetContent(node);
		if (!content)
			g_error("invalid xpath result");
		res[i] = strdup((char*)content);
		xmlFree(content);
	}

	xmlXPathFreeObject(xres);
	return res;
}

gboolean
darxen_xml_get_color(xmlXPathContext* xpath, DarxenColor color, const char* expr)
{
	xmlNodeSet* nodes = darxen_xml_get_nodes(xpath, expr);

	if (nodes->nodeNr != 1)
	{
		xmlXPathFreeNodeSet(nodes);
		return FALSE;
	}

	xmlNode* node = nodes->nodeTab[0];
	node = node->children;

	int r = -1;
	int g = -1;
	int b = -1;
	int a = 255;

	while (node)
	{
		if (!strcmp((char*)node->name, "R"))
		{
			r = atoi((char*)node->children->content);
		}
		else if (!strcmp((char*)node->name, "G"))
		{
			g = atoi((char*)node->children->content);
		}
		else if (!strcmp((char*)node->name, "B"))
		{
			b = atoi((char*)node->children->content);
		}
		else if (!strcmp((char*)node->name, "A"))
		{
			a = atoi((char*)node->children->content);
		}

		node = node->next;
	}

	color[0] = r / 255.0;
	color[1] = g / 255.0;
	color[2] = b / 255.0;
	color[3] = a / 255.0;

	xmlXPathFreeNodeSet(nodes);

	return (r > -1) && (g > -1) && (b > -1);
}

GDate*
darxen_xml_get_date(xmlXPathContext* xpath, const char* expr)
{
	xmlNodeSet* nodes = darxen_xml_get_nodes(xpath, expr);

	g_assert(nodes->nodeNr == 1);

	xmlNode* node = nodes->nodeTab[0];
	node = node->children;

	int day = -1;
	int month = -1;
	int year = -1;

	while (node)
	{
		if (!strcmp((char*)node->name, "Day"))
		{
			day = atoi((char*)node->children->content);
		}
		else if (!strcmp((char*)node->name, "Month"))
		{
			month = atoi((char*)node->children->content);
		}
		else if (!strcmp((char*)node->name, "Year"))
		{
			year = atoi((char*)node->children->content);
		}
		node = node->next;
	}

	xmlXPathFreeNodeSet(nodes);

	if (day == -1 || month == -1 || year == -1)
		return NULL;

	return g_date_new_dmy(day, month, year);
}

int
darxen_xml_get_time(xmlXPathContext* xpath, const char* expr)
{
	xmlNodeSet* nodes = darxen_xml_get_nodes(xpath, expr);

	g_assert(nodes->nodeNr == 1);

	xmlNode* node = nodes->nodeTab[0];
	node = node->children;

	int hour = -1;
	int minute = -1;

	while (node)
	{
		if (!strcmp((char*)node->name, "H"))
		{
			hour = atoi((char*)node->children->content);
		}
		else if (!strcmp((char*)node->name, "M"))
		{
			minute = atoi((char*)node->children->content);
		}
		node = node->next;
	}

	xmlXPathFreeNodeSet(nodes);

	if (hour == -1 || minute == -1)
		return -1;

	return hour * (60 * 60) + minute * 60;
}

gchar*
darxen_xml_path_format(char* path, ...)
{
	va_list args;
	va_start(args, path);
	gchar* expr = g_strdup_vprintf(path, args);
	va_end(args);

	return expr;
}

/*********************
 * Private Functions *
 *********************/

static xmlXPathObject*
evalXPath(xmlXPathContext* xpath, const char* expr)
{
	xmlXPathObject* res;
	res = xmlXPathEvalExpression((xmlChar*)expr, xpath);

	if (!res)
	{
		g_error("XPath operation failed: %s", expr);
	}

	return res;
}

char*
get_xpath_str(xmlXPathObject* xres)
{
	char* res;

	if (xres->type == XPATH_STRING)
	{
		xmlChar* content = xmlXPathCastToString(xres);
		res = strdup((char*)content);
		xmlFree(content);
	}
	else if (xres->type == XPATH_NODESET && xres->nodesetval && xres->nodesetval->nodeNr == 1)
	{
		xmlNode* node = xres->nodesetval->nodeTab[0];
		g_assert(node->type == XML_TEXT_NODE || node->type == XML_ELEMENT_NODE || node->type == XML_ATTRIBUTE_NODE);
		xmlChar* content = xmlNodeGetContent(node);
		if (!content)
			g_error("invalid xpath result");
		res = strdup((char*)content);
		xmlFree(content);
	}
	else
	{
		g_error("Invalid xpath expression in get_string");
	}

	xmlXPathFreeObject(xres);
	return res;
}

int
get_xpath_int(xmlXPathObject* xres)
{
	return (int)get_xpath_num(xres);
}

double
get_xpath_num(xmlXPathObject* xres)
{
	double res;

	if (xres->type == XPATH_NUMBER)
	{
		res = xmlXPathCastToNumber(xres);
	}
	else if (xres->type == XPATH_NODESET && xres->nodesetval && xres->nodesetval->nodeNr == 1)
	{
		xmlNode* node = xres->nodesetval->nodeTab[0];
		g_assert(node->type == XML_TEXT_NODE || node->type == XML_ELEMENT_NODE || node->type == XML_ATTRIBUTE_NODE || node->type == XML_TEXT_NODE);
		xmlChar* content = xmlNodeGetContent(node);
		if (!content)
			g_error("Invalid xpath expression in get_number");
		res = strtod((char*)content, NULL);
		xmlFree(content);
	}
	else
	{
		g_error("Invalid xpath expression in get_number");
	}

	xmlXPathFreeObject(xres);
	return res;
}

gboolean
get_xpath_bool(xmlXPathObject* xres)
{
	gboolean res;

	if (xres->type == XPATH_BOOLEAN)
	{
		res = (gboolean)xmlXPathCastToBoolean(xres);
	}
	else if (xres->type == XPATH_NODESET && xres->nodesetval && xres->nodesetval->nodeNr == 1)
	{
		xmlNode* node = xres->nodesetval->nodeTab[0];
		g_assert(node->type == XML_TEXT_NODE || node->type == XML_ELEMENT_NODE || node->type == XML_ATTRIBUTE_NODE || node->type == XML_TEXT_NODE);
		xmlChar* content = xmlNodeGetContent(node);
		if (!content)
			g_error("Invalid xpath expression in get_boolean");
		if (!xmlStrcasecmp(content, (xmlChar*)"true"))
			res = TRUE;
		else if (!xmlStrcasecmp(content, (xmlChar*)"false"))
			res = FALSE;
		else
			res = atoi((char*)content);
		xmlFree(content);
	}
	else
	{
		g_error("Invalid xpath expression in get_boolean");
	}

	xmlXPathFreeObject(xres);
	return res;
}

