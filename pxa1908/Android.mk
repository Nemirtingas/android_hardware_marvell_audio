LOCAL_PATH	:= $(call my-dir)

audio-hals += hal
audio-hals += libvcm libacm libacoustic

include $(call all-named-subdir-makefiles,$(audio-hals))
