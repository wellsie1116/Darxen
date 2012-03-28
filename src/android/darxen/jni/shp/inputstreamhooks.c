
#include "inputstreamhooks.h"

#include "hashtable.h"

#define BUFFER_SIZE 1024

static unsigned long key_hash(int index);
static int key_equal(int a, int b);

static SAFile     InputStream_FOpen(const char* filename, const char* access);
static SAOffset   InputStream_FRead(void* p, SAOffset size, SAOffset nmemb, SAFile file);
static SAOffset   InputStream_FWrite(void* p, SAOffset size, SAOffset nmemb, SAFile file);
static SAOffset   InputStream_FSeek(SAFile file, SAOffset offset, int whence);
static SAOffset   InputStream_FTell(SAFile file);
static int        InputStream_FFlush(SAFile file);
static int        InputStream_FClose(SAFile file);
static int        InputStream_Remove(const char* filename);
static void       InputStream_Error(const char* message);

typedef struct
{
	jobject fin;
	jbyteArray buffer;
	size_t pos;
} InputFileStream;

typedef struct
{
	InputFileStream streams[TYPE_COUNT];
	char path[20];
} ShapefileStreams;

static SAHooks hooks = {NULL, };

static int inputStreamCounter = 1;
static HashTable* inputStreamTable = NULL;

static HashTable* getLookupTable()
{
	if (!inputStreamTable)
	{
		inputStreamTable = hash_table_new((HashFunc)key_hash, (EqualsFunc)key_equal);
	}
	return inputStreamTable;
}

static unsigned long key_hash(int index)
{
	return index;
}

static int key_equal(int a, int b)
{
	return a == b;
}

static void* idToKey(int id)
{
	return (void*)id;
}

SAHooks* getInputStreamHooks()
{
	if (!hooks.FOpen)
	{
		hooks.FOpen = InputStream_FOpen;
		hooks.FRead = InputStream_FRead;
		hooks.FWrite = InputStream_FWrite;
		hooks.FSeek = InputStream_FSeek;
		hooks.FTell = InputStream_FTell;
		hooks.FFlush = InputStream_FFlush;
		hooks.FClose = InputStream_FClose;
		hooks.Remove = InputStream_Remove;
			  
		hooks.Error = InputStream_Error;
		hooks.Atof  = atof;
	}
	return &hooks;
}

static ShapefileStreams* lookupInputStream(const char* path)
{
	HashTable* table = getLookupTable();
	int id = atoi(path);
	return (ShapefileStreams*)hash_table_lookup(table, idToKey(id));
}

const char* registerInputStream(jobject fin, ShapefileType type, int* id)
{
	HashTable* table = getLookupTable();

	ShapefileStreams* file = (ShapefileStreams*)malloc(sizeof(ShapefileStreams));
	memset(file, 0, sizeof(*file));
	file->streams[type].fin = fin;
	*id = inputStreamCounter++;
	sprintf(file->path, "%d", *id);

	hash_table_insert(table, idToKey(*id), file);
	return file->path;
}

bool addInputStream(int id, jobject fin, ShapefileType type)
{
	HashTable* table = getLookupTable();
	ShapefileStreams* file = (ShapefileStreams*)hash_table_lookup(table, idToKey(id));
	if (!file)
		return false;

	if (file->streams[type].fin)
		return false;

	file->streams[type].fin = fin;
	return true;
}

bool unregisterInputStreams(JNIEnv* env, int id)
{
	HashTable* table = getLookupTable();
	ShapefileStreams* file = (ShapefileStreams*)hash_table_lookup(table, idToKey(id));
	if (!file)
		return NULL;

	int i;
	for (i = 0; i < TYPE_COUNT; i++)
	{
		if (file->streams[i].fin)
		{
			(*env)->DeleteGlobalRef(env, file->streams[i].fin);
			if (file->streams[i].buffer)
				log_error("Destroying open file!");
			file->streams[i].fin = NULL;
		}
	}

	hash_table_remove(table, idToKey(id));
	free(file);
	return true;
}

static SAFile InputStream_FOpen(const char* filename, const char* access)
{
	log_debug("SAHooks: FOpen: %s", filename);
	const char* ext = filename + strlen(filename) - 4;

	ShapefileStreams* file = lookupInputStream(filename);

	InputFileStream* fin;

	if (!strcasecmp(".shp", ext))
		fin = &(file->streams[TYPE_SHP]);
	else if (!strcasecmp(".shx", ext))
		fin = &(file->streams[TYPE_SHX]);
	else if (!strcasecmp(".dbf", ext))
		fin = &(file->streams[TYPE_DBF]);
	else
		return NULL;

	if (fin->buffer || fin->pos)
	{
		log_error("File already open");
		return NULL;
	}

	JNIEnv* env = getJniEnv();
	jbyteArray array = (*env)->NewByteArray(env, BUFFER_SIZE);
	fin->buffer = (*env)->NewGlobalRef(env, array);

	return (SAFile)fin;
}

static SAOffset InputStream_FRead(void* p, SAOffset size, SAOffset nmemb, SAFile file)
{
	int res = -1;
	JNIEnv* env = getJniEnv();
	InputFileStream* inputStream = (InputFileStream*)file;

	jclass inputStreamClass = (*env)->FindClass(env, "java/io/InputStream");
	jmethodID method = (*env)->GetMethodID(env, inputStreamClass, "read", "([BII)I");

	size_t pos = 0;
	size_t total = size*nmemb;
	do
	{
		int read = total-pos;
		if (read > BUFFER_SIZE)
			read = BUFFER_SIZE;
		read = (*env)->CallIntMethod(env, inputStream->fin, method,
				inputStream->buffer, 0, read);

		void* arr = (*env)->GetByteArrayElements(env, inputStream->buffer, NULL);
		memcpy(p + pos, arr, read);
		pos += read;
		(*env)->ReleaseByteArrayElements(env, inputStream->buffer, arr, JNI_ABORT);
	} while (pos != total);

	inputStream->pos += total;
	res = nmemb;

Exit:
	return res;
}

static SAOffset InputStream_FWrite(void* p, SAOffset size, SAOffset nmemb, SAFile file)
{
	//writing not supported
	return 0;
}

static SAOffset InputStream_FSeek(SAFile file, SAOffset offset, int whence)
{
	JNIEnv* env = getJniEnv();
	InputFileStream* inputStream = (InputFileStream*)file;

	jclass inputStreamClass = (*env)->FindClass(env, "java/io/InputStream");

	jmethodID methodReset = (*env)->GetMethodID(env, inputStreamClass, "reset", "()V");
	jmethodID methodSkip = (*env)->GetMethodID(env, inputStreamClass, "skip", "(J)J");
	long res;

	long skip;
	switch (whence)
	{
	case SEEK_SET:
		skip = offset - inputStream->pos;
		break;
	case SEEK_CUR:
		skip = offset;
		break;
	default:
		return -1;
	}

	if (skip == 0)
		return 0;

	if (skip > 0)
	{
		res = (*env)->CallLongMethod(env, inputStream->fin, methodSkip, skip);
		inputStream->pos += res;
	}
	else
	{
		(*env)->CallVoidMethod(env, inputStream->fin, methodReset);
		res = (*env)->CallLongMethod(env, inputStream->fin, methodSkip, inputStream->pos+skip);
		inputStream->pos = res;
	}

	return 0;
}

static SAOffset InputStream_FTell(SAFile file)
{
	InputFileStream* inputStream = (InputFileStream*)file;
	return inputStream->pos;
}

static int InputStream_FFlush(SAFile file)
{
	//writing not supported
	return 0;
}

static int InputStream_FClose(SAFile file)
{
	log_debug("SAHooks: FClose");
	InputFileStream* inputStream = (InputFileStream*)file;
	
	JNIEnv* env = getJniEnv();
	(*env)->DeleteGlobalRef(env, inputStream->buffer);
	inputStream->buffer = NULL;

	return 0;
}

static int InputStream_Remove(const char* filename)
{
	//can't delete resources
	return -1;
}

static void InputStream_Error(const char* message)
{
    log_error("SAHooks: %s", message);
}

