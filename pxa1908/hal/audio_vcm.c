#include <vcm.h>

#define LOG_TAG "voice_control"
#include <utils/Log.h>
#include <stdint.h>
#include <system/audio.h>

#include "audio_hw_mrvl.h"

#ifdef DISABLE_LOGS
    #ifdef ALOGE
    #undef ALOGE
    #endif
    #define ALOGE(...) {}
    #ifdef ALOGW
    #undef ALOGW
    #endif
    #define ALOGW(...) {}
    #ifdef ALOGI
    #undef ALOGI
    #endif
    #define ALOGI(...) {}
    #ifdef ALOGV
    #undef ALOGV
    #endif
    #define ALOGV(...) {}
    #ifdef ALOGD
    #undef ALOGD
    #endif
    #define ALOGD(...) {}
#endif

static bool vcmInitialised = false;
static int32_t recording_stream = 0;

int convert2_profile(audio_devices_t device, int vcm_param)
{
    switch( device )
    {
        case AUDIO_DEVICE_OUT_EARPIECE:
        case AUDIO_DEVICE_IN_BUILTIN_MIC:
        case AUDIO_DEVICE_IN_BACK_MIC:
            if( vcm_param & VCM_DUAL_MIC  ) return 11;
            if( vcm_param & VCM_EXTRA_VOL ) return 14;
            return 1;

        case AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET:
        case AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET:
            return 2;

        case AUDIO_DEVICE_IN_WIRED_HEADSET:
        case AUDIO_DEVICE_OUT_WIRED_HEADSET:
            if( vcm_param & VCM_TTY_FULL        ) return 18;
            if( vcm_param & VCM_TTY_HCO         ) return 19;
            if( vcm_param & VCM_TTY_VCO         ) return 20;
            if( vcm_param & VCM_TTY_VCO_DUALMIC ) return 21;
            return 2;

        case AUDIO_DEVICE_OUT_SPEAKER:
            if( vcm_param & VCM_DUAL_MIC  ) return 13;
            if( vcm_param & VCM_EXTRA_VOL ) return 15;
            return 3;

        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET:
            if( vcm_param & VCM_BT_WB )
            {
               if( vcm_param & VCM_BT_NREC_OFF ) return 10;
               return 9;
            }
            if( vcm_param & VCM_BT_NREC_OFF ) return 8;
            return 4;

        case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
            return 7;

    }
    ALOGI("%s: Invalid input of audio device 0x%x", __func__, device);
    return 0;
}

int vcm_check_init()
{
    if( !vcmInitialised )
    {
        int res = VCMInit();
        if( res != 1 )
        {
            ALOGE("%s: VCM init error, error code %d.", __func__, res);
            return -1;
        }
        ALOGI("%s: VCm init success.", __func__);
        vcmInitialised = true;
    }
    return 0;
}

void vcm_mute_all(int direction, int reserved)
{
    if( !vcm_check_init() )
    {
        VCMAudioProfileMute(0, direction, reserved);
        VCMAudioProfileMute(1, direction, reserved);
    }
}

void vcm_mute_mic(int direction, int reserved)
{
    if( !vcm_check_init() )
        VCMAudioProfileMute(0, direction, reserved);
}

void vcm_setvolume(uint8_t input_gain1, uint8_t input_gain2, uint8_t output_gain1, uint8_t output_gain2, uint32_t volume)
{
    ALOGI("%s: input_gain 0x%x output_gain 0x%x volume %d", __func__, input_gain1+37, output_gain1+37, volume);
    VCMAudioProfileVolumeSet(0, input_gain1, input_gain2, volume);
    VCMAudioProfileVolumeSet(1, output_gain1, output_gain2, volume);
}

void vcm_set_user_eq(void* parameter, size_t parameter_size)
{
    if( !vcm_check_init() )
    {
        int res = VCMAudioParameterSet(3, parameter, parameter_size);
        if( res != 1 )
            ALOGE("%s: Failed to set dha parameters %d.", __func__, res);
    }
}

void vcm_select_path(audio_devices_t device, int a2, int params)
{
    ALOGI("%s: device 0x%x", __func__, device);
    if( !vcm_check_init() )
    {
        int profile = convert2_profile(device, params);
        if( profile )
        {
            ALOGI("%s: Enable VC, profile 0x%x", __func__, profile);
            VCMAudioProfileSet(0, profile, 0);
        }
        else
        {
            ALOGE("%s: Failed to get VC profile profile %d since it is invalid", __func__, device);
        }
    }
}

void vcm_recording_stop()
{
    if( recording_stream )
    {
        int res;
        ALOGI("%s: Stop VC recording", __func__);
        res = VCMAudioStreamInStop(recording_stream);
        if( res != 1 )
            ALOGE("%s: Failed to stop VCM stream In %d.", __func__, res);
        recording_stream = 0;
    }
    else
        ALOGE("%s: can't find available stream to stop.", __func__);
}

int vcm_recording_read(void *buffer, size_t size)
{
    size_t buf_size = size;

    if( recording_stream )
    {
        int res = VCMAudioStreamRead(recording_stream, buffer, &buf_size);
        if( res == 1 )
            return buf_size;

        ALOGE("%s: Failed to read stream %d.", __func__, res);
    }
    else
    {
        ALOGE("%s: can't find available stream to read.", __func__);
    }
    return -1;
}

int vcm_recording_start()
{
    int res;
    int32_t stream_id;
    if( recording_stream )
    {
        ALOGE("%s: VCM recording already started error %d.", __func__, recording_stream);
        return -1;
    }

    res = VCMAudioStreamInStart(3, 3, 1, &stream_id);
    if( res != 1 )
    {
        ALOGE("%s: Failed to start VCM stream In %d.", __func__, res);
        return -1;
    }

    ALOGI("%s: Start VCM recording success, id %d", __func__, stream_id);
    recording_stream = stream_id;
    return 0;
}

int vcm_set_loopback(audio_devices_t device, bool arg2)
{
    int res;

    res = vcm_check_init();
    if( !res )
    {
        int profile = convert2_profile(device, 0);
        res = VCMAudioProfileSet(0, profile, (arg2?2:1));
    }
    return res;
}

