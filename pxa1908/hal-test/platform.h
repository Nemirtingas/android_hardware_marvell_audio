#ifndef __INCLUDED_PLATFORM_H__
#define __INCLUDED_PLATFORM_H__

#include <cstdint>

enum {
    FLUENCE_DISABLE,                  /* Target dosent support fluence */
    FLUENCE_ENABLE      = 0x1,        /* Target supports fluence */
    FLUENCE_PRO_ENABLE  = 0x2,        /* Target supports fluence pro */
};

enum {
    SOURCE_MONO_MIC  = 0x1,            /* Target contains 1 mic */
    SOURCE_DUAL_MIC  = 0x2,            /* Target contains 2 mics */
    SOURCE_THREE_MIC = 0x4,            /* Target contains 3 mics */
    SOURCE_QUAD_MIC  = 0x8,            /* Target contains 4 mics */
};

/*
 * Below are the devices for which is back end is same, SLIMBUS_0_RX.
 * All these devices are handled by the internal HW codec. We can
 * enable any one of these devices at any time
 */
#define AUDIO_DEVICE_OUT_ALL_CODEC_BACKEND \
    (AUDIO_DEVICE_OUT_EARPIECE | AUDIO_DEVICE_OUT_SPEAKER | \
     AUDIO_DEVICE_OUT_SPEAKER_SAFE | \
     AUDIO_DEVICE_OUT_WIRED_HEADSET | AUDIO_DEVICE_OUT_WIRED_HEADPHONE | \
     AUDIO_DEVICE_OUT_LINE)

/* Sound devices specific to the platform
 * The DEVICE_OUT_* and DEVICE_IN_* should be mapped to these sound
 * devices to enable corresponding mixer paths
 */
enum {
    SND_DEVICE_NONE = 0,

    /* Playback devices */
    SND_DEVICE_MIN,
    SND_DEVICE_OUT_BEGIN = SND_DEVICE_MIN,
    SND_DEVICE_OUT_HANDSET = SND_DEVICE_OUT_BEGIN,
    SND_DEVICE_OUT_SPEAKER,
    SND_DEVICE_OUT_SPEAKER_REVERSE,
    SND_DEVICE_OUT_SPEAKER_SAFE,
    SND_DEVICE_OUT_HEADPHONES,
    SND_DEVICE_OUT_LINE,
    SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES,
    SND_DEVICE_OUT_SPEAKER_SAFE_AND_HEADPHONES,
    SND_DEVICE_OUT_SPEAKER_AND_LINE,
    SND_DEVICE_OUT_SPEAKER_SAFE_AND_LINE,
    SND_DEVICE_OUT_VOICE_HANDSET,
    SND_DEVICE_OUT_VOICE_SPEAKER,
    SND_DEVICE_OUT_VOICE_HEADPHONES,
    SND_DEVICE_OUT_VOICE_LINE,
    SND_DEVICE_OUT_HDMI,
    SND_DEVICE_OUT_SPEAKER_AND_HDMI,
    SND_DEVICE_OUT_BT_SCO,
    SND_DEVICE_OUT_BT_SCO_WB,
    SND_DEVICE_OUT_VOICE_HANDSET_TMUS,
    SND_DEVICE_OUT_VOICE_TTY_FULL_HEADPHONES,
    SND_DEVICE_OUT_VOICE_TTY_VCO_HEADPHONES,
    SND_DEVICE_OUT_VOICE_TTY_HCO_HANDSET,
    SND_DEVICE_OUT_VOICE_HAC_HANDSET,
    SND_DEVICE_OUT_VOICE_TX,
    SND_DEVICE_OUT_SPEAKER_PROTECTED,
    SND_DEVICE_OUT_VOICE_SPEAKER_PROTECTED,
    SND_DEVICE_OUT_END,

    /*
     * Note: IN_BEGIN should be same as OUT_END because total number of devices
     * SND_DEVICES_MAX should not exceed MAX_RX + MAX_TX devices.
     */
    /* Capture devices */
    SND_DEVICE_IN_BEGIN = SND_DEVICE_OUT_END,
    SND_DEVICE_IN_HANDSET_MIC  = SND_DEVICE_IN_BEGIN,
    SND_DEVICE_IN_HANDSET_MIC_AEC,
    SND_DEVICE_IN_HANDSET_MIC_NS,
    SND_DEVICE_IN_HANDSET_MIC_AEC_NS,
    SND_DEVICE_IN_HANDSET_DMIC,
    SND_DEVICE_IN_HANDSET_DMIC_AEC,
    SND_DEVICE_IN_HANDSET_DMIC_NS,
    SND_DEVICE_IN_HANDSET_DMIC_AEC_NS,
    SND_DEVICE_IN_HANDSET_DMIC_STEREO,

    SND_DEVICE_IN_SPEAKER_MIC,
    SND_DEVICE_IN_SPEAKER_MIC_AEC,
    SND_DEVICE_IN_SPEAKER_MIC_NS,
    SND_DEVICE_IN_SPEAKER_MIC_AEC_NS,
    SND_DEVICE_IN_SPEAKER_DMIC,
    SND_DEVICE_IN_SPEAKER_DMIC_AEC,
    SND_DEVICE_IN_SPEAKER_DMIC_NS,
    SND_DEVICE_IN_SPEAKER_DMIC_AEC_NS,
    SND_DEVICE_IN_SPEAKER_DMIC_STEREO,

    SND_DEVICE_IN_HEADSET_MIC,
    SND_DEVICE_IN_HEADSET_MIC_AEC,

    SND_DEVICE_IN_HDMI_MIC,
    SND_DEVICE_IN_BT_SCO_MIC,
    SND_DEVICE_IN_BT_SCO_MIC_NREC,
    SND_DEVICE_IN_BT_SCO_MIC_WB,
    SND_DEVICE_IN_BT_SCO_MIC_WB_NREC,
    SND_DEVICE_IN_CAMCORDER_MIC,

    SND_DEVICE_IN_VOICE_DMIC,
    SND_DEVICE_IN_VOICE_DMIC_TMUS,
    SND_DEVICE_IN_VOICE_SPEAKER_MIC,
    SND_DEVICE_IN_VOICE_SPEAKER_DMIC,
    SND_DEVICE_IN_VOICE_HEADSET_MIC,
    SND_DEVICE_IN_VOICE_TTY_FULL_HEADSET_MIC,
    SND_DEVICE_IN_VOICE_TTY_VCO_HANDSET_MIC,
    SND_DEVICE_IN_VOICE_TTY_HCO_HEADSET_MIC,

    SND_DEVICE_IN_VOICE_REC_MIC,
    SND_DEVICE_IN_VOICE_REC_MIC_NS,
    SND_DEVICE_IN_VOICE_REC_DMIC_STEREO,
    SND_DEVICE_IN_VOICE_REC_DMIC_FLUENCE,
    SND_DEVICE_IN_VOICE_REC_HEADSET_MIC,

    SND_DEVICE_IN_VOICE_RX,

    SND_DEVICE_IN_THREE_MIC,
    SND_DEVICE_IN_QUAD_MIC,
    SND_DEVICE_IN_CAPTURE_VI_FEEDBACK,

    SND_DEVICE_IN_HANDSET_TMIC,
    SND_DEVICE_IN_HANDSET_QMIC,
    SND_DEVICE_IN_END,

    SND_DEVICE_MAX = SND_DEVICE_IN_END,

};

#define DEFAULT_OUTPUT_SAMPLING_RATE 48000

#define ALL_SESSION_VSID                0xFFFFFFFF
#define DEFAULT_MUTE_RAMP_DURATION_MS   20
#define DEFAULT_VOLUME_RAMP_DURATION_MS 20
#define MIXER_PATH_MAX_LENGTH 100

#define ACDB_ID_VOICE_SPEAKER 15
#define ACDB_ID_VOICE_HANDSET 7
#define ACDB_ID_VOICE_HANDSET_TMUS 88
#define ACDB_ID_VOICE_DMIC_EF_TMUS 89
#define ACDB_ID_HEADSET_MIC_AEC 8

#define MAX_VOL_INDEX 5
#define MIN_VOL_INDEX 0
#define percent_to_index(val, min, max) \
            ((val) * ((max) - (min)) * 0.01 + (min) + .5)

/*
 * tinyAlsa library interprets period size as number of frames
 * one frame = channel_count * sizeof (pcm sample)
 * so if format = 16-bit PCM and channels = Stereo, frame size = 2 ch * 2 = 4 bytes
 * DEEP_BUFFER_OUTPUT_PERIOD_SIZE = 1024 means 1024 * 4 = 4096 bytes
 * We should take care of returning proper size when AudioFlinger queries for
 * the buffer size of an input/output stream
 */

/* 1920 frames(40ms) at 2 buffers gives a good tradeoff between power and latency */
#define DEEP_BUFFER_OUTPUT_PERIOD_SIZE 1920
#define DEEP_BUFFER_OUTPUT_PERIOD_COUNT 2

#define LOW_LATENCY_OUTPUT_PERIOD_SIZE 240
#define LOW_LATENCY_OUTPUT_PERIOD_COUNT 2

#define HDMI_MULTI_PERIOD_SIZE  336
#define HDMI_MULTI_PERIOD_COUNT 8
#define HDMI_MULTI_DEFAULT_CHANNEL_COUNT 6
#define HDMI_MULTI_PERIOD_BYTES (HDMI_MULTI_PERIOD_SIZE * HDMI_MULTI_DEFAULT_CHANNEL_COUNT * 2)

#define AUDIO_CAPTURE_PERIOD_DURATION_MSEC 20
#define AUDIO_CAPTURE_PERIOD_COUNT 2

#define LOW_LATENCY_CAPTURE_SAMPLE_RATE 48000
#define LOW_LATENCY_CAPTURE_PERIOD_SIZE 240
#define LOW_LATENCY_CAPTURE_USE_CASE 1

#define DEEP_BUFFER_PCM_DEVICE 0
#define AUDIO_RECORD_PCM_DEVICE 0
#define MULTIMEDIA2_PCM_DEVICE 1

#define SPKR_PROT_CALIB_RX_PCM_DEVICE 5
#define SPKR_PROT_CALIB_TX_PCM_DEVICE 25

#define MULTIMEDIA3_PCM_DEVICE 4

#define QUAT_MI2S_PCM_DEVICE    44
#define PLAYBACK_OFFLOAD_DEVICE 9
#define LOWLATENCY_PCM_DEVICE 0
#define VOICE_VSID  0x10C01000

#define VOICE_CALL_PCM_DEVICE 2
#define VOICE2_CALL_PCM_DEVICE 22
#define VOLTE_CALL_PCM_DEVICE 14
#define QCHAT_CALL_PCM_DEVICE 20
#define VOWLAN_CALL_PCM_DEVICE 36

#define AFE_PROXY_PLAYBACK_PCM_DEVICE 7
#define AFE_PROXY_RECORD_PCM_DEVICE 8

#define HFP_PCM_RX 5
#ifdef PLATFORM_MSM8x26
#define HFP_SCO_RX 28
#define HFP_ASM_RX_TX 29
#else
#define HFP_SCO_RX 23
#define HFP_ASM_RX_TX 24
#endif

#define PLATFORM_CONFIG_KEY_SOUNDCARD_NAME "snd_card_name"
#define PLATFORM_CONFIG_KEY_MAX_MIC_COUNT "input_mic_max_count"
#define PLATFORM_DEFAULT_MIC_COUNT 2

struct platform_data;
typedef int audio_usecase_t;
typedef int snd_device_t;
// From system/media/audio/include/audio.h
typedef uint32_t audio_devices_t;

platform_data *platform_init(struct audio_device *adev);
void platform_deinit(platform_data *platform);
const char *platform_get_snd_device_name(snd_device_t snd_device);
void platform_add_backend_name(platform_data *platform, char *mixer_path,
                                                    snd_device_t snd_device);
bool platform_send_gain_dep_cal(platform_data *platform, int level);
int platform_get_pcm_device_id(audio_usecase_t usecase, int device_type);
int platform_get_snd_device_index(char *snd_device_index_name);
int platform_set_snd_device_acdb_id(snd_device_t snd_device, unsigned int acdb_id);
int platform_get_snd_device_acdb_id(snd_device_t snd_device);
int platform_send_audio_calibration(platform_data *platform, snd_device_t snd_device);
int platform_switch_voice_call_device_pre(platform_data *platform);
int platform_switch_voice_call_enable_device_config(platform_data *platform,
                                                    snd_device_t out_snd_device,
                                                    snd_device_t in_snd_device);
int platform_switch_voice_call_device_post(platform_data *platform,
                                           snd_device_t out_snd_device,
                                           snd_device_t in_snd_device);
int platform_switch_voice_call_usecase_route_post(platform_data *platform,
                                                  snd_device_t out_snd_device,
                                                  snd_device_t in_snd_device);
int platform_start_voice_call(platform_data *platform, uint32_t vsid);
int platform_stop_voice_call(platform_data *platform, uint32_t vsid);
int platform_set_voice_volume(platform_data *platform, int volume);
void platform_set_speaker_gain_in_combo(struct audio_device *adev,
                                        snd_device_t snd_device,
                                        bool enable);
int platform_set_mic_mute(platform_data *platform, bool state);
int platform_get_sample_rate(platform_data *platform, uint32_t *rate);
int platform_set_device_mute(platform_data *platform, bool state, char *dir);
snd_device_t platform_get_output_snd_device(platform_data *platform, audio_devices_t devices);
snd_device_t platform_get_input_snd_device(platform_data *platform, audio_devices_t out_device);
int platform_set_hdmi_channels(platform_data *platform, int channel_count);
int platform_edid_get_max_channels(platform_data *platform);
void platform_add_operator_specific_device(snd_device_t snd_device,
                                           const char *op,
                                           const char *mixer_path,
                                           unsigned int acdb_id);

/* returns the latency for a usecase in Us */
int64_t platform_render_latency(audio_usecase_t usecase);

int platform_set_incall_recording_session_id(platform_data *platform,
                                             uint32_t session_id, int rec_mode);
int platform_stop_incall_recording_usecase(platform_data *platform);
int platform_start_incall_music_usecase(platform_data *platform);
int platform_stop_incall_music_usecase(platform_data *platform);

int platform_set_snd_device_backend(snd_device_t snd_device, const char * backend,
                                    const char * hw_interface);

/* From platform_info.c */
int platform_info_init(platform_data *);

int platform_get_usecase_index(const char * usecase);
int platform_set_usecase_pcm_id(audio_usecase_t usecase, int32_t type, int32_t pcm_id);
void platform_set_echo_reference(struct audio_device *adev, bool enable, audio_devices_t out_device);
int platform_swap_lr_channels(struct audio_device *adev, bool swap_channels);

bool platform_can_split_snd_device(snd_device_t in_snd_device,
                                   int *num_devices,
                                   snd_device_t *out_snd_devices);

bool platform_check_backends_match(snd_device_t snd_device1, snd_device_t snd_device2);

int platform_set_parameters(platform_data *platform, struct str_parms *parms);



#endif
