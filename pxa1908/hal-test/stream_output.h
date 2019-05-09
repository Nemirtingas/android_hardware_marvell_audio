#ifndef __INCLUDED_OUTPUTSTREAM_H__
#define __INCLUDED_OUTPUTSTREAM_H__

#include "common.h"

class audio_device;

class stream_output
{
    public:
    friend class audio_device;

    audio_stream_out     _out;
    android::Mutex       _lock;
    struct pcm_config    _config;
    struct pcm          *_pcm;
    bool                 _standby;
    int                  _pcm_device_id;
    unsigned int         _sample_rate;
    audio_channel_mask_t _channel_mask;
    audio_format_t       _format;
    audio_devices_t      _devices;
    audio_output_flags_t _flags;
    audio_usecase_t      _usecase;
    audio_channel_mask_t _supported_channel_masks[MAX_SUPPORTED_CHANNEL_MASKS+1];
    bool                 _muted;
    uint64_t             _written;
    audio_io_handle_t    _handle;
    audio_device        *_dev;

    ssize_t start_output_stream();
    int stop_output_stream();

    public:
        stream_output();
        ~stream_output();

        int            init(audio_device *adev, audio_io_handle_t handle, audio_devices_t devices, audio_output_flags_t flags, audio_config* config, audio_stream_out** stream_out, const char* address);
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
        uint32_t       get_latency();
        int            set_volume(float left, float right);
        ssize_t        write(const void *buffer, size_t bytes);
        int            get_render_position(uint32_t *dsp_frames);
        int            add_audio_effect(effect_handle_t effect);
        int            remove_audio_effect(effect_handle_t effect);
        int            get_next_write_timestamp(int64_t *timestamp);
        int            get_presentation_position(uint64_t *frames, struct timespec *timestamp);
};

#endif
