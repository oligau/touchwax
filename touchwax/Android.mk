LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL2
LO_PATH := ../lo
SDL_TTF_PATH := ../SDL2_ttf

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
	$(LOCAL_PATH)/$(LO_PATH) \
	$(LOCAL_PATH)/$(SDL_TTF_PATH)

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
	touchwax.c \
	osc.c \
	track.c \
	button.c \
	fader.c \
	overview.c \
	closeup.c \
	label.c \
	interface.c

LOCAL_SHARED_LIBRARIES := lo SDL2 SDL2_ttf libzip

LOCAL_LDLIBS := -lGLESv1_CM -llog

include $(BUILD_SHARED_LIBRARY)
