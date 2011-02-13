/* parser.c

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

#include "DarxenParser.h"

void
parser_end(FILE *objFile)
{
	parser_read_byte(objFile);
	if (!feof(objFile))
	{
		int intBytes = 1;
		while (!feof(objFile))
		{
			parser_read_byte(objFile);
			intBytes++;
		}
		if (intBytes == 1)
			printf("File Parse Warning: There was %i byte of unread data.\n", intBytes);
		else
			printf("File Parse Warning: There are %i bytes of unread data.\n", intBytes);
	}
}

short int
parser_read_short(FILE *objFile)
{
	unsigned char chrShort[2];
	if (fread(chrShort, 2, 1, objFile) != 1)
	{
		printf("Read Error: short int read failed\n");
	}
	return (short int)((chrShort[0] << 8) | (chrShort[1]));
}

int
parser_read_int(FILE *objFile)
{
	unsigned char chrInt[4];
	if (fread(chrInt, 4, 1, objFile) != 1)
		printf("Read Error: int read failed\n");
	return (int)(((unsigned char)chrInt[0] << 24) | ((unsigned char)chrInt[1] << 16) | ((unsigned char)chrInt[2] << 8) | ((unsigned char)chrInt[3]));
}

float
parser_read_float(FILE *objFile)
{
	unsigned char chrFloat[4];
	float fltReturn;
	printf("FYI: readFloat method not tested.\n");
	if (fread(chrFloat, 4, 1, objFile) != 1)
		printf("Read Error: float read failed\n");
	if (parser_get_endianness())
	{
		((char*)&fltReturn)[0] = chrFloat[3];
		((char*)&fltReturn)[1] = chrFloat[2];
		((char*)&fltReturn)[2] = chrFloat[1];
		((char*)&fltReturn)[3] = chrFloat[0];
	}
	else
	{
		((char*)&fltReturn)[0] = chrFloat[0];
		((char*)&fltReturn)[1] = chrFloat[1];
		((char*)&fltReturn)[2] = chrFloat[2];
		((char*)&fltReturn)[3] = chrFloat[3];
	}
	return fltReturn;
}

unsigned char
parser_read_byte(FILE *objFile)
{
	return (unsigned char)fgetc(objFile);
}

void
parser_read_bytes(FILE *objFile, unsigned char *chrBuffer, int intCount)
{
	if (fread(chrBuffer, intCount, 1, objFile) != 1)
		printf("Read Error: Bytes read failed\n");
}

int
parser_read_null_string(FILE *objFile, char *chrBuffer)
{
	char chrTemp = '4';
	int i = 0;
	while (chrTemp != 0x00 && fread(&chrTemp, 1, 1, objFile) == 1)
	{
		chrBuffer[i] = chrTemp;
		i++;
	}
	if (chrTemp != 0x00)
		printf("Read Error: null-terminated string not terminated before end of file\n");
	return i;
}

gboolean
parser_get_endianness()
{
	short int intTest = 1;
	return ((char*)&intTest)[0] == 1;
}
