#include "common.h"
#include "audio_device.h"
#include "platform.h"


std::map<int, std::string> use_case_table =
{
    {USECASE_AUDIO_PLAYBACK_DEEP_BUFFER, "deep-buffer-playback"},
    {USECASE_AUDIO_PLAYBACK_LOW_LATENCY, "low-latency-playback"},
    {USECASE_AUDIO_PLAYBACK_MULTI_CH, "multi-channel-playback"},
    {USECASE_AUDIO_PLAYBACK_OFFLOAD, "compress-offload-playback"},
    {USECASE_AUDIO_PLAYBACK_TTS, "audio-tts-playback"},
    {USECASE_AUDIO_PLAYBACK_ULL, "audio-ull-playback"},

    {USECASE_AUDIO_RECORD, "audio-record"},
    {USECASE_AUDIO_RECORD_LOW_LATENCY, "low-latency-record"},

    {USECASE_AUDIO_HFP_SCO, "hfp-sco"},
    {USECASE_AUDIO_HFP_SCO_WB, "hfp-sco-wb"},

    {USECASE_VOICE_CALL, "voice-call"},
    {USECASE_VOICE2_CALL, "voice2-call"},
    {USECASE_VOLTE_CALL, "volte-call"},
    {USECASE_QCHAT_CALL, "qchat-call"},
    {USECASE_VOWLAN_CALL, "vowlan-call"},

    {USECASE_AUDIO_SPKR_CALIB_RX, "spkr-rx-calib"},
    {USECASE_AUDIO_SPKR_CALIB_TX, "spkr-vi-record"},

    {USECASE_AUDIO_PLAYBACK_AFE_PROXY, "afe-proxy-playback"},
    {USECASE_AUDIO_RECORD_AFE_PROXY, "afe-proxy-record"},
};

pcm_config pcm_config_afe_proxy_playback = {
    AFE_PROXY_CHANNEL_COUNT,
    AFE_PROXY_SAMPLING_RATE,
    AFE_PROXY_PLAYBACK_PERIOD_SIZE,
    AFE_PROXY_PLAYBACK_PERIOD_COUNT,
    PCM_FORMAT_S16_LE,
    AFE_PROXY_PLAYBACK_PERIOD_SIZE,
    INT_MAX,
    0,
#ifndef IGNORE_SILENCE_SIZE
    0,                             // silence_size
#endif
    AFE_PROXY_PLAYBACK_PERIOD_SIZE,
};

pcm_config pcm_config_afe_proxy_record = {
    AFE_PROXY_CHANNEL_COUNT,       // channels
    AFE_PROXY_SAMPLING_RATE,       // rate
    AFE_PROXY_RECORD_PERIOD_SIZE,  // period_size
    AFE_PROXY_RECORD_PERIOD_COUNT, // period_count
    PCM_FORMAT_S16_LE,             // format
    AFE_PROXY_RECORD_PERIOD_SIZE,  // start_threshold
    INT_MAX,                       // stop_threshold
    0,                             // silence_threshold
#ifndef IGNORE_SILENCE_SIZE
    0,                             // silence_size
#endif
    AFE_PROXY_RECORD_PERIOD_SIZE,  // avail_min
};

pcm_config pcm_config_deep_buffer = {
    DEFAULT_CHANNEL_COUNT,
    DEFAULT_OUTPUT_SAMPLING_RATE,
    DEEP_BUFFER_OUTPUT_PERIOD_SIZE,
    DEEP_BUFFER_OUTPUT_PERIOD_COUNT,
    PCM_FORMAT_S16_LE,
    DEEP_BUFFER_OUTPUT_PERIOD_SIZE / 4,
    INT_MAX,
    0,
#ifndef IGNORE_SILENCE_SIZE
    0,
#endif
    DEEP_BUFFER_OUTPUT_PERIOD_SIZE / 4,
};

pcm_config pcm_config_low_latency = {
    DEFAULT_CHANNEL_COUNT,
    DEFAULT_OUTPUT_SAMPLING_RATE,
    LOW_LATENCY_OUTPUT_PERIOD_SIZE,
    LOW_LATENCY_OUTPUT_PERIOD_COUNT,
    PCM_FORMAT_S16_LE,
    240, //LOW_LATENCY_OUTPUT_PERIOD_SIZE / 4,
    240*2,//INT_MAX,
    0,
#ifndef IGNORE_SILENCE_SIZE
    0,
#endif
    0,
    //LOW_LATENCY_OUTPUT_PERIOD_SIZE / 4,
};

const struct string_to_enum out_channels_name_to_enum_table[] = {
    STRING_TO_ENUM(AUDIO_CHANNEL_OUT_STEREO),
    STRING_TO_ENUM(AUDIO_CHANNEL_OUT_5POINT1),
    STRING_TO_ENUM(AUDIO_CHANNEL_OUT_7POINT1),
};

int64_t platform_render_latency(audio_usecase_t usecase)
{
    switch (usecase)
    {
        case USECASE_AUDIO_PLAYBACK_DEEP_BUFFER:
            return DEEP_BUFFER_PLATFORM_DELAY;
        case USECASE_AUDIO_PLAYBACK_LOW_LATENCY:
            return LOW_LATENCY_PLATFORM_DELAY;
        default:
            return 0;
    }
}
