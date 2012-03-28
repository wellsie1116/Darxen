
#include "common.h"

#include "shapefil.h"
#include "inputstreamhooks.h"

static int getInt(JNIEnv* env, jobject this, const char* name)
{
	jclass klass = (*env)->GetObjectClass(env, this);
	jfieldID fid = (*env)->GetFieldID(env, klass, name, "I");
	return (*env)->GetIntField(env, this, fid);
}

static void setInt(JNIEnv* env, jobject this, const char* name, int value)
{
	jclass klass = (*env)->GetObjectClass(env, this);
	jfieldID fid = (*env)->GetFieldID(env, klass, name, "I");
	(*env)->SetIntField(env, this, fid, value);
}

static DBFHandle getDbfHandle(JNIEnv* env, jobject this)
{
	jclass klass = (*env)->GetObjectClass(env, this);
	jfieldID fid = (*env)->GetFieldID(env, klass, "hDbf", "J");
	long l = (*env)->GetLongField(env, this, fid);
	return (DBFHandle)(void*)l;
}

static void setDbfHandle(JNIEnv* env, jobject this, DBFHandle handle)
{
	jclass klass = (*env)->GetObjectClass(env, this);
	jfieldID fid = (*env)->GetFieldID(env, klass, "hDbf", "J");
	(*env)->SetLongField(env, this, fid, (long)handle);
}

void Java_me_kevinwells_darxen_shp_DbfFile_init(JNIEnv* env, jobject this, jobject fDbf)
{
	int id;
	const char* dbfPath = registerInputStream((*env)->NewGlobalRef(env, fDbf), TYPE_DBF, &id);
	setInt(env, this, "mInputId", id);

    log_debug("Opening dbf file: %s", dbfPath);

	DBFHandle hDbf = DBFOpenLL(dbfPath, "rb", getInputStreamHooks());
	if (!hDbf)
	{
		log_error("Failed to open dbf file");
		return;
	}

	int fieldCount = DBFGetFieldCount(hDbf);
	int recordCount = DBFGetRecordCount(hDbf);
	setInt(env, this, "mFieldCount", fieldCount);
	setInt(env, this, "mRecordCount", recordCount);

	setDbfHandle(env, this, hDbf);
}

void Java_me_kevinwells_darxen_shp_DbfFile_close(JNIEnv* env, jobject this)
{
	int id = getInt(env, this, "mInputId");
	DBFHandle hDbf = getDbfHandle(env, this);
	if (!hDbf)
		return;

    log_debug("Closing dbf file");

	DBFClose(hDbf);
	setDbfHandle(env, this, NULL);
	
	unregisterInputStreams(env, id);
	setInt(env, this, "mInputId", 0);
}

static void ThrowClosedException(JNIEnv* env)
{
	jclass excClass = (*env)->FindClass(env, "java/lang/IllegalStateException");
	if (!excClass)
		return;

	(*env)->ThrowNew(env, excClass, "Dbf file closed");
}

jboolean Java_me_kevinwells_darxen_shp_DbfFile_isNull(JNIEnv* env, jobject this, int record, int field)
{
	DBFHandle hDbf = getDbfHandle(env, this);
	if (!hDbf)
	{
		ThrowClosedException(env);
		return;
	}

	return DBFIsAttributeNULL(hDbf, record, field);
}

jint Java_me_kevinwells_darxen_shp_DbfFile_getInt(JNIEnv* env, jobject this, int record, int field)
{
	DBFHandle hDbf = getDbfHandle(env, this);
	if (!hDbf)
	{
		ThrowClosedException(env);
		return;
	}

	return DBFReadIntegerAttribute(hDbf, record, field);
}

jdouble Java_me_kevinwells_darxen_shp_DbfFile_getDouble(JNIEnv* env, jobject this, int record, int field)
{
	DBFHandle hDbf = getDbfHandle(env, this);
	if (!hDbf)
	{
		ThrowClosedException(env);
		return;
	}

	return DBFReadDoubleAttribute(hDbf, record, field);
}

jstring Java_me_kevinwells_darxen_shp_DbfFile_getString(JNIEnv* env, jobject this, int record, int field)
{
	DBFHandle hDbf = getDbfHandle(env, this);
	if (!hDbf)
	{
		ThrowClosedException(env);
		return;
	}

	const char* value = DBFReadStringAttribute(hDbf, record, field);
	return (*env)->NewStringUTF(env, value);
}

