# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH:= $(call my-dir)

# build audio.primary.mrvl.so
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    audio_device.cpp \
    stream_input.cpp \
    stream_output.cpp \
    audio_hw.cpp \
    common.cpp \
    platform.cpp \
    platform_info.cpp \
    voice.cpp \

LOCAL_C_INCLUDES += \
    external/libxml2/include \
    external/expat/lib \
    external/icu/icu4c/source/common \
    external/tinyalsa/include/ \
    system/media/audio/include \
	$(call include-path-for, audio-utils) \
	$(call include-path-for, audio-route) \
	$(call include-path-for, audio-effects) \
    frameworks/av/include

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libtinyalsa \
    libhardware \
    libaudioutils \
    libaudioroute \
    libeffects \
    libexpat \
    libacm \
    libxml2

LOCAL_MODULE:= audio.primary.mrvl
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -Wall -Werror -Wno-parentheses
LOCAL_CFLAGS += -DIGNORE_SILENCE_SIZE
#LOCAL_CFLAGS += -Wno-unused-parameter

ifeq ($(BOARD_WITH_SAMSUNG_POSTPROCESS_AUDIO),true)
   LOCAL_CFLAGS += -DSAMSUNG_AUDIO
endif

ifeq ($(BOARD_WITH_MRVL_AEC_AUDIO),true)
   #LOCAL_CFLAGS += -DMRVL_AEC
   #LOCAL_SHARED_LIBRARIES += libeffects
   #LOCAL_SRC_FILES += audio_aec.c
   #LOCAL_C_INCLUDES += frameworks/av/media/libeffects/factory/
endif

ifeq ($(BOARD_WITH_TELEPHONY_AUDIO),true)
   #LOCAL_CFLAGS += -DWITH_TELEPHONY
   #LOCAL_SHARED_LIBRARIES += libvcm
   #LOCAL_SRC_FILES += audio_vcm.c
endif

ifeq ($(strip $(BOARD_WITH_ACOUSTIC)),true)
   #LOCAL_CFLAGS += -DWITH_ACOUSTIC
   #LOCAL_SHARED_LIBRARIES += libacoustic
endif

ifeq ($(strip $(BOARD_ENABLE_ADVANCED_AUDIO)),true)
   #LOCAL_CFLAGS += -DWITH_ADVANCED_AUDIO
endif

ifeq ($(strip $(BOARD_WITH_STEREO_SPKR)),true)
   #LOCAL_CFLAGS += -DWITH_STEREO_SPKR
endif

ifeq ($(strip $(BOARD_WITH_HEADSET_OUTPUT_ONLY)),true)
   #LOCAL_CFLAGS += -DROUTE_SPEAKER_TO_HEADSET
endif

LOCAL_CFLAGS += -DPLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION) -D_POSIX_SOURCE

ifeq ($(strip $(BOARD_AUDIO_COMPONENT_APU)), MAP-LITE)
   LOCAL_CFLAGS += -DWITH_MAP_LITE
endif

include $(BUILD_SHARED_LIBRARY)
