/* DarxenDataStructs.h

   Copyright (C) 2008 Kevin Wells <kevin@darxen.org>

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

#ifndef __DATA_STRUCTS_H__
#define __DATA_STRUCTS_H__

#include "libdarxenCommon.h"

#include <glib/gdate.h>
#include <stdio.h>

G_BEGIN_DECLS

typedef struct _ProductsLevel3MessageHeader				ProductsLevel3MessageHeader;
typedef struct _ProductsLevel3Radial					ProductsLevel3Radial;
typedef struct _ProductsLevel3RasterRow					ProductsLevel3RasterRow;
typedef struct _ProductsLevel3RadialDataPacket			ProductsLevel3RadialDataPacket;
typedef struct _ProductsLevel3RasterDataPacket			ProductsLevel3RasterDataPacket;
typedef struct _ProductsLevel3Description				ProductsLevel3Description;
typedef struct _ProductsLevel3SymbologyBlock			ProductsLevel3SymbologyBlock;
typedef struct _ProductsLevel3GraphicAlphanumericBlock	ProductsLevel3GraphicAlphanumericBlock;
typedef struct _ProductsLevel3TabularAlphanumericBlock	ProductsLevel3TabularAlphanumericBlock;
typedef struct _ProductsLevel3Data						ProductsLevel3Data;
typedef enum   _ProductsLevel3PacketType				ProductsLevel3PacketType;
typedef union  _ProductsLevel3Packet					ProductsLevel3Packet;

/* function prototypes */
G_EXPORT char* data_wmo_get_site(unsigned char *chrWmoHeader);
G_EXPORT char* data_wmo_get_product_code(unsigned char *chrWmoHeader);

/** Holds information on the message */
struct _ProductsLevel3MessageHeader
{
	/** The message code */
	short int intMessageCode;
	/** The date that the message was sent */
	GDate *objDate;
	/** The time that the message was sent (seconds after midnight GMT) */
	unsigned int intTime;
	/** The source ID of the sender */
	short int intSourceID;
	/** The destination id for message transmission */
	short int intDestinationID;
	/** The number of blocks in this product (1-51) */
	short int intNumBlocks;
};

/** Holds information on a single radial in a radial image */
struct _ProductsLevel3Radial
{
	/** The angle that this radial starts at */
	float fltStartAngle;
	/** The difference between the start angle and the end angle (fltStartAngle + fltAngleDelta = the end angle) */
	float fltAngleDelta;
	/** The array of bytes containing the index of the color for that point */
	unsigned char *chrColorCode;
};

/** Holds information on a single row in a raster image */
struct _ProductsLevel3RasterRow
{
	/** The number of bytes in this row */
	short int intNumBytes;
	/** The array of bytes containing the index of the color for that point */
	unsigned char *chrColorCode;
};

/** Holds radial information */
struct _ProductsLevel3RadialDataPacket
{
	/** The location of the first range bin */
	short int intFirstRangeBin;
	/** The total number of range bins (number of data for each radial) */
	short int intNumRangeBins;
	/** I coordinate of center of sweep */
	short int intCenterSweepI;
	/** J coordinate of center of sweep */
	short int intCenterSweepJ;
	/** Number of pixels per range bin */
	float fltScaleFactor;
	/** Total number of radials */
	short int intRadialCount;
	/** The array of radials (size of array is determined by intRadialCount */
	ProductsLevel3Radial *objRadials;
};

/** Holds raster information */
struct _ProductsLevel3RasterDataPacket
{
	/** I coordinate for the starting position of the image */
	short int intStartCoordI;
	/** J coordinate for the starting position of the image */
	short int intStartCoordJ;
	/** Scale on the X axis */
	short int intScaleX;
	/** Scale on the Y axis */
	short int intScaleY;
	/** The number of rows in the image */
	short int intNumRows;
	/** The packing descriptor (this should always be 2) */
	short int intPackingDescriptor;
	/** The array of radials (size of array is determined by intNumRows */
	ProductsLevel3RasterRow *objRows;
};

/** enumeration that contains all different possible packet types */
enum _ProductsLevel3PacketType
{
	LEVEL3_RADIAL_DATA,
	LEVEL3_RASTER_DATA
};

/** union that contains all different possible packets */
union _ProductsLevel3Packet
{
	ProductsLevel3RadialDataPacket objRadialData;
	ProductsLevel3RasterDataPacket objRasterData;
};

/** Holds information specific to Level 3 products */
struct _ProductsLevel3Description
{
	/** The latitude of the radar site (North is positive) */
	float fltLat;
	/** The longitude of the radar site (East is positive) */
	float fltLon;
	/** The altitude of the radar site above sea level (in feet) */
	float fltAltitude;
	/** The product code for this product */
	short int intProdCode;
	/** The operation mode of the radar site (0=Maintenance, 1=Clean Air, 2=Precip/severe weather) */
	short int intOpMode;
	/** RDA volume coverage pattern for the scan strategy being used */
	short int intVolCovPat;
	/** Sequence number of the request that generated the product */
	short int intSeqNum;
	/** Scan number */
	short int intScanNum;
	/** Date from when the product was scanned */
	GDate *objScanDate;
	/** Number of seconds after midnight GMT from when the product was scanned */
	int intScanTime;
	/** Date from when the product was generated */
	GDate *objGenDate;
	/** Number of seconds after midnight GMT from when the product was generated */
	int intGenTime;
	/** Product specific codes */
	unsigned short int intProdCodes[10];
	/** Elevation number within volume scan for elevation based products (0-20) */
	short int intElevationNum;
	/** Info on the data thresholds for the product */
	unsigned char chrDataThresholdInfo[16];
	/** Values of the data thresholds for the product) */
	unsigned char chrDataThresholdValue[16];
	/** Version of the product */
	unsigned char chrVersion;
	/** A pointer to a SymbologyBlock (the block is not in the file if the pointer is NULL) */
	ProductsLevel3SymbologyBlock *objSymbologyBlock;
	/** A pointer to a GraphicAlphanumericBlock (the block is not in the file if the pointer is NULL) */
	ProductsLevel3GraphicAlphanumericBlock *objGraphicAlphanumericBlock;
	/** A pointer to a TabularAlphanumericBlock (the block is not in the file if the pointer is NULL) */
	ProductsLevel3TabularAlphanumericBlock *objTabularAlphanumericBlock;
};

/** Holds information on symbology (imagery) */
struct _ProductsLevel3SymbologyBlock
{
	/** The number of data layers in this block */
	short int intNumLayers;
	/** Array of PacketType signifying what type each element in objPackets is */
	ProductsLevel3PacketType *intPacketTypes;
	/** Array of Packet containing a structure for the specific packet defined by intPacketTypes */
	ProductsLevel3Packet *objPackets;
};

/** Holds information on the corresponding symbology data */
struct _ProductsLevel3GraphicAlphanumericBlock
{
	/** The number of pages contained in this block */
	short int intNumPages;
	/** An array containing the number of bytes in the current Text Packet */
	short int *intTextLength;
	/** An array of Text Packets */
	unsigned char **chrTextPacket;
};

/** Holds information in a tabular format */
struct _ProductsLevel3TabularAlphanumericBlock
{
	/** A second message header block */
	ProductsLevel3MessageHeader objHeader;
	/** A second product description block */
	ProductsLevel3Description objDescription;
	/** The number of pages */
	short int intNumPages;
	/** The number of characters per page */
	short int intCharsPerPage;
	/** The array of characters that make up the table */
	unsigned char *chrData;
};

/** Holds all information for a level 3 file */
struct _ProductsLevel3Data
{
	/** The WMO Header */
	unsigned char chrWmoHeader[28];
	/** The message header */
	ProductsLevel3MessageHeader objHeader;
	/** The product description */
	ProductsLevel3Description objDescription;
};

G_END_DECLS

#endif /* __DATA_STRUCTS_H__ */
