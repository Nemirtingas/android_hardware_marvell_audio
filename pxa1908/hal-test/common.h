#ifndef __INCLUDED_COMMON_H__
#define __INCLUDED_COMMON_H__

#ifndef LOG_TAG
#define LOG_TAG "audio_hw_primary"
#endif

#include <map>
#include <list>
#include <utility>
#include <algorithm>
#include <string>

#include <hardware/audio.h>
#include <hardware/audio_alsaops.h>
#include <cstdint>
#include <pthread.h>
#include <tinyalsa/asoundlib.h>
#include <audio_route/audio_route.h>
#include <utils/Mutex.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <cutils/log.h>
#include <cutils/str_parms.h>
#include <cutils/properties.h>

#define ARRAY_SIZE(arr) (sizeof((arr))/sizeof((arr)[0]))

/* Flags used to initialize acdb_settings variable that goes to ACDB library */
#define DMIC_FLAG       0x00000002
#define TTY_MODE_OFF    0x00000010
#define TTY_MODE_FULL   0x00000020
#define TTY_MODE_VCO    0x00000040
#define TTY_MODE_HCO    0x00000080
#define TTY_MODE_CLEAR  0xFFFFFF0F

#define MAX_SUPPORTED_CHANNEL_MASKS 2
#define DEFAULT_HDMI_OUT_CHANNELS   2

#define AFE_PROXY_CHANNEL_COUNT 2
#define AFE_PROXY_SAMPLING_RATE 48000
#define AFE_PROXY_PLAYBACK_PERIOD_SIZE  768
#define AFE_PROXY_PLAYBACK_PERIOD_COUNT 4
#define AFE_PROXY_RECORD_PERIOD_SIZE  768
#define AFE_PROXY_RECORD_PERIOD_COUNT 4

#define PROXY_OPEN_RETRY_COUNT           100
#define PROXY_OPEN_WAIT_TIME             20

#define MIN_CHANNEL_COUNT                1
#define DEFAULT_CHANNEL_COUNT            2
#define DEFAULT_OUTPUT_SAMPLING_RATE 48000

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

#define DEEP_BUFFER_PLATFORM_DELAY (29*1000LL)
#define LOW_LATENCY_PLATFORM_DELAY (13*1000LL)

enum{
    USECASE_INVALID = -1,
    /* Playback usecases */
    USECASE_AUDIO_PLAYBACK_DEEP_BUFFER = 0,
    USECASE_AUDIO_PLAYBACK_LOW_LATENCY,
    USECASE_AUDIO_PLAYBACK_MULTI_CH,
    USECASE_AUDIO_PLAYBACK_OFFLOAD,
    USECASE_AUDIO_PLAYBACK_TTS,
    USECASE_AUDIO_PLAYBACK_ULL,

    /* HFP Use case*/
    USECASE_AUDIO_HFP_SCO,
    USECASE_AUDIO_HFP_SCO_WB,

    /* Capture usecases */
    USECASE_AUDIO_RECORD,
    USECASE_AUDIO_RECORD_LOW_LATENCY,

    USECASE_VOICE_CALL,

    /* Voice extension usecases */
    USECASE_VOICE2_CALL,
    USECASE_VOLTE_CALL,
    USECASE_QCHAT_CALL,
    USECASE_VOWLAN_CALL,
    USECASE_INCALL_REC_UPLINK,
    USECASE_INCALL_REC_DOWNLINK,
    USECASE_INCALL_REC_UPLINK_AND_DOWNLINK,

    USECASE_AUDIO_SPKR_CALIB_RX,
    USECASE_AUDIO_SPKR_CALIB_TX,

    USECASE_AUDIO_PLAYBACK_AFE_PROXY,
    USECASE_AUDIO_RECORD_AFE_PROXY,
    USECASE_AUDIO_DSM_FEEDBACK,

    AUDIO_USECASE_MAX
};

typedef int audio_usecase_t;
typedef int snd_device_t;

typedef enum {
    PCM_PLAYBACK,
    PCM_CAPTURE,
    VOICE_CALL,
    PCM_HFP_CALL
} usecase_type_t;

class audio_device;
class stream_input;
class stream_output;

union stream_ptr {
    stream_input *in;
    stream_output *out;
};

struct audio_usecase
{
    //struct listnode list;
    audio_usecase_t id;
    usecase_type_t  type;
    audio_devices_t devices;
    snd_device_t out_snd_device;
    snd_device_t in_snd_device;
    union stream_ptr stream;
};

extern std::map<int, std::string> use_case_table;
extern pcm_config pcm_config_afe_proxy_playback;
extern pcm_config pcm_config_afe_proxy_record;
extern pcm_config pcm_config_deep_buffer;
extern pcm_config pcm_config_low_latency;

#define STRING_TO_ENUM(string) { #string, string }

struct string_to_enum {
    const char *name;
    uint32_t value;
};

extern const struct string_to_enum out_channels_name_to_enum_table[3];


#endif
