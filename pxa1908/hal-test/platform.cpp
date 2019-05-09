#include "platform.h"

#define LOG_TAG "pxa1908_platform"

#include "common.h"
#include "audio_device.h"

#define MIXER_XML_PATH "/system/etc/mixer_paths.xml"
#define AUDIO_DATA_BLOCK_MIXER_CTL "HDMI EDID"
#define CVD_VERSION_MIXER_CTL "CVD Version"


/*
 * This file will have a maximum of 38 bytes:
 *
 * 4 bytes: number of audio blocks
 * 4 bytes: total length of Short Audio Descriptor (SAD) blocks
 * Maximum 10 * 3 bytes: SAD blocks
 */
#define MAX_SAD_BLOCKS      10
#define SAD_BLOCK_SIZE      3

#define MAX_CVD_VERSION_STRING_SIZE    100

/* EDID format ID for LPCM audio */
#define EDID_FORMAT_LPCM    1

/* Retry for delay in FW loading*/
#define RETRY_NUMBER 10
#define RETRY_US 500000
#define MAX_SND_CARD 8

#define MAX_SND_CARD_NAME_LEN 31

#define DEFAULT_APP_TYPE_RX_PATH  0x11130

struct platform_data
{
    struct audio_device *adev;
    bool fluence_in_spkr_mode;
    bool fluence_in_voice_call;
    bool fluence_in_voice_comm;
    bool fluence_in_voice_rec;
    /* 0 = no fluence, 1 = fluence, 2 = fluence pro */
    int  fluence_type;
    int  source_mic_type;
    bool speaker_lr_swap;

    char ec_ref_mixer_path[64];

    char *snd_card_name;
    int max_vol_index;
    int max_mic_count;
};

// usecase -> {input device, output device}
static std::map<audio_usecase_t,std::pair<int,int>> pcm_device_table =
{
    {USECASE_AUDIO_PLAYBACK_DEEP_BUFFER, {DEEP_BUFFER_PCM_DEVICE, DEEP_BUFFER_PCM_DEVICE}},
    {USECASE_AUDIO_PLAYBACK_LOW_LATENCY, {LOWLATENCY_PCM_DEVICE, LOWLATENCY_PCM_DEVICE}},
    {USECASE_AUDIO_PLAYBACK_MULTI_CH, {MULTIMEDIA2_PCM_DEVICE, MULTIMEDIA2_PCM_DEVICE}},
    {USECASE_AUDIO_PLAYBACK_OFFLOAD, {PLAYBACK_OFFLOAD_DEVICE, PLAYBACK_OFFLOAD_DEVICE}},
    {USECASE_AUDIO_PLAYBACK_TTS, {MULTIMEDIA2_PCM_DEVICE, MULTIMEDIA2_PCM_DEVICE}},
    {USECASE_AUDIO_PLAYBACK_ULL, {MULTIMEDIA3_PCM_DEVICE, MULTIMEDIA3_PCM_DEVICE}},

    {USECASE_AUDIO_RECORD, {AUDIO_RECORD_PCM_DEVICE, AUDIO_RECORD_PCM_DEVICE}},
    {USECASE_AUDIO_RECORD_LOW_LATENCY, {LOWLATENCY_PCM_DEVICE, LOWLATENCY_PCM_DEVICE}},

    {USECASE_VOICE_CALL, {VOICE_CALL_PCM_DEVICE, VOICE_CALL_PCM_DEVICE}},
    {USECASE_VOICE2_CALL, {VOICE2_CALL_PCM_DEVICE, VOICE2_CALL_PCM_DEVICE}},
    {USECASE_VOLTE_CALL, {VOLTE_CALL_PCM_DEVICE, VOLTE_CALL_PCM_DEVICE}},
    {USECASE_QCHAT_CALL, {QCHAT_CALL_PCM_DEVICE, QCHAT_CALL_PCM_DEVICE}},
    {USECASE_VOWLAN_CALL, {VOWLAN_CALL_PCM_DEVICE, VOWLAN_CALL_PCM_DEVICE}},
    {USECASE_INCALL_REC_UPLINK, {AUDIO_RECORD_PCM_DEVICE, AUDIO_RECORD_PCM_DEVICE}},
    {USECASE_INCALL_REC_DOWNLINK, {AUDIO_RECORD_PCM_DEVICE, AUDIO_RECORD_PCM_DEVICE}},
    {USECASE_INCALL_REC_UPLINK_AND_DOWNLINK, {AUDIO_RECORD_PCM_DEVICE, AUDIO_RECORD_PCM_DEVICE}},
    {USECASE_AUDIO_HFP_SCO, {HFP_PCM_RX, HFP_SCO_RX}},

    {USECASE_AUDIO_SPKR_CALIB_RX, {SPKR_PROT_CALIB_RX_PCM_DEVICE, -1}},
    {USECASE_AUDIO_SPKR_CALIB_TX, {-1, SPKR_PROT_CALIB_TX_PCM_DEVICE}},

    {USECASE_AUDIO_PLAYBACK_AFE_PROXY, {AFE_PROXY_PLAYBACK_PCM_DEVICE, AFE_PROXY_RECORD_PCM_DEVICE}},
    {USECASE_AUDIO_RECORD_AFE_PROXY, {AFE_PROXY_PLAYBACK_PCM_DEVICE, AFE_PROXY_RECORD_PCM_DEVICE}},
    {USECASE_AUDIO_DSM_FEEDBACK, {QUAT_MI2S_PCM_DEVICE, QUAT_MI2S_PCM_DEVICE}},

};

static std::map<int, std::string> device_table =
{
    {SND_DEVICE_NONE, "none"},
    /* Playback sound devices */
    {SND_DEVICE_OUT_HANDSET, "handset"},
    {SND_DEVICE_OUT_SPEAKER, "speaker"},
    {SND_DEVICE_OUT_SPEAKER_REVERSE, "speaker-reverse"},
    {SND_DEVICE_OUT_SPEAKER_SAFE, "speaker-safe"},
    {SND_DEVICE_OUT_HEADPHONES, "headphones"},
    {SND_DEVICE_OUT_LINE, "line"},
    {SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES, "speaker-and-headphones"},
    {SND_DEVICE_OUT_SPEAKER_SAFE_AND_HEADPHONES, "speaker-safe-and-headphones"},
    {SND_DEVICE_OUT_SPEAKER_AND_LINE, "speaker-and-line"},
    {SND_DEVICE_OUT_SPEAKER_SAFE_AND_LINE, "speaker-safe-and-line"},
    {SND_DEVICE_OUT_VOICE_HANDSET, "voice-handset"},
    {SND_DEVICE_OUT_VOICE_HAC_HANDSET, "voice-hac-handset"},
    {SND_DEVICE_OUT_VOICE_SPEAKER, "voice-speaker"},
    {SND_DEVICE_OUT_VOICE_HEADPHONES, "voice-headphones"},
    {SND_DEVICE_OUT_VOICE_LINE, "voice-line"},
    {SND_DEVICE_OUT_HDMI, "hdmi"},
    {SND_DEVICE_OUT_SPEAKER_AND_HDMI, "speaker-and-hdmi"},
    {SND_DEVICE_OUT_BT_SCO, "bt-sco-headset"},
    {SND_DEVICE_OUT_BT_SCO_WB, "bt-sco-headset-wb"},
    {SND_DEVICE_OUT_VOICE_HANDSET_TMUS, "voice-handset-tmus"},
    {SND_DEVICE_OUT_VOICE_TTY_FULL_HEADPHONES, "voice-tty-full-headphones"},
    {SND_DEVICE_OUT_VOICE_TTY_VCO_HEADPHONES, "voice-tty-vco-headphones"},
    {SND_DEVICE_OUT_VOICE_TTY_HCO_HANDSET, "voice-tty-hco-handset"},
    {SND_DEVICE_OUT_VOICE_TX, "voice-tx"},
    {SND_DEVICE_OUT_SPEAKER_PROTECTED, "speaker-protected"},
    {SND_DEVICE_OUT_VOICE_SPEAKER_PROTECTED, "voice-speaker-protected"},

    /* Capture sound devices */
    {SND_DEVICE_IN_HANDSET_MIC, "handset-mic"},
    {SND_DEVICE_IN_HANDSET_MIC_AEC, "handset-mic"},
    {SND_DEVICE_IN_HANDSET_MIC_NS, "handset-mic"},
    {SND_DEVICE_IN_HANDSET_MIC_AEC_NS, "handset-mic"},
    {SND_DEVICE_IN_HANDSET_DMIC, "dmic-endfire"},
    {SND_DEVICE_IN_HANDSET_DMIC_AEC, "dmic-endfire"},
    {SND_DEVICE_IN_HANDSET_DMIC_NS, "dmic-endfire"},
    {SND_DEVICE_IN_HANDSET_DMIC_AEC_NS, "dmic-endfire"},
    {SND_DEVICE_IN_HANDSET_DMIC_STEREO, "dmic-endfire"},

    {SND_DEVICE_IN_SPEAKER_MIC, "speaker-mic"},
    {SND_DEVICE_IN_SPEAKER_MIC_AEC, "speaker-mic"},
    {SND_DEVICE_IN_SPEAKER_MIC_NS, "speaker-mic"},
    {SND_DEVICE_IN_SPEAKER_MIC_AEC_NS, "speaker-mic"},
    {SND_DEVICE_IN_SPEAKER_DMIC, "speaker-dmic-endfire"},
    {SND_DEVICE_IN_SPEAKER_DMIC_AEC, "speaker-dmic-endfire"},
    {SND_DEVICE_IN_SPEAKER_DMIC_NS, "speaker-dmic-endfire"},
    {SND_DEVICE_IN_SPEAKER_DMIC_AEC_NS, "speaker-dmic-endfire"},
    {SND_DEVICE_IN_SPEAKER_DMIC_STEREO, "speaker-dmic-endfire"},

    {SND_DEVICE_IN_HEADSET_MIC, "headset-mic"},
    {SND_DEVICE_IN_HEADSET_MIC_AEC, "headset-mic"},

    {SND_DEVICE_IN_HDMI_MIC, "hdmi-mic"},
    {SND_DEVICE_IN_BT_SCO_MIC, "bt-sco-mic"},
    {SND_DEVICE_IN_BT_SCO_MIC_NREC, "bt-sco-mic"},
    {SND_DEVICE_IN_BT_SCO_MIC_WB, "bt-sco-mic-wb"},
    {SND_DEVICE_IN_BT_SCO_MIC_WB_NREC, "bt-sco-mic-wb"},
    {SND_DEVICE_IN_CAMCORDER_MIC, "camcorder-mic"},

    {SND_DEVICE_IN_VOICE_DMIC, "voice-dmic-ef"},
    {SND_DEVICE_IN_VOICE_DMIC_TMUS, "voice-dmic-ef-tmus"},
    {SND_DEVICE_IN_VOICE_SPEAKER_MIC, "voice-speaker-mic"},
    {SND_DEVICE_IN_VOICE_SPEAKER_DMIC, "voice-speaker-dmic-ef"},
    {SND_DEVICE_IN_VOICE_HEADSET_MIC, "voice-headset-mic"},
    {SND_DEVICE_IN_VOICE_TTY_FULL_HEADSET_MIC, "voice-tty-full-headset-mic"},
    {SND_DEVICE_IN_VOICE_TTY_VCO_HANDSET_MIC, "voice-tty-vco-handset-mic"},
    {SND_DEVICE_IN_VOICE_TTY_HCO_HEADSET_MIC, "voice-tty-hco-headset-mic"},

    {SND_DEVICE_IN_VOICE_REC_MIC, "voice-rec-mic"},
    {SND_DEVICE_IN_VOICE_REC_MIC_NS, "voice-rec-mic"},
    {SND_DEVICE_IN_VOICE_REC_DMIC_STEREO, "voice-rec-dmic-ef"},
    {SND_DEVICE_IN_VOICE_REC_DMIC_FLUENCE, "voice-rec-dmic-ef-fluence"},
    {SND_DEVICE_IN_VOICE_REC_HEADSET_MIC, "headset-mic"},

    {SND_DEVICE_IN_VOICE_RX, "voice-rx"},

    {SND_DEVICE_IN_THREE_MIC, "three-mic"},
    {SND_DEVICE_IN_QUAD_MIC, "quad-mic"},
    {SND_DEVICE_IN_CAPTURE_VI_FEEDBACK, "vi-feedback"},
    {SND_DEVICE_IN_HANDSET_TMIC, "three-mic"},
    {SND_DEVICE_IN_HANDSET_QMIC, "quad-mic"},
};

#define TO_NAME_INDEX(x) #x, x

/* Used to get index from parsed string */
static std::map<std::string, int> snd_device_name_index =
{
    /* out */
    {TO_NAME_INDEX(SND_DEVICE_OUT_HANDSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_REVERSE)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_SAFE)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_HEADPHONES)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_LINE)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_SAFE_AND_HEADPHONES)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_AND_LINE)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_SAFE_AND_LINE)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_HANDSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_SPEAKER)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_HEADPHONES)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_LINE)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_HDMI)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_AND_HDMI)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_BT_SCO)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_BT_SCO_WB)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_HANDSET_TMUS)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_HAC_HANDSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_TTY_FULL_HEADPHONES)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_TTY_VCO_HEADPHONES)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_TTY_HCO_HANDSET)},

    /* in */
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_PROTECTED)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_SPEAKER_PROTECTED)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_MIC_AEC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_MIC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_MIC_AEC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_DMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_DMIC_AEC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_DMIC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_DMIC_AEC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_DMIC_STEREO)},

    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_MIC_AEC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_MIC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_MIC_AEC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC_AEC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC_AEC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC_STEREO)},

    {TO_NAME_INDEX(SND_DEVICE_IN_HEADSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HEADSET_MIC_AEC)},

    {TO_NAME_INDEX(SND_DEVICE_IN_HDMI_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_BT_SCO_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_BT_SCO_MIC_NREC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_BT_SCO_MIC_WB)},
    {TO_NAME_INDEX(SND_DEVICE_IN_BT_SCO_MIC_WB_NREC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_CAMCORDER_MIC)},

    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_DMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_DMIC_TMUS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_SPEAKER_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_SPEAKER_DMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_HEADSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_TTY_FULL_HEADSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_TTY_VCO_HANDSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_TTY_HCO_HEADSET_MIC)},

    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_REC_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_REC_MIC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_REC_DMIC_STEREO)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_REC_DMIC_FLUENCE)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_REC_HEADSET_MIC)},

    {TO_NAME_INDEX(SND_DEVICE_IN_THREE_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_QUAD_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_CAPTURE_VI_FEEDBACK)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_TMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_QMIC)},
};

static std::map<std::string, int> usecase_name_index =
{
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_DEEP_BUFFER)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_LOW_LATENCY)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_MULTI_CH)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_OFFLOAD)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_TTS)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_ULL)},
    {TO_NAME_INDEX(USECASE_AUDIO_RECORD)},
    {TO_NAME_INDEX(USECASE_AUDIO_RECORD_LOW_LATENCY)},
    {TO_NAME_INDEX(USECASE_VOICE_CALL)},
    {TO_NAME_INDEX(USECASE_VOICE2_CALL)},
    {TO_NAME_INDEX(USECASE_VOLTE_CALL)},
    {TO_NAME_INDEX(USECASE_QCHAT_CALL)},
    {TO_NAME_INDEX(USECASE_VOWLAN_CALL)},
    {TO_NAME_INDEX(USECASE_INCALL_REC_UPLINK)},
    {TO_NAME_INDEX(USECASE_INCALL_REC_DOWNLINK)},
    {TO_NAME_INDEX(USECASE_INCALL_REC_UPLINK_AND_DOWNLINK)},
    {TO_NAME_INDEX(USECASE_AUDIO_HFP_SCO)},
};

static char * backend_tag_table[SND_DEVICE_MAX] = {0};
static char * hw_interface_table[SND_DEVICE_MAX] = {0};

//static struct listnode operator_info_list;
//static struct listnode *operator_specific_device_table[SND_DEVICE_MAX];

static pthread_once_t check_op_once_ctl = PTHREAD_ONCE_INIT;
static bool is_tmus = false;

static void check_operator()
{
    char value[PROPERTY_VALUE_MAX];
    int mccmnc;
    property_get("gsm.sim.operator.numeric",value,"0");
    mccmnc = atoi(value);
    ALOGD("%s: tmus mccmnc %d", __func__, mccmnc);
    switch(mccmnc) {
    /* TMUS MCC(310), MNC(490, 260, 026) */
    case 310490:
    case 310260:
    case 310026:
    /* Add new TMUS MNC(800, 660, 580, 310, 270, 250, 240, 230, 220, 210, 200, 160) */
    case 310800:
    case 310660:
    case 310580:
    case 310310:
    case 310270:
    case 310250:
    case 310240:
    case 310230:
    case 310220:
    case 310210:
    case 310200:
    case 310160:
        is_tmus = true;
        break;
    }
}

bool is_operator_tmus()
{
    pthread_once(&check_op_once_ctl, check_operator);
    return is_tmus;
}

const char *platform_get_snd_device_name(snd_device_t snd_device)
{
    if (snd_device >= SND_DEVICE_MIN && snd_device < SND_DEVICE_MAX)
    {
        /*
        if( operator_specific_device_table[snd_device] != NULL )
        {
            return get_operator_specific_device_mixer_path(snd_device);
        }
        */
        return device_table[snd_device].c_str();
    }
    else
        return "none";
}

int platform_get_pcm_device_id(audio_usecase_t usecase, int device_type)
{
    return (device_type == PCM_PLAYBACK ?
                pcm_device_table[usecase].first :
                pcm_device_table[usecase].second);
}

int platform_get_snd_device_index(char *device_name)
{
    std::map<std::string, int>::const_iterator it = snd_device_name_index.find(device_name);
    return (it == snd_device_name_index.end() ? -ENODEV : it->second);
}

int platform_get_usecase_index(const char * usecase)
{
    std::map<std::string, int>::const_iterator it = usecase_name_index.find(usecase);
    return (it == usecase_name_index.end() ? -ENODEV : it->second);
}

snd_device_t platform_get_output_snd_device(platform_data *my_data, audio_devices_t devices)
{
    audio_device *adev = my_data->adev;
    audio_mode_t mode = adev->_mode;
    snd_device_t snd_device = SND_DEVICE_NONE;

    ALOGV("%s: enter: output devices(%#x)", __func__, devices);
    if (devices == AUDIO_DEVICE_NONE ||
        devices & AUDIO_DEVICE_BIT_IN)
    {
        ALOGV("%s: Invalid output devices (%#x)", __func__, devices);
        goto exit;
    }
    if (popcount(devices) == 2)
    {
        if( devices == (AUDIO_DEVICE_OUT_WIRED_HEADPHONE | AUDIO_DEVICE_OUT_SPEAKER) ||
            devices == (AUDIO_DEVICE_OUT_WIRED_HEADSET   | AUDIO_DEVICE_OUT_SPEAKER)
          )
        {
            snd_device = SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES;
        }
        else if( devices == (AUDIO_DEVICE_OUT_LINE | AUDIO_DEVICE_OUT_SPEAKER) )
        {
            snd_device = SND_DEVICE_OUT_SPEAKER_AND_LINE;
        }
        else if( devices == (AUDIO_DEVICE_OUT_WIRED_HEADPHONE | AUDIO_DEVICE_OUT_SPEAKER_SAFE) ||
                 devices == (AUDIO_DEVICE_OUT_WIRED_HEADSET   | AUDIO_DEVICE_OUT_SPEAKER_SAFE))
        {
            snd_device = SND_DEVICE_OUT_SPEAKER_SAFE_AND_HEADPHONES;
        }
        else if( devices == (AUDIO_DEVICE_OUT_LINE | AUDIO_DEVICE_OUT_SPEAKER_SAFE))
        {
            snd_device = SND_DEVICE_OUT_SPEAKER_SAFE_AND_LINE;
        }
        else if( devices == (AUDIO_DEVICE_OUT_AUX_DIGITAL | AUDIO_DEVICE_OUT_SPEAKER))
        {
            snd_device = SND_DEVICE_OUT_SPEAKER_AND_HDMI;
        }
        else
        {
            ALOGE("%s: Invalid combo device(%#x)", __func__, devices);
            goto exit;
        }
        if (snd_device != SND_DEVICE_NONE)
        {
            goto exit;
        }
    }
    if (popcount(devices) != 1)
    {
        ALOGE("%s: Invalid output devices(%#x)", __func__, devices);
        goto exit;
    }
    if( voice_is_in_call(adev) || adev->_enable_voicerx )
    {
        if( devices & AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
            devices & AUDIO_DEVICE_OUT_WIRED_HEADSET ||
            devices & AUDIO_DEVICE_OUT_LINE)
        {
            if (voice_is_in_call(adev) && (adev->_voice.tty_mode == TTY_MODE_FULL))
                snd_device = SND_DEVICE_OUT_VOICE_TTY_FULL_HEADPHONES;
            else if (voice_is_in_call(adev) && (adev->_voice.tty_mode == TTY_MODE_VCO))
                snd_device = SND_DEVICE_OUT_VOICE_TTY_VCO_HEADPHONES;
            else if (voice_is_in_call(adev) && (adev->_voice.tty_mode == TTY_MODE_HCO))
                snd_device = SND_DEVICE_OUT_VOICE_TTY_HCO_HANDSET;
            else
            {
                if( devices & AUDIO_DEVICE_OUT_LINE)
                    snd_device = SND_DEVICE_OUT_VOICE_LINE;
                else
                    snd_device = SND_DEVICE_OUT_VOICE_HEADPHONES;
            }
        }
        else if( devices & AUDIO_DEVICE_OUT_ALL_SCO )
        {
            if( adev->_bt_wb_speech_enabled )
            {
                snd_device = SND_DEVICE_OUT_BT_SCO_WB;
            }
            else
            {
                snd_device = SND_DEVICE_OUT_BT_SCO;
            }
        }
        else if( devices & (AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_SPEAKER_SAFE) )
        {
            snd_device = SND_DEVICE_OUT_VOICE_SPEAKER;
        }
        else if( devices & AUDIO_DEVICE_OUT_EARPIECE )
        {
            if(adev->_voice.hac)
                snd_device = SND_DEVICE_OUT_VOICE_HAC_HANDSET;
            else if (is_operator_tmus())
                snd_device = SND_DEVICE_OUT_VOICE_HANDSET_TMUS;
            else
                snd_device = SND_DEVICE_OUT_VOICE_HANDSET;
        }
        else if( devices & AUDIO_DEVICE_OUT_TELEPHONY_TX )
            snd_device = SND_DEVICE_OUT_VOICE_TX;

        if( snd_device != SND_DEVICE_NONE )
        {
            goto exit;
        }
    }

    if( devices & AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
        devices & AUDIO_DEVICE_OUT_WIRED_HEADSET)
    {
        snd_device = SND_DEVICE_OUT_HEADPHONES;
    }
    else if( devices & AUDIO_DEVICE_OUT_LINE )
    {
        snd_device = SND_DEVICE_OUT_LINE;
    }
    else if( devices & AUDIO_DEVICE_OUT_SPEAKER_SAFE )
    {
        snd_device = SND_DEVICE_OUT_SPEAKER_SAFE;
    }
    else if (devices & AUDIO_DEVICE_OUT_SPEAKER)
    {
        if (my_data->speaker_lr_swap)
            snd_device = SND_DEVICE_OUT_SPEAKER_REVERSE;
        else
            snd_device = SND_DEVICE_OUT_SPEAKER;
    }
    else if( devices & AUDIO_DEVICE_OUT_ALL_SCO )
    {
        if( adev->_bt_wb_speech_enabled )
        {
            snd_device = SND_DEVICE_OUT_BT_SCO_WB;
        }
        else
        {
            snd_device = SND_DEVICE_OUT_BT_SCO;
        }
    }
    else if (devices & AUDIO_DEVICE_OUT_AUX_DIGITAL)
    {
        snd_device = SND_DEVICE_OUT_HDMI ;
    }
    else if (devices & AUDIO_DEVICE_OUT_EARPIECE)
    {
        /*HAC support for voice-ish audio (eg visual voicemail)*/
        if(adev->_voice.hac)
            snd_device = SND_DEVICE_OUT_VOICE_HAC_HANDSET;
        else
            snd_device = SND_DEVICE_OUT_HANDSET;
    }
    else
    {
        ALOGE("%s: Unknown device(s) %#x", __func__, devices);
    }
exit:
    ALOGV("%s: exit: snd_device(%s)", __func__, device_table[snd_device].c_str());
    return snd_device;
}

snd_device_t platform_get_input_snd_device(platform_data *my_data, audio_devices_t out_device)
{
    struct audio_device *adev = my_data->adev;
    audio_source_t  source = (adev->_active_input == NULL) ?
                                AUDIO_SOURCE_DEFAULT : (audio_source_t)adev->_active_input->_source;

    audio_mode_t    mode   = adev->_mode;
    audio_devices_t in_device = ((adev->_active_input == NULL) ?
                                    AUDIO_DEVICE_NONE : adev->_active_input->_device)
                                & ~AUDIO_DEVICE_BIT_IN;
    audio_channel_mask_t channel_mask = (adev->_active_input == NULL) ?
                                AUDIO_CHANNEL_IN_MONO : adev->_active_input->_channel_mask;
    snd_device_t snd_device = SND_DEVICE_NONE;
    int channel_count = popcount(channel_mask);

    ALOGV("%s: enter: out_device(%#x) in_device(%#x) channel_count (%d) channel_mask (0x%x)",
          __func__, out_device, in_device, channel_count, channel_mask);
    if( (out_device != AUDIO_DEVICE_NONE) && voice_is_in_call(adev) )
    {
        if( adev->_voice.tty_mode != TTY_MODE_OFF )
        {
            if( out_device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
                out_device & AUDIO_DEVICE_OUT_WIRED_HEADSET ||
                out_device & AUDIO_DEVICE_OUT_LINE )
            {
                switch (adev->_voice.tty_mode)
                {
                    case TTY_MODE_FULL:
                        snd_device = SND_DEVICE_IN_VOICE_TTY_FULL_HEADSET_MIC;
                        break;
                    case TTY_MODE_VCO:
                        snd_device = SND_DEVICE_IN_VOICE_TTY_VCO_HANDSET_MIC;
                        break;
                    case TTY_MODE_HCO:
                        snd_device = SND_DEVICE_IN_VOICE_TTY_HCO_HEADSET_MIC;
                        break;
                    default:
                        ALOGE("%s: Invalid TTY mode (%#x)", __func__, adev->_voice.tty_mode);
                }
                goto exit;
            }
        }
        if( out_device & AUDIO_DEVICE_OUT_EARPIECE)
        {
            if( my_data->fluence_in_voice_call == false)
            {
                snd_device = SND_DEVICE_IN_HANDSET_MIC;
            }
            else
            {
                if (is_operator_tmus())
                    snd_device = SND_DEVICE_IN_VOICE_DMIC_TMUS;
                else
                    snd_device = SND_DEVICE_IN_VOICE_DMIC;
            }
        }
        else if( out_device & AUDIO_DEVICE_OUT_WIRED_HEADSET )
        {
            snd_device = SND_DEVICE_IN_VOICE_HEADSET_MIC;
        }
        else if( out_device & AUDIO_DEVICE_OUT_ALL_SCO )
        {
            if( adev->_bt_wb_speech_enabled )
            {
                if( adev->_bluetooth_nrec )
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_WB_NREC;
                else
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_WB;
            }
            else
            {
                if (adev->_bluetooth_nrec)
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_NREC;
                else
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC;
            }
        }
        else if( out_device & AUDIO_DEVICE_OUT_SPEAKER ||
                 out_device & AUDIO_DEVICE_OUT_SPEAKER_SAFE ||
                 out_device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
                 out_device & AUDIO_DEVICE_OUT_LINE )
        {
            if( my_data->fluence_in_voice_call && my_data->fluence_in_spkr_mode )
            {
                if( my_data->source_mic_type & SOURCE_DUAL_MIC )
                {
                    snd_device = SND_DEVICE_IN_VOICE_SPEAKER_DMIC;
                }
                else
                {
                    snd_device = SND_DEVICE_IN_VOICE_SPEAKER_MIC;
                }
            }

            //select default
            if( snd_device == SND_DEVICE_NONE )
            {
                snd_device = SND_DEVICE_IN_VOICE_SPEAKER_MIC;
            }
        }
        else if( out_device & AUDIO_DEVICE_OUT_TELEPHONY_TX )
        {
            snd_device = SND_DEVICE_IN_VOICE_RX;
        }
    }
    else if( source == AUDIO_SOURCE_CAMCORDER )
    {
        if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC ||
            in_device & AUDIO_DEVICE_IN_BACK_MIC)
        {
            snd_device = SND_DEVICE_IN_CAMCORDER_MIC;
        }
    }
    else if( source == AUDIO_SOURCE_VOICE_RECOGNITION )
    {
        if( in_device & AUDIO_DEVICE_IN_BUILTIN_MIC )
        {
            if( my_data->fluence_in_voice_rec && channel_count == 1 )
            {
                if ((my_data->fluence_type == FLUENCE_PRO_ENABLE) &&
                    (my_data->source_mic_type & SOURCE_QUAD_MIC))
                {
                    snd_device = SND_DEVICE_IN_HANDSET_QMIC;
                }
                else if ((my_data->fluence_type == FLUENCE_PRO_ENABLE) &&
                    (my_data->source_mic_type & SOURCE_THREE_MIC))
                {
                    snd_device = SND_DEVICE_IN_HANDSET_TMIC;
                }
                else if (((my_data->fluence_type == FLUENCE_PRO_ENABLE) ||
                    (my_data->fluence_type == FLUENCE_ENABLE)) &&
                    (my_data->source_mic_type & SOURCE_DUAL_MIC))
                {
                    snd_device = SND_DEVICE_IN_VOICE_REC_DMIC_FLUENCE;
                }
                platform_set_echo_reference(adev, true, out_device);
            }
            else if ((channel_mask == AUDIO_CHANNEL_IN_FRONT_BACK) &&
                       (my_data->source_mic_type & SOURCE_DUAL_MIC))
            {
                snd_device = SND_DEVICE_IN_VOICE_REC_DMIC_STEREO;
            }
            else if (((int)channel_mask == AUDIO_CHANNEL_INDEX_MASK_3) &&
                       (my_data->source_mic_type & SOURCE_THREE_MIC))
            {
                snd_device = SND_DEVICE_IN_THREE_MIC;
            }
            else if (((int)channel_mask == AUDIO_CHANNEL_INDEX_MASK_4) &&
                       (my_data->source_mic_type & SOURCE_QUAD_MIC))
            {
                snd_device = SND_DEVICE_IN_QUAD_MIC;
            }
            if (snd_device == SND_DEVICE_NONE)
            {
                if (adev->_active_input->_enable_ns)
                    snd_device = SND_DEVICE_IN_VOICE_REC_MIC_NS;
                else
                    snd_device = SND_DEVICE_IN_VOICE_REC_MIC;
            }
        }
        else if (in_device & AUDIO_DEVICE_IN_WIRED_HEADSET)
        {
            snd_device = SND_DEVICE_IN_VOICE_REC_HEADSET_MIC;
        }
    }
    else if (source == AUDIO_SOURCE_VOICE_COMMUNICATION ||
            mode == AUDIO_MODE_IN_COMMUNICATION)
    {
        if (out_device & (AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_SPEAKER_SAFE))
            in_device = AUDIO_DEVICE_IN_BACK_MIC;
        if (adev->_active_input)
        {
            if (adev->_active_input->_enable_aec &&
                    adev->_active_input->_enable_ns)
            {
                if (in_device & AUDIO_DEVICE_IN_BACK_MIC)
                {
                    if (my_data->fluence_in_spkr_mode &&
                            my_data->fluence_in_voice_comm &&
                            (my_data->source_mic_type & SOURCE_DUAL_MIC))
                    {
                        snd_device = SND_DEVICE_IN_SPEAKER_DMIC_AEC_NS;
                    }
                    else
                    {
                        snd_device = SND_DEVICE_IN_SPEAKER_MIC_AEC_NS;
                    }
                }
                else if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC)
                {
                    if (my_data->fluence_in_voice_comm &&
                            (my_data->source_mic_type & SOURCE_DUAL_MIC))
                    {
                        snd_device = SND_DEVICE_IN_HANDSET_DMIC_AEC_NS;
                    }
                    else
                    {
                        snd_device = SND_DEVICE_IN_HANDSET_MIC_AEC_NS;
                    }
                }
                else if (in_device & AUDIO_DEVICE_IN_WIRED_HEADSET)
                {
                    snd_device = SND_DEVICE_IN_HEADSET_MIC_AEC;
                }
                platform_set_echo_reference(adev, true, out_device);
            }
            else if (adev->_active_input->_enable_aec)
            {
                if (in_device & AUDIO_DEVICE_IN_BACK_MIC)
                {
                    if (my_data->fluence_in_spkr_mode &&
                    my_data->fluence_in_voice_comm &&
                    (my_data->source_mic_type & SOURCE_DUAL_MIC))
                    {
                        snd_device = SND_DEVICE_IN_SPEAKER_DMIC_AEC;
                    }
                    else
                    {
                        snd_device = SND_DEVICE_IN_SPEAKER_MIC_AEC;
                    }
                }
                else if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC)
                {
                    if (my_data->fluence_in_voice_comm &&
                    (my_data->source_mic_type & SOURCE_DUAL_MIC))
                    {
                        snd_device = SND_DEVICE_IN_HANDSET_DMIC_AEC;
                    }
                    else
                    {
                        snd_device = SND_DEVICE_IN_HANDSET_MIC_AEC;
                    }
                }
                else if (in_device & AUDIO_DEVICE_IN_WIRED_HEADSET)
                {
                    snd_device = SND_DEVICE_IN_HEADSET_MIC_AEC;
                }
                platform_set_echo_reference(adev, true, out_device);
            }
            else if (adev->_active_input->_enable_ns)
            {
                if (in_device & AUDIO_DEVICE_IN_BACK_MIC)
                {
                    if (my_data->fluence_in_spkr_mode &&
                            my_data->fluence_in_voice_comm &&
                            (my_data->source_mic_type & SOURCE_DUAL_MIC))
                    {
                        snd_device = SND_DEVICE_IN_SPEAKER_DMIC_NS;
                    }
                    else
                    {
                        snd_device = SND_DEVICE_IN_SPEAKER_MIC_NS;
                    }
                }
                else if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC)
                {
                    if (my_data->fluence_in_voice_comm &&
                            (my_data->source_mic_type & SOURCE_DUAL_MIC))
                    {
                        snd_device = SND_DEVICE_IN_HANDSET_DMIC_NS;
                    }
                    else
                    {
                        snd_device = SND_DEVICE_IN_HANDSET_MIC_NS;
                    }
                }
            }
        }
    }
    else if (source == AUDIO_SOURCE_DEFAULT)
    {
        goto exit;
    }


    if (snd_device != SND_DEVICE_NONE)
    {
        goto exit;
    }

    if (in_device != AUDIO_DEVICE_NONE &&
            !(in_device & AUDIO_DEVICE_IN_VOICE_CALL) &&
            !(in_device & AUDIO_DEVICE_IN_COMMUNICATION))
    {
        if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC)
        {
            if ((my_data->source_mic_type & SOURCE_QUAD_MIC) &&
                (int)channel_mask == AUDIO_CHANNEL_INDEX_MASK_4)
            {
                snd_device = SND_DEVICE_IN_QUAD_MIC;
            }
            else if ((my_data->source_mic_type & SOURCE_THREE_MIC) &&
                       (int)channel_mask == AUDIO_CHANNEL_INDEX_MASK_3)
            {
                snd_device = SND_DEVICE_IN_THREE_MIC;
            }
            else if ((my_data->source_mic_type & SOURCE_DUAL_MIC) &&
                       channel_count == 2)
            {
                snd_device = SND_DEVICE_IN_HANDSET_DMIC_STEREO;
            }
            else if ((my_data->source_mic_type & SOURCE_MONO_MIC) &&
                       channel_count == 1)
            {
                snd_device = SND_DEVICE_IN_HANDSET_MIC;
            }
            else
            {
                ALOGE("%s: something wrong (1): source type (%d) channel_count (%d) .."
                      " channel mask (0x%x) no combination found .. setting to mono", __func__,
                       my_data->source_mic_type, channel_count, channel_mask);
                snd_device = SND_DEVICE_IN_HANDSET_MIC;
            }
        }
        else if (in_device & AUDIO_DEVICE_IN_BACK_MIC)
        {
            if ((my_data->source_mic_type & SOURCE_DUAL_MIC) &&
                    channel_count == 2)
            {
                snd_device = SND_DEVICE_IN_SPEAKER_DMIC_STEREO;
            }
            else if ((my_data->source_mic_type & SOURCE_MONO_MIC) &&
                    channel_count == 1)
            {
                snd_device = SND_DEVICE_IN_SPEAKER_MIC;
            }
            else
            {
                ALOGE("%s: something wrong (2): source type (%d) channel_count (%d) .."
                      " no combination found .. setting to mono", __func__,
                       my_data->source_mic_type, channel_count);
                snd_device = SND_DEVICE_IN_SPEAKER_MIC;
            }
        }
        else if (in_device & AUDIO_DEVICE_IN_WIRED_HEADSET)
        {
            snd_device = SND_DEVICE_IN_HEADSET_MIC;
        }
        else if (in_device & AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)
        {
            if (adev->_bt_wb_speech_enabled)
            {
                if (adev->_bluetooth_nrec)
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_WB_NREC;
                else
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_WB;
            } else {
                if (adev->_bluetooth_nrec)
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_NREC;
                else
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC;
            }
        }
        else if (in_device & AUDIO_DEVICE_IN_AUX_DIGITAL)
        {
            snd_device = SND_DEVICE_IN_HDMI_MIC;
        }
        else
        {
            ALOGE("%s: Unknown input device(s) %#x", __func__, in_device);
            ALOGW("%s: Using default handset-mic", __func__);
            snd_device = SND_DEVICE_IN_HANDSET_MIC;
        }
    }
    else
    {
        if (out_device & AUDIO_DEVICE_OUT_EARPIECE)
        {
            snd_device = SND_DEVICE_IN_HANDSET_MIC;
        }
        else if (out_device & AUDIO_DEVICE_OUT_WIRED_HEADSET)
        {
            snd_device = SND_DEVICE_IN_HEADSET_MIC;
        }
        else if (out_device & AUDIO_DEVICE_OUT_SPEAKER ||
                 out_device & AUDIO_DEVICE_OUT_SPEAKER_SAFE ||
                 out_device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
                 out_device & AUDIO_DEVICE_OUT_LINE)
        {
            if ((my_data->source_mic_type & SOURCE_DUAL_MIC) &&
                    channel_count == 2)
            {
                snd_device = SND_DEVICE_IN_SPEAKER_DMIC_STEREO;
            }
            else if ((my_data->source_mic_type & SOURCE_MONO_MIC) &&
                          channel_count == 1)
            {
                snd_device = SND_DEVICE_IN_SPEAKER_MIC;
            }
            else
            {
                ALOGE("%s: something wrong (3): source type (%d) channel_count (%d) .."
                      " no combination found .. setting to mono", __func__,
                       my_data->source_mic_type, channel_count);
                snd_device = SND_DEVICE_IN_SPEAKER_MIC;
            }
        }
        else if (out_device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET)
        {
            if (adev->_bt_wb_speech_enabled)
            {
                if (adev->_bluetooth_nrec)
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_WB_NREC;
                else
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_WB;
            }
            else
            {
                if (adev->_bluetooth_nrec)
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_NREC;
                else
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC;
            }
        }
        else if (out_device & AUDIO_DEVICE_OUT_AUX_DIGITAL)
        {
            snd_device = SND_DEVICE_IN_HDMI_MIC;
        }
        else
        {
            ALOGE("%s: Unknown output device(s) %#x", __func__, out_device);
            ALOGW("%s: Using default handset-mic", __func__);
            snd_device = SND_DEVICE_IN_HANDSET_MIC;
        }
    }
exit:
    ALOGV("%s: exit: in_snd_device(%s)", __func__, device_table[snd_device].c_str());
    return snd_device;
}

int platform_switch_voice_call_device_pre(platform_data *my_data)
{
    int ret = 0;

    /*
    if (my_data->csd != NULL &&
        voice_is_in_call(my_data->adev))
    {
        // This must be called before disabling mixer controls on APQ side
        ret = my_data->csd->disable_device();
        if (ret < 0)
        {
            ALOGE("%s: csd_client_disable_device, failed, error %d",
                  __func__, ret);
        }
    }
    */
    return ret;
}

int platform_switch_voice_call_enable_device_config(platform_data *my_data,
                                                    snd_device_t out_snd_device,
                                                    snd_device_t in_snd_device)
{
    int acdb_rx_id, acdb_tx_id;
    int ret = 0;
    /*
    if (my_data->csd == NULL)
        return ret;

    if (out_snd_device == SND_DEVICE_OUT_VOICE_SPEAKER &&
        audio_extn_spkr_prot_is_enabled())
        acdb_rx_id = platform_get_snd_device_acdb_id(SND_DEVICE_OUT_SPEAKER_PROTECTED);
    else
        acdb_rx_id = platform_get_snd_device_acdb_id(out_snd_device);

    acdb_tx_id = platform_get_snd_device_acdb_id(in_snd_device);

    if (acdb_rx_id > 0 && acdb_tx_id > 0) {
        ret = my_data->csd->enable_device_config(acdb_rx_id, acdb_tx_id);
        if (ret < 0) {
            ALOGE("%s: csd_enable_device_config, failed, error %d",
                  __func__, ret);
        }
    } else {
        ALOGE("%s: Incorrect ACDB IDs (rx: %d tx: %d)", __func__,
              acdb_rx_id, acdb_tx_id);
    }
    */
    return ret;
}

int platform_switch_voice_call_device_post(platform_data *my_data,
                                           snd_device_t out_snd_device,
                                           snd_device_t in_snd_device)
{
    /*
    int acdb_rx_id, acdb_tx_id;

    if (my_data->acdb_send_voice_cal == NULL) {
        ALOGE("%s: dlsym error for acdb_send_voice_call", __func__);
    } else {
        if (out_snd_device == SND_DEVICE_OUT_VOICE_SPEAKER &&
            audio_extn_spkr_prot_is_enabled())
            out_snd_device = SND_DEVICE_OUT_VOICE_SPEAKER_PROTECTED;

        acdb_rx_id = platform_get_snd_device_acdb_id(out_snd_device);
        acdb_tx_id = platform_get_snd_device_acdb_id(in_snd_device);

        if (acdb_rx_id > 0 && acdb_tx_id > 0)
            my_data->acdb_send_voice_cal(acdb_rx_id, acdb_tx_id);
        else
            ALOGE("%s: Incorrect ACDB IDs (rx: %d tx: %d)", __func__,
                  acdb_rx_id, acdb_tx_id);
    }
    */
    return 0;
}

int platform_switch_voice_call_usecase_route_post(platform_data *my_data,
                                                  snd_device_t out_snd_device,
                                                  snd_device_t in_snd_device)
{
    int acdb_rx_id, acdb_tx_id;
    int ret = 0;
    /*
    if (my_data->csd == NULL)
        return ret;

    if (out_snd_device == SND_DEVICE_OUT_VOICE_SPEAKER &&
        audio_extn_spkr_prot_is_enabled())
        acdb_rx_id = platform_get_snd_device_acdb_id(SND_DEVICE_OUT_VOICE_SPEAKER_PROTECTED);
    else
        acdb_rx_id = platform_get_snd_device_acdb_id(out_snd_device);

    acdb_tx_id = platform_get_snd_device_acdb_id(in_snd_device);

    if (acdb_rx_id > 0 && acdb_tx_id > 0) {
        ret = my_data->csd->enable_device(acdb_rx_id, acdb_tx_id,
                                          my_data->adev->acdb_settings);
        if (ret < 0) {
            ALOGE("%s: csd_enable_device, failed, error %d", __func__, ret);
        }
    } else {
        ALOGE("%s: Incorrect ACDB IDs (rx: %d tx: %d)", __func__,
              acdb_rx_id, acdb_tx_id);
    }
    */
    return ret;
}

void platform_set_echo_reference(struct audio_device *adev, bool enable, audio_devices_t out_device)
{
    struct platform_data *my_data = (struct platform_data *)adev->_platform;
    snd_device_t snd_device = SND_DEVICE_NONE;

    if (strcmp(my_data->ec_ref_mixer_path, ""))
    {
        ALOGV("%s: diabling %s", __func__, my_data->ec_ref_mixer_path);
        audio_route_reset_and_update_path(adev->_audio_route, my_data->ec_ref_mixer_path);
    }

    if (enable)
    {
        strcpy(my_data->ec_ref_mixer_path, "echo-reference");
        if (out_device != AUDIO_DEVICE_NONE)
        {
            snd_device = platform_get_output_snd_device(my_data, out_device);
            platform_add_backend_name(my_data, my_data->ec_ref_mixer_path, snd_device);
        }

        ALOGD("%s: enabling %s", __func__, my_data->ec_ref_mixer_path);
        audio_route_apply_and_update_path(adev->_audio_route, my_data->ec_ref_mixer_path);
    }
}

void platform_add_backend_name(platform_data *my_data, char *mixer_path,
                               snd_device_t snd_device)
{
    if ((snd_device < SND_DEVICE_MIN) || (snd_device >= SND_DEVICE_MAX))
    {
        ALOGE("%s: Invalid snd_device = %d", __func__, snd_device);
        return;
    }

    const char * suffix = backend_tag_table[snd_device];

    if (suffix != NULL)
    {
        strcat(mixer_path, " ");
        strcat(mixer_path, suffix);
    }
}

int platform_send_audio_calibration(platform_data *my_data, snd_device_t snd_device)
{
    int acdb_dev_id, acdb_dev_type;
    /*
    acdb_dev_id = acdb_device_table[audio_extn_get_spkr_prot_snd_device(snd_device)];
    if (acdb_dev_id < 0) {
        ALOGE("%s: Could not find acdb id for device(%d)",
              __func__, snd_device);
        return -EINVAL;
    }
    if (my_data->acdb_send_audio_cal) {
        ALOGD("%s: sending audio calibration for snd_device(%d) acdb_id(%d)",
              __func__, snd_device, acdb_dev_id);
        if (snd_device >= SND_DEVICE_OUT_BEGIN &&
                snd_device < SND_DEVICE_OUT_END)
            acdb_dev_type = ACDB_DEV_TYPE_OUT;
        else
            acdb_dev_type = ACDB_DEV_TYPE_IN;
        my_data->acdb_send_audio_cal(acdb_dev_id, acdb_dev_type);
    }
    */
    return 0;
}

void platform_set_speaker_gain_in_combo(audio_device *adev,
                                        snd_device_t snd_device,
                                        bool enable)
{
    const char* name;
    switch (snd_device)
    {
        case SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES:
            if (enable)
                name = "spkr-gain-in-headphone-combo";
            else
                name = "speaker-gain-default";
            break;
        case SND_DEVICE_OUT_SPEAKER_AND_LINE:
            if (enable)
                name = "spkr-gain-in-line-combo";
            else
                name = "speaker-gain-default";
            break;
        case SND_DEVICE_OUT_SPEAKER_SAFE_AND_HEADPHONES:
            if (enable)
                name = "spkr-safe-gain-in-headphone-combo";
            else
                name = "speaker-safe-gain-default";
            break;
        case SND_DEVICE_OUT_SPEAKER_SAFE_AND_LINE:
            if (enable)
                name = "spkr-safe-gain-in-line-combo";
            else
                name = "speaker-safe-gain-default";
            break;
        default:
            return;
    }

    audio_route_apply_and_update_path(adev->_audio_route, name);
}

bool platform_check_backends_match(snd_device_t snd_device1, snd_device_t snd_device2)
{
    bool result = true;

    ALOGV("%s: snd_device1 = %s, snd_device2 = %s", __func__,
                platform_get_snd_device_name(snd_device1),
                platform_get_snd_device_name(snd_device2));

    if ((snd_device1 < SND_DEVICE_MIN) || (snd_device1 >= SND_DEVICE_MAX))
    {
        ALOGE("%s: Invalid snd_device = %s", __func__,
                platform_get_snd_device_name(snd_device1));
        return false;
    }
    if ((snd_device2 < SND_DEVICE_MIN) || (snd_device2 >= SND_DEVICE_MAX))
    {
        ALOGE("%s: Invalid snd_device = %s", __func__,
                platform_get_snd_device_name(snd_device2));
        return false;
    }
    const char * be_itf1 = hw_interface_table[snd_device1];
    const char * be_itf2 = hw_interface_table[snd_device2];

    if (NULL != be_itf1 && NULL != be_itf2)
    {
        if ((NULL == strstr(be_itf2, be_itf1)) && (NULL == strstr(be_itf1, be_itf2)))
            result = false;
    }

    ALOGV("%s: be_itf1 = %s, be_itf2 = %s, match %d", __func__, be_itf1, be_itf2, result);
    return result;
}

bool platform_can_split_snd_device(snd_device_t snd_device,
                                   int *num_devices,
                                   snd_device_t *new_snd_devices)
{
    bool status = false;

    if (NULL == num_devices || NULL == new_snd_devices)
    {
        ALOGE("%s: NULL pointer ..", __func__);
        return false;
    }

    /*
     * If wired headset/headphones/line devices share the same backend
     * with speaker/earpiece this routine returns false.
     */
    if (snd_device == SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES &&
        !platform_check_backends_match(SND_DEVICE_OUT_SPEAKER, SND_DEVICE_OUT_HEADPHONES))
    {
        *num_devices = 2;
        new_snd_devices[0] = SND_DEVICE_OUT_SPEAKER;
        new_snd_devices[1] = SND_DEVICE_OUT_HEADPHONES;
        status = true;
    }
    else if (snd_device == SND_DEVICE_OUT_SPEAKER_AND_LINE &&
               !platform_check_backends_match(SND_DEVICE_OUT_SPEAKER, SND_DEVICE_OUT_LINE))
    {
        *num_devices = 2;
        new_snd_devices[0] = SND_DEVICE_OUT_SPEAKER;
        new_snd_devices[1] = SND_DEVICE_OUT_LINE;
        status = true;
    }
    else if (snd_device == SND_DEVICE_OUT_SPEAKER_SAFE_AND_HEADPHONES &&
               !platform_check_backends_match(SND_DEVICE_OUT_SPEAKER_SAFE, SND_DEVICE_OUT_HEADPHONES))
    {
        *num_devices = 2;
        new_snd_devices[0] = SND_DEVICE_OUT_SPEAKER_SAFE;
        new_snd_devices[1] = SND_DEVICE_OUT_HEADPHONES;
        status = true;
    }
    else if (snd_device == SND_DEVICE_OUT_SPEAKER_SAFE_AND_LINE &&
               !platform_check_backends_match(SND_DEVICE_OUT_SPEAKER_SAFE, SND_DEVICE_OUT_LINE))
    {
        *num_devices = 2;
        new_snd_devices[0] = SND_DEVICE_OUT_SPEAKER_SAFE;
        new_snd_devices[1] = SND_DEVICE_OUT_LINE;
        status = true;
    }
    return status;
}

platform_data* platform_init(audio_device *adev)
{
    platform_data *my_data = new platform_data;
    int snd_card = 0;
    int retry_num;
    mixer *mixer;
    std::string snd_card_name;

    memset(my_data, 0, sizeof(platform_data));
    my_data->adev = adev;
    platform_info_init(my_data);

    while( snd_card < MAX_SND_CARD )
    {
        retry_num = 0;

        mixer = mixer_open(snd_card);
        while( mixer == nullptr && retry_num < RETRY_NUMBER )
        {
            usleep(RETRY_US);
            mixer = mixer_open(snd_card);
            ++retry_num;
        }

        if( mixer == nullptr )
        {
            ++snd_card;
            continue;
        }

        snd_card_name = mixer_get_name(mixer);

        if( my_data->snd_card_name != nullptr &&
            snd_card_name != my_data->snd_card_name )
        {
            ++snd_card;
            continue;
        }

        ALOGD("%s: snd_card_name: ", __func__, snd_card_name.c_str());

        adev->_audio_route = audio_route_init(snd_card, MIXER_XML_PATH);
        if( adev->_audio_route == nullptr )
        {
            ALOGE("%s: Failed to init audio route controls, aborting.", __func__);
            if( my_data )
            {
                delete my_data;
            }
            return nullptr;
        }

        adev->_mixer = mixer;
        adev->_snd_card = snd_card;
        break;
    }

    if( snd_card >= MAX_SND_CARD )
    {
        ALOGE("%s: Unable to find correct sound card, aborting.", __func__);
        if( my_data )
        {
            delete my_data;
        }
        return nullptr;
    }

    mixer_ctl *mctl;
    mctl = mixer_get_ctl_by_name(mixer, "d1in4_mix_enable Switch");
    mixer_ctl_set_value(mctl, 0, 1);
    mctl = mixer_get_ctl_by_name(mixer, "dsp1_enable Switch");
    mixer_ctl_set_value(mctl, 0, 1);
    mctl = mixer_get_ctl_by_name(mixer, "DAC2 output out2");
    mixer_ctl_set_enum_by_string(mctl, "Reserved");
    mctl = mixer_get_ctl_by_name(mixer, "out1_spkr_en Switch");
    mixer_ctl_set_value(mctl, 0, 1);

    my_data->source_mic_type = SOURCE_DUAL_MIC;
    my_data->source_mic_type |= SOURCE_MONO_MIC;

    my_data->fluence_in_spkr_mode = false;
    my_data->fluence_in_voice_call = false;
    my_data->fluence_in_voice_comm = false;
    my_data->fluence_in_voice_rec = false;

    return my_data;
}

void platform_deinit(platform_data *my_data)
{
    if( my_data->snd_card_name )
    {
        free(my_data->snd_card_name);
    }
    delete my_data;
}
