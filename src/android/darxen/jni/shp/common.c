
#include "common.h"

static JavaVM* javaVM = NULL;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	javaVM = vm;

	JNIEnv* env;
	if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6))
	{
		log_error("Unable to obtain JNIEnv!");
		return JNI_ERR;
	}

	log_debug("JNI loaded successfully");
	return JNI_VERSION_1_6;
}

void log_debug(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
    __android_log_vprint(ANDROID_LOG_DEBUG, DEBUG_TAG, fmt, args);
	va_end(args);
}

void log_error(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
    __android_log_vprint(ANDROID_LOG_ERROR, DEBUG_TAG, fmt, args);
	va_end(args);
}

JNIEnv* getJniEnv()
{
	if (!javaVM)
	{
		log_error("Java VM not initialized!");
		return NULL;
	}

	JNIEnv* res = NULL;
	(*javaVM)->GetEnv(javaVM, (void**)&res, JNI_VERSION_1_6);

	if (!res)
		log_error("Unable to obtain JNIEnv!");
	return res;
}

