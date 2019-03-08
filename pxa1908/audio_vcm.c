#include <vcm.h>

#define LOG_TAG "voice_control"
#include <utils/Log.h>
#include <stdint.h>

#define true 1
#define false 0
typedef uint8_t bool;

static bool vcmInitialised = false;
static int32_t recording_stream = 0;

int convert2_profile(int arg1, int arg2)
{
    int result;

  if ( arg1 == 0x40 )
    goto LABEL_32;
  if ( arg1 > 0x40 )
  {
    if ( arg1 != 0x80000004 )
    {
      if ( arg1 <= 0x80000004 )
      {
        if ( arg1 == 2048 || arg1 == 4096 )
          return 2;
        goto LABEL_48;
      }
      if ( arg1 == 0x80000010 )
        goto LABEL_41;
      if ( arg1 != 0x80000080 )
      {
        if ( arg1 != 0x80000008 )
        {
LABEL_48:
          ALOGI("%s: Invalid input of audio device 0x%x", "convert2_profile", arg1);
          return 0;
        }
        goto LABEL_32;
      }
    }
LABEL_22:
    if ( arg2 & 0x80 )
      return 11;
    if ( arg2 & 1 )
      return 14;
    return 1;
  }
  if ( arg1 == 4 )
  {
LABEL_41:
    if ( arg2 & 8 )
      return 18;
    if ( arg2 & 0x10 )
      return 19;
    if ( arg2 & 0x20 )
      return 20;
    if ( arg2 & 0x40 )
      return 21;
    return 2;
  }
  if ( arg1 > 4 )
  {
    if ( arg1 != 16 && arg1 != 32 )
    {
      if ( arg1 != 8 )
        goto LABEL_48;
      return 7;
    }
LABEL_32:
    if ( arg2 & 4 )
    {
      if ( arg2 & 2 )
        result = 10;
      else
        result = 9;
    }
    else if ( arg2 & 2 )
    {
      result = 8;
    }
    else
    {
      result = 4;
    }
    return result;
  }
  if ( arg1 == 1 )
    goto LABEL_22;
  if ( arg1 != 2 )
    goto LABEL_48;
  if ( arg2 & 0x80 )
    return 13;
  if ( arg2 & 1 )
    return 15;

  return 3;
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

void vcm_select_path(int32_t device, int a2, int params)
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

int vcm_set_loopback(int arg1, bool arg2)
{
    int res;
    
    res = vcm_check_init();
    if( !res )
    {
        int profile = convert2_profile(arg1, 0);
        res = VCMAudioProfileSet(0, profile, (arg2?2:1));
    }
    return res;
}
