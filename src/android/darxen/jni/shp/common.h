
#ifndef COMMON_H_C93QBOI7
#define COMMON_H_C93QBOI7

#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#define DEBUG_TAG "NDK_Darxen"

void log_debug(const char* fmt, ...);
void log_error(const char* fmt, ...);

JNIEnv* getJniEnv();

#endif
