# Build the unit tests.
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifneq ($(TARGET_SIMULATOR),true)

LOCAL_MODULE := RecorderQueueBufferTest

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    SurfaceMediaSource_test.cpp

LOCAL_SHARED_LIBRARIES := \
	libEGL \
	libGLESv2 \
	libandroid \
	libbinder \
	libcutils \
	libgui \
	libmedia \
	libstagefright \
	libstagefright_omx \
	libstagefright_foundation \
	libstlport \
	libui \
	libutils \

LOCAL_C_INCLUDES := \
    bionic \
    bionic/libstdc++/include \
    external/gtest/include \
    external/stlport/stlport \
	frameworks/base/media/libstagefright \
	frameworks/base/media/libstagefright/include \
	$(TOP)/frameworks/base/include/media/stagefright/openmax \

include $(BUILD_EXECUTABLE)

endif

################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=         \
        CedarxStreammingSourceTest.cpp   \

LOCAL_SHARED_LIBRARIES := \
	libstagefright liblog libutils libbinder libgui \
        libstagefright_foundation libmedia

LOCAL_C_INCLUDES:= \
	$(JNI_H_INCLUDE) \
	frameworks/base/media/libstagefright \
	$(TOP)/frameworks/base/include/media/stagefright/openmax

LOCAL_CFLAGS += -Wno-multichar

LOCAL_MODULE_TAGS := debug

LOCAL_MODULE:= CedarXStreamTest

include $(BUILD_EXECUTABLE)

################################################################################


