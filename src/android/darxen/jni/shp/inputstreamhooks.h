
#ifndef INPUTSTREAMHOOKS_H_EGMGSYY9
#define INPUTSTREAMHOOKS_H_EGMGSYY9

#include "common.h"
#include "shapefil.h"

typedef enum
{
	TYPE_SHP,
	TYPE_SHX,
	TYPE_DBF,

	TYPE_COUNT
} ShapefileType;

SAHooks* getInputStreamHooks();

const char* registerInputStream(jobject fin, ShapefileType type, int* id);
bool addInputStream(int id, jobject fin, ShapefileType type);
bool unregisterInputStreams(JNIEnv* env, int id);

#endif
