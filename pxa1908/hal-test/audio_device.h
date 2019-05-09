#ifndef __INCLUDED_AUDIODEVICE_H__
#define __INCLUDED_AUDIODEVICE_H__


#include "stream_input.h"
#include "stream_output.h"
#include "platform.h"
#include "voice.h"

class audio_device
{
    public:
    friend class stream_input;
    friend class stream_output;

    audio_hw_device     _device;
    android::Mutex      _lock;
    stream_input       *_active_input;
    stream_output      *_primary_output;
    stream_output      *_voice_tx_output;
    stream_output      *_current_call_output;
    platform_data      *_platform;
    mixer              *_mixer;
    audio_route        *_audio_route;
    int  _snd_card;
    audio_mode_t  _mode;
    bool _mic_muted;
    bool _enable_voicerx;
    bool _bt_wb_speech_enabled;
    bool _bluetooth_nrec;
    int  _snd_dev_ref_cnt[SND_DEVICE_MAX];

    pcm_params         *_use_case_table[AUDIO_USECASE_MAX];
    voice _voice;

    std::list<audio_usecase> _usecase_list;

    public:
        audio_device();
        ~audio_device();

        hw_device_t* get_hw_device();
        void add_usecase(audio_usecase &&uc);
        void del_usecase(audio_usecase_t uc_id);
        audio_usecase *get_usecase_from_list(audio_usecase_t uc_id);
        audio_usecase_t get_voice_usecase_id_from_list();
        int select_devices(audio_usecase_t ud_id);
        bool output_drives_call(stream_output *out);

        void check_and_route_playback_usecases(audio_usecase *uc_info, snd_device_t snd_device);
        void check_and_route_capture_usecases(audio_usecase *uc_info, snd_device_t snd_device);
        int disable_audio_route(audio_usecase *usecase);
        int disable_snd_device(snd_device_t snd_device);
        int enable_snd_device(snd_device_t snd_device);
        int enable_audio_route(audio_usecase *usecase);

        int init(const hw_module_t *module, const char *name, hw_device_t **dev);
        int close();
        int get_mic_mute(bool *state);
        int get_input_buffer_size(const audio_config *config);
        int dump(int fd);
};

#endif
