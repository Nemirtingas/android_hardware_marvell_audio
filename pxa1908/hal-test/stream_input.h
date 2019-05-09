#ifndef __INCLUDED_INPUTSTREAM_H__
#define __INCLUDED_INPUTSTREAM_H__

#include "common.h"

class audio_device;

class stream_input
{
    public:
    friend class audio_device;

    audio_stream_in      _in;
    android::Mutex       _lock;
    struct pcm_config    _config;
    struct pcm          *_pcm;
    bool                 _standby;
    int                  _source;
    int                  _pcm_device_id;
    audio_devices_t      _device;
    audio_channel_mask_t _channel_mask;
    audio_usecase_t      _usecase;
    audio_io_handle_t    _capture_handle;
    audio_input_flags_t  _flags;
    audio_device        *_dev;
    bool                 _enable_aec;
    bool                 _enable_ns;


    ssize_t start_input_stream();
    int stop_input_stream();

    public:
        stream_input();
        ~stream_input();

        int            init(audio_hw_device* adev, audio_io_handle_t handle, audio_devices_t devices, audio_config* config, audio_stream_in** stream, audio_input_flags_t flags, const char* address, audio_source_t source);
        void           close();
        uint32_t       get_sample_rate();
        int            set_sample_rate(uint32_t rate);
        size_t         get_buffer_size();
        uint32_t       get_channels();
        audio_format_t get_format();
        int            set_format(audio_format_t format);
        int            standby();
        int            dump(int fd);
        int            set_parameters(const char *kvpairs);
        char*          get_parameters(const char *keys);
        int            set_gain(float gain);
        ssize_t        read(void *buffer, size_t bytes);
        uint32_t       get_input_frames_lost();
        int            add_audio_effect(effect_handle_t effect);
        int            remove_audio_effect(effect_handle_t effect);
};

#endif
