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

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    audio_hw_mrvl_test.cpp

LOCAL_C_INCLUDES += \
    external/libxml2/include \
    external/expat/lib \
    external/icu/icu4c/source/common \
    external/tinyalsa/include/ \
    system/media/audio/include \
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
    libxml2

LOCAL_MODULE:= audio_test
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -Wall -Wno-parentheses -DIGNORE_SILENCE_SIZE
#LOCAL_CFLAGS += -Wno-unused-parameter

include $(BUILD_EXECUTABLE)



include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    tinyalsa_logger.cpp

LOCAL_C_INCLUDES += \
    external/tinyalsa/include/ \

LOCAL_MODULE:= libmytinyal
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
    libcutils \

include $(BUILD_SHARED_LIBRARY)
