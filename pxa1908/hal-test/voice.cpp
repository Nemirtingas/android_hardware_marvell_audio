#define LOG_TAG "voice"

#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <cutils/log.h>
#include <cutils/str_parms.h>

#include "audio_hw.h"
#include "voice.h"
//#include "voice_extn/voice_extn.h"
#include "platform.h"

struct pcm_config pcm_config_voice_call = {
    .channels = 1,
    .rate = 8000,
    .period_size = 160,
    .period_count = 2,
    .format = PCM_FORMAT_S16_LE,
    .start_threshold = 0,
    .stop_threshold = 0,
    .silence_threshold = 0,
    .avail_min = 0
};

static bool voice_is_sidetone_device(snd_device_t out_device, char *mixer_path)
{
    bool is_sidetone_dev = true;

    switch (out_device)
    {
        case SND_DEVICE_OUT_VOICE_HAC_HANDSET:
            strlcpy(mixer_path, "sidetone-hac-handset", MIXER_PATH_MAX_LENGTH);
            break;
        case SND_DEVICE_OUT_VOICE_HANDSET:
            strlcpy(mixer_path, "sidetone-handset", MIXER_PATH_MAX_LENGTH);
            break;
        case SND_DEVICE_OUT_VOICE_HEADPHONES:
            strlcpy(mixer_path, "sidetone-headphones", MIXER_PATH_MAX_LENGTH);
            break;
        default:
            is_sidetone_dev = false;
            break;
    }

    return is_sidetone_dev;
}

int voice_start_usecase(struct audio_device *adev, audio_usecase_t usecase_id)
{
    return -ENOSYS;
}

int voice_stop_usecase(struct audio_device *adev, audio_usecase_t usecase_id)
{
    return -ENOSYS;
}

int voice_start_call(struct audio_device *adev)
{
    return -ENOSYS;
}

int voice_stop_call(struct audio_device *adev)
{
    return -ENOSYS;
}

int voice_set_parameters(struct audio_device *adev, struct str_parms *parms)
{
    return -ENOSYS;
}

void voice_get_parameters(struct audio_device *adev, struct str_parms *query, struct str_parms *reply)
{

}

void voice_init(audio_device *adev)
{
    int i = 0;

    memset(&adev->_voice, 0, sizeof(adev->_voice));
    adev->_voice.tty_mode = TTY_MODE_OFF;
    adev->_voice.hac = false;
    adev->_voice.volume = 1.0f;
    adev->_voice.mic_mute = false;
    adev->_voice.in_call = false;
    for (i = 0; i < MAX_VOICE_SESSIONS; i++)
        {
        adev->_voice.session[i].pcm_rx = NULL;
        adev->_voice.session[i].pcm_tx = NULL;
        adev->_voice.session[i].state.current = CALL_INACTIVE;
        adev->_voice.session[i].state.new_state = CALL_INACTIVE;
        adev->_voice.session[i].vsid = VOICE_VSID;
    }

    //voice_extn_init(adev);
}

bool voice_is_in_call_rec_stream(struct stream_in *in)
{
    return false;
}

int voice_set_mic_mute(struct audio_device *dev, bool state)
{
    return -ENOSYS;
}

bool voice_get_mic_mute(struct audio_device *dev)
{
    return false;
}

int voice_set_volume(struct audio_device *adev, float volume)
{
    return -ENOSYS;
}

int voice_check_and_set_incall_rec_usecase(struct audio_device *adev,
                                           struct stream_in *in)
{
    return -ENOSYS;
}

int voice_check_and_set_incall_music_usecase(struct audio_device *adev,
                                             struct stream_out *out)
{
    return -ENOSYS;
}

int voice_check_and_stop_incall_rec_usecase(struct audio_device *adev,
                                            struct stream_in *in)
{
    return -ENOSYS;
}

void voice_update_devices_for_all_voice_usecases(struct audio_device *adev)
{

}

void voice_set_sidetone(audio_device *adev, snd_device_t out_snd_device, bool enable)
{
    char mixer_path[MIXER_PATH_MAX_LENGTH];
    bool is_sidetone_dev;
    ALOGD("%s: %s, out_snd_device: %d\n",
          __func__, (enable ? "enable" : "disable"),
          out_snd_device);

    is_sidetone_dev = voice_is_sidetone_device(out_snd_device, mixer_path);

    if (!is_sidetone_dev)
    {
        ALOGD("%s: device %d does not support sidetone\n",
              __func__, out_snd_device);
        return;
    }

    ALOGD("%s: sidetone out device = %s\n",
          __func__, mixer_path);

    if (enable)
        audio_route_apply_and_update_path(adev->_audio_route, mixer_path);
    else
        audio_route_reset_and_update_path(adev->_audio_route, mixer_path);

    return;
}

bool voice_is_in_call(audio_device *adev)
{
    return adev->_voice.in_call;
}

bool voice_is_call_state_active(struct audio_device *adev)
{
    bool call_state = false;
    int ret = 0;

    //ret = voice_extn_is_call_state_active(adev, &call_state);
    //if (ret == -ENOSYS) {
        call_state = (adev->_voice.session[VOICE_SESS_IDX].state.current == CALL_ACTIVE) ? true : false;
    //}

    return call_state;
}
