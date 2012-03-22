LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

include frameworks/base/media/CedarX-Projects/Config.mk

LOCAL_PREBUILT_LIBS := libcedarxosal.so libcedarv.so 
ifeq ($(CEDARX_DEBUG_ENABLE),N)
LOCAL_PREBUILT_LIBS += libcedarxbase.so libswdrm.so libstagefright_soft_cedar_h264dec.so
endif
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
