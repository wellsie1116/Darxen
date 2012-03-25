LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_LDLIBS := -llog

LOCAL_MODULE    := shp
LOCAL_SRC_FILES := shpopen.c shptree.c dbfopen.c safileio.c shapefile.c

include $(BUILD_SHARED_LIBRARY)
