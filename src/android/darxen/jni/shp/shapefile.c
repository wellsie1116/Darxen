#include <jni.h>
#include <string.h>
#include <android/log.h>

#include "shapefil.h"

#define DEBUG_TAG "NDK_Darxen"

static void setInt(JNIEnv* env, jobject this, const char* name, int value)
{
	jclass klass = (*env)->GetObjectClass(env, this);
	jfieldID fid = (*env)->GetFieldID(env, klass, name, "I");
	(*env)->SetIntField(env, this, fid, value);
}

static void setShapeHandle(JNIEnv* env, jobject this, SHPHandle handle)
{
	jclass klass = (*env)->GetObjectClass(env, this);
	jfieldID fid = (*env)->GetFieldID(env, klass, "hShp", "J");
	(*env)->SetLongField(env, this, fid, (long)handle);
}

static SHPHandle getShapeHandle(JNIEnv* env, jobject this)
{
	jclass klass = (*env)->GetObjectClass(env, this);
	jfieldID fid = (*env)->GetFieldID(env, klass, "hShp", "J");
	long l = (*env)->GetLongField(env, this, fid);
	return (SHPHandle)(void*)l;
}

void Java_me_kevinwells_darxen_shp_Shapefile_open(JNIEnv* env, jobject this, jstring path)
{
    jboolean isCopy;
    const char* sPath = (*env)->GetStringUTFChars(env, path, &isCopy);

    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Opening shapefile: %s", sPath);

	SHPHandle hShp = SHPOpen(sPath, "rb");
	if (!hShp)
		goto Exit;

	int entities;
	int shapeType;
	SHPGetInfo(hShp, &entities, &shapeType, NULL, NULL);
	setInt(env, this, "entities", entities);
	setInt(env, this, "shapeType", shapeType);

	setShapeHandle(env, this, hShp);

Exit:
    (*env)->ReleaseStringUTFChars(env, path, sPath);
}

void Java_me_kevinwells_darxen_shp_Shapefile_close(JNIEnv* env, jobject this)
{
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Closing shapefile");

	SHPHandle hShp = getShapeHandle(env, this);
	SHPClose(hShp);
	setShapeHandle(env, this, NULL);
}

static jobject NewShapefile(JNIEnv* env, jobject this, SHPObject* shpObj)
{
	jclass shapefileClass = (*env)->FindClass(env, "me/kevinwells/darxen/shp/ShapefileObject");
	if (!shapefileClass)
		goto Exit;

	jmethodID constructor = (*env)->GetMethodID(env, shapefileClass, "<init>",
			"(IIDDDDJ)V");

	jobject res = (*env)->NewObject(env, shapefileClass, constructor, 
			shpObj->nParts, shpObj->nVertices,
			shpObj->dfXMin, shpObj->dfYMin, shpObj->dfXMax, shpObj->dfYMax,
			(long)shpObj);

	return res;

Exit:
	return NULL;
}

jobject Java_me_kevinwells_darxen_shp_Shapefile_get(JNIEnv* env, jobject this, int i)
{
	jobject res = NULL;
    //__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Reading shape: %d", i);

	SHPHandle hShp = getShapeHandle(env, this);
	if (!hShp)
		goto Exit;

	SHPObject* shpObj = SHPReadObject(hShp, i);
	if (!shpObj)
		goto Exit;

	res = NewShapefile(env, this, shpObj);

Exit:
	return res;
}

//int Java_me_kevinwells_darxen_shp_Shapefile_getShapeCount(JNIEnv * env, jobject this)
//{
//	return 42;
//}

