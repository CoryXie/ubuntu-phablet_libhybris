LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	media_compatibility_layer.cpp

LOCAL_MODULE:= libmedia_compat_layer
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	canonical/hybris/compat/surface_flinger \
	canonical/hybris/compat/input \
	frameworks/base/media/libstagefright/include \
	frameworks/base/include/media/stagefright/openmax \
	frameworks/base/include/media

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libhardware \
	libui \
	libgui \
	libstagefright \
	libmedia

#LOCAL_STATIC_LIBRARIES := libstagefright_aacdec_omx

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=		\
	test_player.cpp \

LOCAL_MODULE:= test_player
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	bionic \
	bionic/libstdc++/include \
	external/gtest/include \
	external/stlport/stlport \
	external/skia/include/core \
	canonical/hybris/compat/surface_flinger \
	canonical/hybris/compat/input \
	frameworks/base/include

LOCAL_SHARED_LIBRARIES := \
	libis_compat_layer \
	libsf_compat_layer \
	libmedia_compat_layer \
	libcutils \
	libutils \
	libbinder \
	libhardware \
	libui \
	libgui \
	libEGL \
	libGLESv2

include $(BUILD_EXECUTABLE)
