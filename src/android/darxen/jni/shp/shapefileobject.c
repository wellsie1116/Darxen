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

static void setDoubleArray(JNIEnv* env, jobject this, const char* name, jobject value)
{
	jclass klass = (*env)->GetObjectClass(env, this);
	jfieldID fid = (*env)->GetFieldID(env, klass, name, "[D");
	(*env)->SetObjectField(env, this, fid, value);
}

static void setIntArray(JNIEnv* env, jobject this, const char* name, jobject value)
{
	jclass klass = (*env)->GetObjectClass(env, this);
	jfieldID fid = (*env)->GetFieldID(env, klass, name, "[I");
	(*env)->SetObjectField(env, this, fid, value);
}

static SHPObject* getShapeObject(JNIEnv* env, jobject this)
{
	jclass klass = (*env)->GetObjectClass(env, this);
	jfieldID fid = (*env)->GetFieldID(env, klass, "hShape", "J");
	long l = (*env)->GetLongField(env, this, fid);
	return (SHPObject*)(void*)l;
}

static void setShapeObject(JNIEnv* env, jobject this, SHPObject* handle)
{
	jclass klass = (*env)->GetObjectClass(env, this);
	jfieldID fid = (*env)->GetFieldID(env, klass, "hShape", "J");
	(*env)->SetLongField(env, this, fid, (long)handle);
}

static jdoubleArray InitDoubleArray(JNIEnv* env, double* v, int n)
{
	jdoubleArray array = (*env)->NewDoubleArray(env, n);
	double* arr = (*env)->GetDoubleArrayElements(env, array, NULL);
	int i;
	for (i = 0; i < n; i++)
		arr[i] = v[i];
	(*env)->ReleaseDoubleArrayElements(env, array, arr, 0);
	return array;
}

static jintArray InitIntArray(JNIEnv* env, int* v, int n)
{
	jintArray array = (*env)->NewIntArray(env, n);
	int* arr = (*env)->GetIntArrayElements(env, array, NULL);
	int i;
	for (i = 0; i < n; i++)
		arr[i] = v[i];
	(*env)->ReleaseIntArrayElements(env, array, arr, 0);
	return array;
}

void Java_me_kevinwells_darxen_shp_ShapefileObject_load(JNIEnv* env, jobject this)
{
	SHPObject* shpObj = getShapeObject(env, this);
	if (!shpObj)
		return;

	jintArray panPartStart = InitIntArray(env, shpObj->panPartStart, shpObj->nParts);
	jintArray panPartType = InitIntArray(env, shpObj->panPartType, shpObj->nParts);
	jdoubleArray padfX = InitDoubleArray(env, shpObj->padfX, shpObj->nVertices);
	jdoubleArray padfY = InitDoubleArray(env, shpObj->padfY, shpObj->nVertices);
	jdoubleArray padfZ = InitDoubleArray(env, shpObj->padfZ, shpObj->nVertices);

	setIntArray(env, this, "panPartStart", panPartStart);
	setIntArray(env, this, "panPartType", panPartType);
	setDoubleArray(env, this, "padfX", padfX);
	setDoubleArray(env, this, "padfY", padfY);
	setDoubleArray(env, this, "padfZ", padfZ);

	SHPDestroyObject(shpObj);
	setShapeObject(env, this, NULL);
}

void Java_me_kevinwells_darxen_shp_ShapefileObject_close(JNIEnv* env, jobject this)
{
	SHPObject* shpObj = getShapeObject(env, this);
	if (!shpObj)
		return;

	SHPDestroyObject(shpObj);
	setShapeObject(env, this, NULL);
}

