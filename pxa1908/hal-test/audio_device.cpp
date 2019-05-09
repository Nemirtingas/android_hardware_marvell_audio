#include "audio_device.h"
#include "voice.h"

audio_device::audio_device()
{
}

audio_device::~audio_device()
{

}

hw_device_t* audio_device::get_hw_device()
{
    return &_device.common;
}

void audio_device::add_usecase(audio_usecase &&uc)
{
    _usecase_list.push_back(uc);
}

void audio_device::del_usecase(audio_usecase_t uc_id)
{
    std::list<audio_usecase>::iterator it;

    for( it = _usecase_list.begin(); it != _usecase_list.end(); ++it )
    {
        if( it->id == uc_id )
        {
            break;
        }
    }
    if( it != _usecase_list.end() )
    {
        disable_audio_route(&(*it));

        disable_snd_device(it->out_snd_device);

        _usecase_list.erase(it);
    }
}

audio_usecase * audio_device::get_usecase_from_list(audio_usecase_t uc_id)
{
    for( auto& item : _usecase_list )
        if( item.id == uc_id )
            return &item;

    return nullptr;
}

audio_usecase_t audio_device::get_voice_usecase_id_from_list()
{
    for( auto& item : _usecase_list )
        if( item.type == VOICE_CALL )
            return item.id;

    return USECASE_INVALID;
}

/*
  ID_IPATH_INVALID = -1,
  ID_IPATH_RX_MIN,
  ID_IPATH_RX_HIFI_LL = 1  // low latency  "HiFi1Playback"
  ID_IPATH_RX_HIFI_DB = 2  // deep buffer  "HiFi2Playback"
  ID_IPATH_RX_VC      = 3  // voice call   "VoicePlayback"
  ID_IPATH_RX_VC_ST   = 4  // sidetone     "VoicePlaybackST"
  ID_IPATH_RX_HFP     = 5  // HFP          "HfpPlayback"
  ID_IPATH_RX_FM      = 6  // FM           "FMPlaybackI2S"
  ID_IPATH_RX_MAX     = 6
  ID_IPATH_TX_MIN     = 7
  ID_IPATH_TX_HIFI    = 7                  "HiFi1Record"
  ID_IPATH_TX_VC      = 8  // voice call   "VoiceRecord"
  ID_IPATH_TX_HFP     = 9  // HFP          "HfpRecord"
  ID_IPATH_TX_FM      = 10 // FM recording "FMI2SRecord"
  ID_IPATH_TX_MAX     = 10

  // Audio path operation methods
#define INVALID_METHOD_ID 0xFFFFFFFF
#define METHOD_ENABLE 0x00000001
#define METHOD_DISABLE 0x00000002
#define METHOD_MUTE 0x00000003
#define METHOD_VOLUME_SET 0x00000004
#define METHOD_SWITCH 0x00000005

// Audio Hardware devices, relative with APU and codec paths
// Output Device must be defined by order, 1 bit 1 step
#define HWDEV_INVALID 0xFFFFFFFF
#define HWDEV_OUT_BASE 0x00000001
#define HWDEV_EARPIECE (HWDEV_OUT_BASE << 0)
#define HWDEV_SPEAKER (HWDEV_OUT_BASE << 1)
#define HWDEV_STEREOSPEAKER (HWDEV_OUT_BASE << 2)
#define HWDEV_HEADPHONE (HWDEV_OUT_BASE << 3)
#define HWDEV_BLUETOOTH_NB (HWDEV_OUT_BASE << 4)
#define HWDEV_BT_NREC_OFF_NB (HWDEV_OUT_BASE << 5)
#define HWDEV_BLUETOOTH_WB (HWDEV_OUT_BASE << 6)
#define HWDEV_BT_NREC_OFF_WB (HWDEV_OUT_BASE << 7)
#define HWDEV_OUT_TTY (HWDEV_OUT_BASE << 8)
#define HWDEV_OUT_TTY_HCO_SPEAKER (HWDEV_OUT_BASE << 9)
#define HWDEV_OUT_TTY_HCO_STEREOSPEAKER (HWDEV_OUT_BASE << 10)

// Input Device must be defined by order, 1 bit 1 step
#define HWDEV_BIT_IN 0x80000000
#define HWDEV_IN_BASE 0x00000001
#define HWDEV_AMIC1 (HWDEV_BIT_IN | (HWDEV_IN_BASE << 0))
#define HWDEV_AMIC2 (HWDEV_BIT_IN | (HWDEV_IN_BASE << 1))
#define HWDEV_DMIC1 (HWDEV_BIT_IN | (HWDEV_IN_BASE << 2))
#define HWDEV_DMIC2 (HWDEV_BIT_IN | (HWDEV_IN_BASE << 3))
#define HWDEV_AMIC1_SPK_MODE (HWDEV_BIT_IN | (HWDEV_IN_BASE << 4))
#define HWDEV_AMIC2_SPK_MODE (HWDEV_BIT_IN | (HWDEV_IN_BASE << 5))
#define HWDEV_DUAL_DMIC1 (HWDEV_BIT_IN | (HWDEV_IN_BASE << 6))
#define HWDEV_HSMIC (HWDEV_BIT_IN | (HWDEV_IN_BASE << 7))
#define HWDEV_BTMIC_NB (HWDEV_BIT_IN | (HWDEV_IN_BASE << 8))
#define HWDEV_BTMIC_NREC_OFF_NB (HWDEV_BIT_IN | (HWDEV_IN_BASE << 9))
#define HWDEV_BTMIC_WB (HWDEV_BIT_IN | (HWDEV_IN_BASE << 10))
#define HWDEV_BTMIC_NREC_OFF_WB (HWDEV_BIT_IN | (HWDEV_IN_BASE << 11))
#define HWDEV_DUAL_AMIC (HWDEV_BIT_IN | (HWDEV_IN_BASE << 12))
#define HWDEV_DUAL_AMIC_SPK_MODE (HWDEV_BIT_IN | (HWDEV_IN_BASE << 13))
#define HWDEV_IN_TTY (HWDEV_BIT_IN | (HWDEV_IN_BASE << 14))
#define HWDEV_IN_TTY_VCO (HWDEV_BIT_IN | (HWDEV_IN_BASE << 15))
#define HWDEV_IN_TTY_VCO_AMIC1 (HWDEV_BIT_IN | (HWDEV_IN_BASE << 16))
#define HWDEV_IN_TTY_VCO_AMIC2 (HWDEV_BIT_IN | (HWDEV_IN_BASE << 17))
#define HWDEV_IN_TTY_VCO_DUAL_AMIC (HWDEV_BIT_IN | (HWDEV_IN_BASE << 18))
#define HWDEV_IN_TTY_VCO_DUAL_AMIC_SPK_MODE \
  (HWDEV_BIT_IN | (HWDEV_IN_BASE << 19))
#define HWDEV_IN_TTY_VCO_DUAL_DMIC1 (HWDEV_BIT_IN | (HWDEV_IN_BASE << 20))

// Mic Name, used to parse platform config xml
static char *input_devname[] = {
    "AMIC1", "AMIC2", "DMIC1", "DMIC2", "AMIC1_SPK_MODE", "AMIC2_SPK_MODE",
    "DUAL_DMIC1", "HSMIC", "BTMIC_NB", "BTMIC_NREC_OFF_NB", "BTMIC_WB",
    "BTMIC_NREC_OFF_WB", "DUAL_AMIC", "DUAL_AMIC_SPK_MODE"};

enum mic_modes {
  MIC_MODE_NONE = 0,
  MIC_MODE_MIC1,
  MIC_MODE_MIC2,
  MIC_MODE_DUALMIC
};

METHOD_ENABLE  = 1
METHOD_DISABLE = 2

select_interface(madev, ID_IPATH_RX_HIFI_LL, true)
{
    [enable]
    {
        route_interface(ipath, METHOD_ENABLE);
        select_virtual_path()
        {
            get_active_vrtl_path(madev, &active_v_path);
            route_vrtl_path(tmp_vpath->v_mode, hw_dev, METHOD_ENABLE, flag, value)
            {
                path = get_vrtl_path(v_mode, hw_dev, flag); // HiFi1PlaybackToSpeaker
                handle_ctl_info(path, enable, value);
                ACMAudioPathEnable(path_name, val);
            }
        }
    }
    [disable]
    {
        route_devices(madev, mrvl_path_manager.active_out_device, METHOD_DISABLE, 0);
        route_interface(ipath, METHOD_DISABLE);
    }
}
*/

int audio_device::select_devices(audio_usecase_t uc_id)
{
    int status = 0;
    snd_device_t out_snd_device = SND_DEVICE_NONE;
    snd_device_t in_snd_device = SND_DEVICE_NONE;

    audio_usecase *vc_usecase;
    audio_usecase *usecase;

    usecase = get_usecase_from_list(uc_id);
    if( usecase == nullptr )
    {
        ALOGE("%s: Could not find the usecase(%d)", __func__, uc_id);
        return -EINVAL;
    }

    if( usecase->type == VOICE_CALL ||
        usecase->type == PCM_HFP_CALL )
    {
        out_snd_device = platform_get_output_snd_device(_platform, usecase->stream.out->_devices);
        in_snd_device = platform_get_input_snd_device(_platform, usecase->stream.out->_devices);
        usecase->devices = usecase->stream.out->_devices;
    }
    else
    {
        if( voice_is_in_call(this) )
        {
            vc_usecase = get_usecase_from_list(get_voice_usecase_id_from_list());
            if( vc_usecase != nullptr &&
                ((vc_usecase->devices & AUDIO_DEVICE_OUT_ALL_CODEC_BACKEND) ||
                (usecase->devices == AUDIO_DEVICE_IN_VOICE_CALL)))
            {
                in_snd_device = vc_usecase->in_snd_device;
                out_snd_device = vc_usecase->out_snd_device;
            }
        }
        //else if (audio_extn_hfp_is_active(adev))
        //{
        //    hfp_ucid = audio_extn_hfp_get_usecase();
        //    hfp_usecase = get_usecase_from_list(hfp_ucid);
        //    if( hfp_usecase->devices & AUDIO_DEVICE_OUT_ALL_CODEC_BACKEND)
        //    {
        //           in_snd_device = hfp_usecase->in_snd_device;
        //           out_snd_device = hfp_usecase->out_snd_device;
        //    }
        //}
        if( usecase->type == PCM_PLAYBACK )
        {
            usecase->devices = usecase->stream.out->_devices;
            in_snd_device = SND_DEVICE_NONE;
            if( out_snd_device == SND_DEVICE_NONE )
            {
                out_snd_device = platform_get_output_snd_device(_platform, usecase->stream.out->_devices);
                if( usecase->stream.out == _primary_output &&
                    _active_input &&
                   (_active_input->_source == AUDIO_SOURCE_VOICE_COMMUNICATION ||
                    _mode == AUDIO_MODE_IN_COMMUNICATION) &&
                    out_snd_device != usecase->out_snd_device )
                {
                    select_devices(_active_input->_usecase);
                }
            }
        }
        else if( usecase->type == PCM_CAPTURE )
        {
            usecase->devices = usecase->stream.in->_device;
            out_snd_device = SND_DEVICE_NONE;
            if (in_snd_device == SND_DEVICE_NONE)
            {
                audio_devices_t out_device = AUDIO_DEVICE_NONE;
                if( _active_input &&
                   (_active_input->_source == AUDIO_SOURCE_VOICE_COMMUNICATION ||
                    _mode == AUDIO_MODE_IN_COMMUNICATION))
                {
                    platform_set_echo_reference(this, false, AUDIO_DEVICE_NONE);
                    if( usecase->id == USECASE_AUDIO_RECORD_AFE_PROXY )
                    {
                        out_device = AUDIO_DEVICE_OUT_TELEPHONY_TX;
                    }
                    else if( _primary_output )
                    {
                        out_device = _primary_output->_devices;
                    }
                }
                in_snd_device = platform_get_input_snd_device(_platform, out_device);
            }
        }
    }

    if( out_snd_device == usecase->out_snd_device &&
        in_snd_device == usecase->in_snd_device )
    {
        return 0;
    }

    ALOGD("%s: out_snd_device(%d: %s) in_snd_device(%d: %s)", __func__,
        out_snd_device, platform_get_snd_device_name(out_snd_device),
        in_snd_device,  platform_get_snd_device_name(in_snd_device));
    // Limitation: While in call, to do a device switch we need to disable
    // and enable both RX and TX devices though one of them is same as current
    // device.
    if ((usecase->type == VOICE_CALL) &&
        (usecase->in_snd_device != SND_DEVICE_NONE) &&
        (usecase->out_snd_device != SND_DEVICE_NONE))
    {
        status = platform_switch_voice_call_device_pre(_platform);
        // Disable sidetone only if voice call already exists
        if( voice_is_call_state_active(this) )
            voice_set_sidetone(this, usecase->out_snd_device, false);
    }

    // Disable current sound devices
    if (usecase->out_snd_device != SND_DEVICE_NONE)
    {
        disable_audio_route(usecase);
        disable_snd_device(usecase->out_snd_device);
    }

    if (usecase->in_snd_device != SND_DEVICE_NONE)
    {
        disable_audio_route(usecase);
        disable_snd_device(usecase->in_snd_device);
    }

    // Applicable only on the targets that has external modem.
    // New device information should be sent to modem before enabling
    // the devices to reduce in-call device switch time.
    //
    if ((usecase->type == VOICE_CALL) &&
        (usecase->in_snd_device != SND_DEVICE_NONE) &&
        (usecase->out_snd_device != SND_DEVICE_NONE))
    {
        status = platform_switch_voice_call_enable_device_config(_platform,
                                                                 out_snd_device,
                                                                 in_snd_device);
    }

    // Enable new sound devices
    if (out_snd_device != SND_DEVICE_NONE)
    {
        if (usecase->devices & AUDIO_DEVICE_OUT_ALL_CODEC_BACKEND)
            check_and_route_playback_usecases(usecase, out_snd_device);
        enable_snd_device(out_snd_device);
    }

    if (in_snd_device != SND_DEVICE_NONE) {
        check_and_route_capture_usecases(usecase, in_snd_device);
        enable_snd_device(in_snd_device);
    }

    if (usecase->type == VOICE_CALL)
        status = platform_switch_voice_call_device_post(_platform,
                                                        out_snd_device,
                                                        in_snd_device);

    usecase->in_snd_device = in_snd_device;
    usecase->out_snd_device = out_snd_device;

    enable_audio_route(usecase);

    // Applicable only on the targets that has external modem.
    //Enable device command should be sent to modem only after
    //enabling voice call mixer controls
    if (usecase->type == VOICE_CALL) {
        status = platform_switch_voice_call_usecase_route_post(_platform,
                                                               out_snd_device,
                                                               in_snd_device);
         // Enable sidetone only if voice call already exists
        if (voice_is_call_state_active(this))
            voice_set_sidetone(this, out_snd_device, true);
    }

    return status;
}

bool audio_device::output_drives_call(stream_output *out)
{
    return _primary_output == out || _voice_tx_output == out;
}

void audio_device::check_and_route_playback_usecases(audio_usecase *uc_info, snd_device_t snd_device)
{
    bool switch_device[AUDIO_USECASE_MAX];
    int i, num_uc_to_switch = 0;

    /*
     * This function is to make sure that all the usecases that are active on
     * the hardware codec backend are always routed to any one device that is
     * handled by the hardware codec.
     * For example, if low-latency and deep-buffer usecases are currently active
     * on speaker and out_set_parameters(headset) is received on low-latency
     * output, then we have to make sure deep-buffer is also switched to headset,
     * because of the limitation that both the devices cannot be enabled
     * at the same time as they share the same backend.
     */
    /* Disable all the usecases on the shared backend other than the
       specified usecase */
    for (i = 0; i < AUDIO_USECASE_MAX; i++)
        switch_device[i] = false;

    for( auto &i : _usecase_list )
    {
        if( i.type != PCM_CAPTURE &&
           &i != uc_info &&
            i.out_snd_device != snd_device &&
            i.devices & AUDIO_DEVICE_OUT_ALL_CODEC_BACKEND &&
            platform_check_backends_match(snd_device, i.out_snd_device))
        {
            ALOGV("%s: Usecase (%s) is active on (%s) - disabling ..",
                  __func__, _use_case_table[i.id],
                  platform_get_snd_device_name(i.out_snd_device));
            disable_audio_route(&i);
            switch_device[i.id] = true;
            num_uc_to_switch++;
        }
    }

    if (num_uc_to_switch)
    {
        for( auto &i : _usecase_list )
        {
            if( switch_device[i.id] )
            {
                disable_snd_device(i.out_snd_device);
            }
        }

        for( auto &i : _usecase_list )
        {
            if( switch_device[i.id] )
            {
                enable_snd_device(snd_device);
            }
        }

        for( auto &i : _usecase_list )
        {
            if (switch_device[i.id] )
            {
                i.out_snd_device = snd_device;
                enable_audio_route(&i);
            }
        }
    }
}

void audio_device::check_and_route_capture_usecases(audio_usecase *uc_info, snd_device_t snd_device)
{

}

int audio_device::disable_audio_route(audio_usecase *usecase)
{
    //ALOGE("%s: NOT IMPLEMENTED", __func__);
    return -ENOSYS;
}

int audio_device::disable_snd_device(snd_device_t snd_device)
{
    //ALOGE("%s: NOT IMPLEMENTED", __func__);
    return -ENOSYS;
}

int audio_device::enable_snd_device(snd_device_t snd_device)
{
    int i, num_devices = 0;
    snd_device_t new_snd_devices[2];

    if (snd_device < SND_DEVICE_MIN ||
        snd_device >= SND_DEVICE_MAX)
    {
        ALOGE("%s: Invalid sound device %d", __func__, snd_device);
        return -EINVAL;
    }

    platform_send_audio_calibration(_platform, snd_device);

    _snd_dev_ref_cnt[snd_device]++;
    if (_snd_dev_ref_cnt[snd_device] > 1)
    {
        ALOGV("%s: snd_device(%d: %s) is already active",
              __func__, snd_device, platform_get_snd_device_name(snd_device));
        return 0;
    }

    // due to the possibility of calibration overwrite between listen
    //   and audio, notify sound trigger hal before audio calibration is sent
    //audio_extn_sound_trigger_update_device_status(snd_device, ST_EVENT_SND_DEVICE_BUSY);

    //if (audio_extn_spkr_prot_is_enabled())
    //     audio_extn_spkr_prot_calib_cancel(adev);

    //audio_extn_dsm_feedback_enable(adev, snd_device, true);

    /*
    if ((snd_device == SND_DEVICE_OUT_SPEAKER ||
        snd_device == SND_DEVICE_OUT_VOICE_SPEAKER) &&
        audio_extn_spkr_prot_is_enabled())
    {
        if (audio_extn_spkr_prot_get_acdb_id(snd_device) < 0) {
            adev->snd_dev_ref_cnt[snd_device]--;
            return -EINVAL;
        }
        if (audio_extn_spkr_prot_start_processing(snd_device)) {
            ALOGE("%s: spkr_start_processing failed", __func__);
            return -EINVAL;
        }
    }
    else*/
    if(platform_can_split_snd_device(snd_device, &num_devices, new_snd_devices))
    {
        for (i = 0; i < num_devices; i++) {
            enable_snd_device(new_snd_devices[i]);
        }
        platform_set_speaker_gain_in_combo(this, snd_device, true);
    }
    else
    {
        const char * dev_path = platform_get_snd_device_name(snd_device);
        ALOGD("%s: snd_device(%d: %s)", __func__, snd_device, dev_path);
        audio_route_apply_and_update_path(_audio_route, dev_path);
    }

    return 0;
}

int audio_device::enable_audio_route(audio_usecase *usecase)
{
    snd_device_t snd_device;
    char mixer_path[50];

    if (usecase == NULL)
        return -EINVAL;

    ALOGV("%s: enter: usecase(%d)", __func__, usecase->id);

    if (usecase->type == PCM_CAPTURE)
        snd_device = usecase->in_snd_device;
    else
        snd_device = usecase->out_snd_device;

    strcpy(mixer_path, use_case_table[usecase->id].c_str());
    platform_add_backend_name(_platform, mixer_path, snd_device);
    ALOGD("%s: apply and update mixer path: %s", __func__, mixer_path);



    audio_route_apply_and_update_path(_audio_route, mixer_path);

    ALOGV("%s: exit", __func__);
    return 0;
}

int audio_device::init(const hw_module_t *module, const char *name, hw_device_t **dev)
{
    _device.common.tag = HARDWARE_DEVICE_TAG;
    _device.common.version = AUDIO_DEVICE_API_VERSION_2_0;
    _device.common.module = (hw_module_t*)module;

    _active_input = nullptr;
    _primary_output = nullptr;
    _voice_tx_output = nullptr;
    _current_call_output = nullptr;
    _snd_card = 0;
    _mode = AUDIO_MODE_NORMAL;
    _enable_voicerx = false;
    _bt_wb_speech_enabled = false;
    _bluetooth_nrec = true;

    for( int i = 0; i < (SND_DEVICE_MAX-1); ++i )
    {
        _snd_dev_ref_cnt[i] = 0;
    }

    voice_init(this);

    _platform = platform_init(this);
    if (_platform == nullptr)
    {
        ALOGE("%s: Failed to init platform data, aborting.", __func__);
        *dev = nullptr;
        return -EINVAL;
    }

    *dev = (hw_device_t*)&_device;

    return 0;
}

int audio_device::close()
{
    platform_deinit(_platform);
    return 0;
}

int audio_device::get_mic_mute(bool *state)
{
    *state = _mic_muted;
    return 0;
}

int audio_device::get_input_buffer_size(const audio_config *config)
{
    return -ENOSYS;
}

int audio_device::dump(int fd)
{
    return 0;
}
