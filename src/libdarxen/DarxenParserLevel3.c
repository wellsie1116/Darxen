/* parserLvl3.c

   Copyright (C) 2011 Kevin Wells <kevin@darxen.org>

   This file is part of darxen.

   darxen is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   darxen is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with darxen.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DarxenParserLevel3.h"

#define G_LOG_DOMAIN "DarxenParserLevel3"
#include <glib/gmessages.h>

#include "DarxenConversions.h"

#include <malloc.h>
#include <glib/gmappedfile.h>
#include <glib/gtree.h>
#include <glib/gslist.h>

static GTree* memory = NULL;

/* Private Functions */
gboolean								parser_lvl3_parse_wmo_header					(GSList* mem, FILE *objFile, unsigned char *objHeader);
gboolean								parser_lvl3_parse_message_header				(GSList* mem, FILE *objFile, ProductsLevel3MessageHeader *objHeader);
gboolean								parser_lvl3_parse_description_block				(GSList* mem, FILE *objFile, ProductsLevel3Description *objDesc);
ProductsLevel3SymbologyBlock* 			parser_lvl3_parse_symbology_block				(GSList* mem, FILE *objFile);
ProductsLevel3GraphicAlphanumericBlock*	parser_lvl3_parse_graphic_alphanumeric_block	(GSList* mem, FILE *objFile);
ProductsLevel3TabularAlphanumericBlock*	parser_lvl3_parse_tabular_alphanumeric_block	(GSList* mem, FILE *objFile);

gboolean parser_lvl3_parse_radial_data_packet(GSList* mem, FILE *objFile, ProductsLevel3RadialDataPacket *objData);
gboolean parser_lvl3_parse_radial_packet(GSList* mem, FILE *objFile, ProductsLevel3RadialDataPacket *objHeader, ProductsLevel3Radial *objData);

static int cmp_pointers(gconstpointer a, gconstpointer b);
static inline void* memlist_alloc(GSList** list, size_t size);
static void memlist_destroy(GSList* list);

ProductsLevel3Data*
parser_lvl3_parse_file(FILE *objFile)
{
	if (!memory)
		memory = g_tree_new(cmp_pointers);

	g_return_val_if_fail(objFile, NULL);

	GSList* mem = NULL;

	ProductsLevel3Data* data = (ProductsLevel3Data*)memlist_alloc(&mem, sizeof(ProductsLevel3Data));

	/* get the WMO header */
	if (!parser_lvl3_parse_wmo_header(mem, objFile, data->chrWmoHeader))
	{
		g_critical("Read Error: WMO Header");
		parser_end(objFile);
		memlist_destroy(mem);
		return NULL;
	}

	/* get the message header */
	if (!parser_lvl3_parse_message_header(mem, objFile, &data->objHeader))
	{
		g_critical("Read Error: Message Header Block");
		parser_end(objFile);
		memlist_destroy(mem);
		return NULL;
	}

	/* get the description */
	if (!parser_lvl3_parse_description_block(mem, objFile, &data->objDescription))
	{
		g_critical("Read Error: Message Description Block");
		parser_end(objFile);
		memlist_destroy(mem);
		return NULL;
	}

	parser_end(objFile);

	g_tree_insert(memory, data, mem);

	return data;
}

ProductsLevel3Data*
darxen_parser_lvl3_parse(const char* data)
{
	g_critical("Not implemented");
	return NULL;
}

void
parser_lvl3_free(ProductsLevel3Data* data)
{
	GSList* mem = g_tree_lookup(memory, data);
	if (!mem)
	{
		g_critical("Invalid data pointer");
		return;
	}

	g_tree_remove(memory, data);
	memlist_destroy(mem);
}

/*********************
 * Private Functions *
 *********************/

gboolean
parser_lvl3_parse_wmo_header(GSList* mem, FILE *objFile, unsigned char *objHeader)
{
	int i;
	parser_read_bytes(objFile, objHeader, 27);
	objHeader[27] = 0x00;
	for (i = 0; i < 3; i++)
		parser_read_byte(objFile);
	/*
	for (int i = 0; i < 27; i++)
	{
		if (objHeader[i] == 0x0A || objHeader[i] == 0x0D)
			objHeader[i] = 0x20;
	}
	*/
	return TRUE;
}

gboolean
parser_lvl3_parse_message_header(GSList* mem, FILE *objFile, ProductsLevel3MessageHeader *objHeader)
{
	objHeader->intMessageCode = parser_read_short(objFile);
	objHeader->objDate = darxen_conversions_get_date_from_mod_julian(parser_read_short(objFile));
	objHeader->intTime = parser_read_int(objFile);
	parser_read_int(objFile); /* unused (message length) */
	objHeader->intSourceID = parser_read_short(objFile);
	objHeader->intDestinationID = parser_read_short(objFile);
	objHeader->intNumBlocks = parser_read_short(objFile);
	return TRUE;
}

gboolean
parser_lvl3_parse_description_block(GSList* mem, FILE *objFile, ProductsLevel3Description *objDesc)
{
	int i;
	int intSymbologyBlockOffset;
	int intGraphicAlphanumericBlockOffset;
	int intTabularAlphanumericBlockOffset;
	if (parser_read_short(objFile) != -1)
	{
		g_critical("Read Error: Description Block Header check failed");
		return FALSE;
	}
	objDesc->fltLat = (float)parser_read_int(objFile) / 1000.0f;
	objDesc->fltLon = (float)parser_read_int(objFile) / 1000.0f;
	objDesc->fltAltitude = (float)parser_read_short(objFile) / 1000.0f;
	objDesc->intProdCode = parser_read_short(objFile);
	objDesc->intOpMode = parser_read_short(objFile);
	objDesc->intVolCovPat = parser_read_short(objFile);
	objDesc->intSeqNum = parser_read_short(objFile);
	objDesc->intScanNum = parser_read_short(objFile);
	objDesc->objScanDate = darxen_conversions_get_date_from_mod_julian(parser_read_short(objFile));
	objDesc->intScanTime = parser_read_int(objFile);
	objDesc->objGenDate = darxen_conversions_get_date_from_mod_julian(parser_read_short(objFile));
	objDesc->intGenTime = parser_read_int(objFile);
	for (i = 0; i < 2; i++)
		objDesc->intProdCodes[i] = parser_read_short(objFile);
	/* end for */
	objDesc->intElevationNum = parser_read_short(objFile);
	objDesc->intProdCodes[2] = parser_read_short(objFile);
	for (i = 0; i < 16; i++)
	{
		objDesc->chrDataThresholdInfo[i] = parser_read_byte(objFile);
		objDesc->chrDataThresholdValue[i] = parser_read_byte(objFile);
	}
	for (i = 0; i < 7; i++)
		objDesc->intProdCodes[i + 3] = parser_read_short(objFile);
	if (parser_read_byte(objFile) > 1)
	{
		g_critical("Read Error: Spot Blank byte was not what was expected");
		return FALSE;
	}
	objDesc->chrVersion = parser_read_byte(objFile);
	objDesc->objSymbologyBlock = NULL;
	objDesc->objGraphicAlphanumericBlock = NULL;
	objDesc->objTabularAlphanumericBlock = NULL;
	intSymbologyBlockOffset = parser_read_int(objFile);
	intGraphicAlphanumericBlockOffset = parser_read_int(objFile);
	intTabularAlphanumericBlockOffset = parser_read_int(objFile);
	if (intSymbologyBlockOffset)
	{
//		fseek(objFile, intSymbologyBlockOffset + 27, SEEK_SET);
		objDesc->objSymbologyBlock = parser_lvl3_parse_symbology_block(mem, objFile);
		if (!objDesc->objSymbologyBlock)
		{
			g_critical("Read Error: Symbology Block");
			return FALSE;
		}
	}
	if (intGraphicAlphanumericBlockOffset)
	{
		objDesc->objGraphicAlphanumericBlock = parser_lvl3_parse_graphic_alphanumeric_block(mem, objFile);
		if (!objDesc->objGraphicAlphanumericBlock)
		{
			g_critical("Read Error: Graphic Alphanumeric Block");
			return FALSE;
		}
	}
	if (intTabularAlphanumericBlockOffset)
	{
		objDesc->objTabularAlphanumericBlock = parser_lvl3_parse_tabular_alphanumeric_block(mem, objFile);
		if (!objDesc->objTabularAlphanumericBlock)
		{
			g_critical("Read Error: Tabular Alphanumeric Block failed to read");
			return FALSE;
		}
	}
	return TRUE;
}

ProductsLevel3SymbologyBlock*
parser_lvl3_parse_symbology_block (GSList* mem, FILE *objFile)
{
	int i;
	unsigned short int intPacketCode;
	ProductsLevel3SymbologyBlock* objSymBlock;
	if (parser_read_short(objFile) != -1 || parser_read_short(objFile) != 1)
	{
		g_critical("Read Error: Symbology Block header check failed");
		return NULL;
	}
	objSymBlock = (ProductsLevel3SymbologyBlock*)malloc(sizeof(ProductsLevel3SymbologyBlock));
	parser_read_int(objFile); /* unused (length) */
	objSymBlock->intNumLayers = parser_read_short(objFile);
//	printf("Num layers: %i\n", objSymBlock->intNumLayers);fflush(stdout);
	if (objSymBlock->intNumLayers < 1 || objSymBlock->intNumLayers > 18)
	{
		g_critical("Read Error: Symbology Block layer count check failed");
		return NULL;
	}
	objSymBlock->intPacketTypes = (ProductsLevel3PacketType*)memlist_alloc(&mem, sizeof(ProductsLevel3PacketType) * objSymBlock->intNumLayers);
	objSymBlock->objPackets = (ProductsLevel3Packet*)memlist_alloc(&mem, sizeof(ProductsLevel3Packet) * objSymBlock->intNumLayers);
	for (i = 0; i < objSymBlock->intNumLayers; i++)
	{
		if (parser_read_short(objFile) != -1)
		{
			g_critical("Read Error: Symbology Block layer divider check failed");
			return NULL;
		}
		parser_read_int(objFile); /*unused (layer length) */
		//FIXME: the packet code should be saved in the structure
		intPacketCode = parser_read_short(objFile);
		gboolean parsed = FALSE;
		switch (intPacketCode)
		{
		case 6: case 9: /* Linked Vector Packet */
			g_critical("Not Implemented: Linked Vector Packet");
			break;
		case 7: case 10:/* Unlinked Vector Packet */
			g_critical("Not Implemented: Unlinked Vector Packet");
			break;
		case 0x0E03: case 0x0802: case 0x3501: /* Contour Vector Packet */
			g_critical("Not Implemented: Contour Vector Packet");
			break;
		case 1: case 2: case 8: /* Text and Special Symbol Packets */
			g_critical("Not Implemented: Text and Special Symbol Packets");
			break;
		case 0x0E23: case 0x4E00: case 0x3521: case 0x4E01: /* Map Message Packet */
			g_critical("Not Implemented: Map Message Packet");
			break;
		case 0xAF1F: /* Radial Data Packet */
			objSymBlock->intPacketTypes[i] = LEVEL3_RADIAL_DATA;
			if (!(parsed = parser_lvl3_parse_radial_data_packet(mem, objFile, &objSymBlock->objPackets[i].objRadialData)))
			{
				g_critical("Radial Data Packet failed to read");
				return NULL;
			}
			break;
		case 0xBA0F: case 0xBA07: /* Raster Data Packet */
			objSymBlock->intPacketTypes[i] = LEVEL3_RASTER_DATA;
			g_critical("Not Implemented: Raster Data Packet");
			break;
		case 17: /* Digital Precipitation Data Array Packet */
			g_critical("Not Implemented: Digital Precipitation Data Array Packet");
			break;
		case 18: /* Precipitation Rate Data Array Packet */
			g_critical("Not Implemented: Precipitation Rate Data Array Packet");
			break;
		case 16: /* Digital Radial Data Array Packet */
			g_critical("Not Implemented: Digital Radial Data Array Packet");
			break;
		case 5: /* Vector Arrow Data Packet */
			g_critical("Not Implemented: Vector Arrow Data Packet");
			break;
		case 4: /* Wind Barb Data Packet */
			g_critical("Not Implemented: Wind Barb Data Packet");
			break;
		case 3: case 11: case 12: case 15: case 19: case 26: case 13: case 14: case 23: case 24: case 25: /* Special Graphic Symbol Packet */
			g_critical("Not Implemented: Special Graphic Symbol Packet");
			break;
		case 20: /* Special Graphic Symbol Packet */
			g_critical("Not Implemented: Special Graphic Symbol Packet");
			break;
		case 21: /* Cell Trend Data Packet */
			g_critical("Not Implemented: Cell Trend Data Packet");
			break;
		case 22: /* Cell Trend Volume Scan Times */
			g_critical("Not Implemented: Cell Trend Volume Scan Times");
			break;
		case 27: /* SuperOb Wind Data Packet */
			g_critical("Not Implemented: SuperOb Wind Data Packet");
			break;
		default:
			g_critical("Read Error: Symbology Block layer code check failed");
		}
		if (!parsed)
			return NULL;
	}
	return objSymBlock;
}

ProductsLevel3GraphicAlphanumericBlock*
parser_lvl3_parse_graphic_alphanumeric_block(GSList* mem, FILE *objFile)
{
	g_critical("Not Implemented: Graphic Alphanumeric Block");
	return NULL;
}

ProductsLevel3TabularAlphanumericBlock*
parser_lvl3_parse_tabular_alphanumeric_block(GSList* mem, FILE *objFile)
{
	g_critical("Not Implemented: Tabular Alphanumeric Block");
	return NULL;
}

gboolean
parser_lvl3_parse_radial_data_packet(GSList* mem, FILE *objFile, ProductsLevel3RadialDataPacket *objData)
{
	int i;
	objData->intFirstRangeBin = parser_read_short(objFile);
	objData->intNumRangeBins = parser_read_short(objFile);
	objData->intCenterSweepI = parser_read_short(objFile);
	objData->intCenterSweepJ = parser_read_short(objFile);
	objData->fltScaleFactor = (float)parser_read_short(objFile) / 1000.0f;
	objData->intRadialCount = parser_read_short(objFile);
	if (objData->intRadialCount < 0 || objData->intRadialCount > 1000)
	{
		g_critical("Read Error: Radial Data Packet radial count check failed");
		return FALSE;
	}
	objData->objRadials = (ProductsLevel3Radial*)memlist_alloc(&mem, sizeof(ProductsLevel3Radial) * objData->intRadialCount);
	for (i = 0; i < objData->intRadialCount; i++)
	{
		if (!parser_lvl3_parse_radial_packet(mem, objFile, objData, &objData->objRadials[i]))
		{
			g_critical("Read Error: Radial Packet");
			return FALSE;
		}
	}
	return TRUE;
}

gboolean
parser_lvl3_parse_radial_packet(GSList* mem, FILE *objFile, ProductsLevel3RadialDataPacket *objHeader, ProductsLevel3Radial *objData)
{
	int intRleAccum;
	unsigned char chrRLE;
	short int intRLE;
	int j;
	int k;
	short int intRadialRleCount = parser_read_short(objFile) * 2;
	objData->fltStartAngle = (float)parser_read_short(objFile) / 10.0f;
	objData->fltAngleDelta = (float)parser_read_short(objFile) / 10.0f;
	if (intRadialRleCount < 0 || intRadialRleCount > 1000)
	{
		g_critical("Read Error: Level 3 Radial Packet RLE Count check failed");
		return FALSE;
	}
	objData->chrColorCode = (unsigned char*)memlist_alloc(&mem, sizeof(unsigned char) * objHeader->intNumRangeBins);
	intRleAccum = 0;
	for (j = 0; j < intRadialRleCount; j++) /* for each RLE */
	{
		chrRLE = parser_read_byte(objFile);
		intRLE = chrRLE >> 4;
		chrRLE = chrRLE & 0x0F;
		for (k = 0; k < intRLE; k++) /* for each rle byte */
		{
			objData->chrColorCode[k + intRleAccum] = chrRLE;
		}
		intRleAccum += intRLE;
	}
	return TRUE;
}

static int
cmp_pointers(gconstpointer a, gconstpointer b)
{
	return (a < b) ? -1 : (a > b);
}

static inline void*
memlist_alloc(GSList** list, size_t size)
{
	void* mem = malloc(size);
	*list = g_slist_prepend(*list, mem);
	return mem;
}

static void
memlist_destroy(GSList* list)
{
	GSList* plist = list;
	while (plist)
	{
		free(plist->data);
		plist = plist->next;
	}
	g_slist_free(list);
}

