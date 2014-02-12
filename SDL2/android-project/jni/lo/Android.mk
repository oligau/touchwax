LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lo

APP_SUBDIRS := src

APP_SUBDIRS := $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/src -type d))

LOCAL_C_INCLUDES := $(LOCAL_PATH)/lo
LOCAL_CFLAGS := -g -O2 -DPACKAGE_NAME=\"liblo\" -DHAVE_POLL -DENABLE_THREADS \
	-DLO_SO_VERSION=\{9,0,2\} -DPACKAGE_VERSION=\"0.28\"

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))

LOCAL_SHARED_LIBRARIES := 

LOCAL_LDLIBS := -lm

include $(BUILD_SHARED_LIBRARY)

