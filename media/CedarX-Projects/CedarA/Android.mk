LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include frameworks/base/media/libstagefright/codecs/common/Config.mk
include frameworks/base/media/CedarX-Projects/Config.mk

ifeq ($(CEDARX_ANDROID_VERSION),4)
CEDARA_VERSION_TAG = _
else
CEDARA_VERSION_TAG = _ICS_
endif

LOCAL_SRC_FILES:=                         \
		CedarARender.cpp \
        CedarAPlayer.cpp				  


LOCAL_C_INCLUDES:= \
	$(JNI_H_INCLUDE) \
	$(LOCAL_PATH)/include \
	${CEDARX_TOP}/libcodecs/include \
	$(TOP)/frameworks/base/include/media/stagefright \
    $(TOP)/frameworks/base/include/media/stagefright/openmax

LOCAL_SHARED_LIBRARIES := \
        libbinder         \
        libmedia          \
        libutils          \
        libcutils         \
        libui

ifneq ($(CEDARX_DEBUG_ENABLE),Y)
LOCAL_LDFLAGS += \
	$(LOCAL_PATH)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libcedara_decoder.a
endif

LOCAL_LDFLAGS += \
	$(LOCAL_PATH)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libGetAudio_format.a \
	$(LOCAL_PATH)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libaacenc.a

ifeq ($(CEDARX_DEBUG_ENABLE),Y)
LOCAL_STATIC_LIBRARIES += \
	libcedara_decoder
endif

ifeq ($(CEDARX_ENABLE_MEMWATCH),Y)
LOCAL_STATIC_LIBRARIES += libmemwatch
endif

ifeq ($(TARGET_OS)-$(TARGET_SIMULATOR),linux-true)
        LOCAL_LDLIBS += -lpthread -ldl
        LOCAL_SHARED_LIBRARIES += libdvm
        LOCAL_CPPFLAGS += -DANDROID_SIMULATOR
endif

ifneq ($(TARGET_SIMULATOR),true)
LOCAL_SHARED_LIBRARIES += libdl
endif

ifeq ($(TARGET_OS)-$(TARGET_SIMULATOR),linux-true)
        LOCAL_LDLIBS += -lpthread
endif

LOCAL_CFLAGS += -Wno-multichar 

LOCAL_CFLAGS += $(CEDARX_EXT_CFLAGS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= libCedarA

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))


















ifneq ($(CEDARX_DEBUG_ENABLE),N)

include $(CLEAR_VARS)


include frameworks/base/media/CedarX-Projects/Config.mk
ifeq ($(CEDARX_ANDROID_VERSION),4)
CEDARA_VERSION_TAG = _
else
CEDARA_VERSION_TAG = _ICS_
LOCAL_CFLAGS += -D__ENABLE_AC3DTSRAW
endif

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
	CedarADecoder.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include \
	${CEDARX_TOP}/libutil \
	${CEDARX_TOP}/libcodecs/include \
	${CEDARX_TOP}/include \
	${CEDARX_TOP}/include/include_base

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

LOCAL_CFLAGS += $(CEDARX_EXT_CFLAGS)   -D__ENABLE_AC3DTS

ifeq ($(CEDARX_DEBUG_FRAMEWORK),Y)
LOCAL_SHARED_LIBRARIES += libcedarxbase 
else
LOCAL_LDFLAGS += \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB$(CEDARA_VERSION_TAG)$(CEDARX_CHIP_VERSION)/libcedarxbase.so
endif

ifeq ($(CEDARX_DEBUG_CEDARV),Y)
LOCAL_SHARED_LIBRARIES += libcedarxosal 
else
LOCAL_LDFLAGS += \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB$(CEDARA_VERSION_TAG)$(CEDARX_CHIP_VERSION)/libcedarxosal.so
endif

LOCAL_LDFLAGS += \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libac3.a \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libdts.a 
ifeq ($(CEDARX_ANDROID_VERSION),4)

else
LOCAL_LDFLAGS += \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libac3_raw.a \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libdts_raw.a

endif	

LOCAL_MODULE:= libswa2

include $(BUILD_SHARED_LIBRARY)
#######################################################################################

include $(CLEAR_VARS)


include frameworks/base/media/CedarX-Projects/Config.mk
ifeq ($(CEDARX_ANDROID_VERSION),4)
CEDARA_VERSION_TAG = _
else
CEDARA_VERSION_TAG = _ICS_
endif

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
	CedarADecoder.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include \
	${CEDARX_TOP}/libutil \
	${CEDARX_TOP}/libcodecs/include \
	${CEDARX_TOP}/include \
	${CEDARX_TOP}/include/include_base

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

LOCAL_CFLAGS += $(CEDARX_EXT_CFLAGS)   -D__ENABLE_OTHER

ifeq ($(CEDARX_DEBUG_FRAMEWORK),Y)
LOCAL_SHARED_LIBRARIES += libcedarxbase 
else
LOCAL_LDFLAGS += \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB$(CEDARA_VERSION_TAG)$(CEDARX_CHIP_VERSION)/libcedarxbase.so
endif

ifeq ($(CEDARX_DEBUG_CEDARV),Y)
LOCAL_SHARED_LIBRARIES += libcedarxosal 
else
LOCAL_LDFLAGS += \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB$(CEDARA_VERSION_TAG)$(CEDARX_CHIP_VERSION)/libcedarxosal.so
endif

LOCAL_LDFLAGS += \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libwma.a \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libaac.a \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libmp3.a \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libatrc.a \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libcook.a \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libsipr.a \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libamr.a \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libape.a \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libogg.a \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libflac.a \
	$(CEDARX_TOP)/../CedarAndroidLib/LIB_$(CEDAR_AUDIOLIB_PATH)/libwav.a
	

LOCAL_MODULE:= libaw_audioa

include $(BUILD_SHARED_LIBRARY)

endif
