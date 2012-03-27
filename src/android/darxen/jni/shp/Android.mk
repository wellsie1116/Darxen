LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_LDLIBS := -llog

SHP_FILES := safileio.c dbfopen.c shpopen.c shptree.c
WRAPPER_FILES := common.c dbffile.c hashtable.c inputstreamhooks.c shapefile.c shapefileobject.c

LOCAL_MODULE    := shp
LOCAL_SRC_FILES := $(SHP_FILES) $(WRAPPER_FILES)

include $(BUILD_SHARED_LIBRARY)
