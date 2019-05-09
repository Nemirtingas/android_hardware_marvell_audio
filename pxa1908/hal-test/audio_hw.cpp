#include "audio_device.h"

static audio_device* adev = nullptr;
static pthread_mutex_t init_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned int ref_count = 0;

//////////////////////////////////////////////////////////
//                                                      //
//                  HAL FUNCTIONS                       //
//                                                      //
//////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//                                                      //
//                  HAL OUTPUT STREAM                   //
//                                                      //
//////////////////////////////////////////////////////////
static uint32_t out_get_sample_rate(const struct audio_stream *stream)
{
    stream_output *out = (stream_output*)stream;
    return out->get_sample_rate();
}

static int out_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    stream_output *out = (stream_output*)stream;
    return out->set_sample_rate(rate);
}

static size_t out_get_buffer_size(const struct audio_stream *stream)
{
    stream_output *out = (stream_output*)stream;
    return out->get_buffer_size();
}

static uint32_t out_get_channels(const struct audio_stream *stream)
{
    stream_output *out = (stream_output*)stream;
    return out->get_channels();
}

static audio_format_t out_get_format(const struct audio_stream *stream)
{
    stream_output *out = (stream_output*)stream;
    return out->get_format();
}

static int out_set_format(struct audio_stream *stream, audio_format_t format)
{
    stream_output *out = (stream_output*)stream;
    return out->set_format(format);
}

static int out_standby(struct audio_stream *stream)
{
    stream_output *out = (stream_output*)stream;
    return out->standby();
}

static int out_dump(const struct audio_stream *stream, int fd)
{
    stream_output *out = (stream_output*)stream;
    return out->dump(fd);
}

static int out_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
    stream_output *out = (stream_output*)stream;
    return out->set_parameters(kvpairs);
}

static char* out_get_parameters(const struct audio_stream *stream, const char *keys)
{
    stream_output *out = (stream_output*)stream;
    return out->get_parameters(keys);
}

static uint32_t out_get_latency(const struct audio_stream_out *stream)
{
    stream_output *out = (stream_output*)stream;
    return out->get_latency();
}

static int out_set_volume(struct audio_stream_out *stream, float left, float right)
{
    stream_output *out = (stream_output*)stream;
    return out->set_volume(left, right);
}

static ssize_t out_write(struct audio_stream_out *stream, const void *buffer,
                         size_t bytes)
{
    stream_output *out = (stream_output*)stream;
    return out->write(buffer, bytes);
}

static int out_get_render_position(const struct audio_stream_out *stream,
                                   uint32_t *dsp_frames)
{
    stream_output *out = (stream_output*)stream;
    return out->get_render_position(dsp_frames);
}

static int out_add_audio_effect(const struct audio_stream *stream,
                                effect_handle_t effect)
{
    stream_output *out = (stream_output*)stream;
    return out->add_audio_effect(effect);
}

static int out_remove_audio_effect(const struct audio_stream *stream,
                                   effect_handle_t effect)
{
    stream_output *out = (stream_output*)stream;
    return out->remove_audio_effect(effect);
}

static int out_get_next_write_timestamp(const struct audio_stream_out *stream,
                                        int64_t *timestamp)
{
    stream_output *out = (stream_output*)stream;
    return out->get_next_write_timestamp(timestamp);
}

static int out_get_presentation_position(const struct audio_stream_out *stream,
                                   uint64_t *frames, struct timespec *timestamp)
{
    stream_output *out = (stream_output*)stream;
    return out->get_presentation_position(frames, timestamp);
}

static int adev_open_output_stream(struct audio_hw_device *dev,
                                   audio_io_handle_t handle,
                                   audio_devices_t devices,
                                   audio_output_flags_t flags,
                                   struct audio_config *config,
                                   struct audio_stream_out **stream_out,
                                   const char *address)
{
    stream_output *stream = new stream_output();

    if( stream == nullptr )
    {
        *stream_out = nullptr;
        return -ENOMEM;
    }

    int res = stream->init((audio_device*)dev, handle, devices, flags, config, stream_out, address);

    if( res )
    {
        ALOGE("%s: Failed to open an ouput stream", __func__);
        delete stream;
        *stream_out = nullptr;
    }
    else
    {
        (*stream_out)->common.get_sample_rate     = out_get_sample_rate;
        (*stream_out)->common.set_sample_rate     = out_set_sample_rate;
        (*stream_out)->common.get_buffer_size     = out_get_buffer_size;
        (*stream_out)->common.get_channels        = out_get_channels;
        (*stream_out)->common.get_format          = out_get_format;
        (*stream_out)->common.set_format          = out_set_format;
        (*stream_out)->common.standby             = out_standby;
        (*stream_out)->common.dump                = out_dump;
        (*stream_out)->common.set_parameters      = out_set_parameters;
        (*stream_out)->common.get_parameters      = out_get_parameters;
        (*stream_out)->common.add_audio_effect    = out_add_audio_effect;
        (*stream_out)->common.remove_audio_effect = out_remove_audio_effect;
        (*stream_out)->get_latency                = out_get_latency;
        (*stream_out)->set_volume                 = out_set_volume;
        (*stream_out)->write                      = out_write;
        (*stream_out)->get_render_position        = out_get_render_position;
        (*stream_out)->get_next_write_timestamp   = out_get_next_write_timestamp;
        (*stream_out)->get_presentation_position  = out_get_presentation_position;
    }

    return res;
}

static void adev_close_output_stream(struct audio_hw_device *dev,
                                     struct audio_stream_out *stream)
{
    stream_output *out = (stream_output*)stream;
    out->close();
    delete out;
}

//////////////////////////////////////////////////////////
//                                                      //
//                  HAL INPUT STREAM                    //
//                                                      //
//////////////////////////////////////////////////////////
static uint32_t in_get_sample_rate(const struct audio_stream *stream)
{
    stream_input *in = (stream_input *)stream;
    return in->get_sample_rate();
}

static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    stream_input *in = (stream_input *)stream;
    return in->set_sample_rate(rate);
}

static size_t in_get_buffer_size(const struct audio_stream *stream)
{
    stream_input *in = (stream_input *)stream;
    return in->get_buffer_size();
}

static uint32_t in_get_channels(const struct audio_stream *stream)
{
    stream_input *in = (stream_input *)stream;
    return in->get_channels();
}

static audio_format_t in_get_format(const struct audio_stream *stream)
{
    stream_input *in = (stream_input *)stream;
    return in->get_format();
}

static int in_set_format(struct audio_stream *stream __unused, audio_format_t format)
{
    stream_input *in = (stream_input *)stream;
    return in->set_format(format);
}

static int in_standby(struct audio_stream *stream)
{
    stream_input *in = (stream_input *)stream;
    return in->standby();
}

static int in_dump(const struct audio_stream *stream, int fd)
{
    stream_input *in = (stream_input *)stream;
    return in->dump(fd);
}

static int in_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
    stream_input *in = (stream_input *)stream;
    return in->set_parameters(kvpairs);
}

static char* in_get_parameters(const struct audio_stream *stream,
                               const char *keys)
{
    stream_input *in = (stream_input *)stream;
    return in->get_parameters(keys);
}

static int in_set_gain(struct audio_stream_in *stream, float gain)
{
    stream_input *in = (stream_input *)stream;
    return in->set_gain(gain);
}

static ssize_t in_read(struct audio_stream_in *stream, void *buffer,
                       size_t bytes)
{
    stream_input *in = (stream_input *)stream;
    return in->read(buffer, bytes);
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream)
{
    stream_input *in = (stream_input *)stream;
    return in->get_input_frames_lost();
}

static int in_add_audio_effect(const struct audio_stream *stream,
                               effect_handle_t effect)
{
    stream_input *in = (stream_input *)stream;
    return in->add_audio_effect(effect);
}

static int in_remove_audio_effect(const struct audio_stream *stream,
                                  effect_handle_t effect)
{
    stream_input *in = (stream_input *)stream;
    return in->remove_audio_effect(effect);
}

static int adev_open_input_stream(struct audio_hw_device *dev,
                                  audio_io_handle_t handle,
                                  audio_devices_t devices,
                                  struct audio_config *config,
                                  struct audio_stream_in **stream_in,
                                  audio_input_flags_t flags,
                                  const char *address ,
                                  audio_source_t source )
{
    int res;
    stream_input *stream = new stream_input();
    if( stream == nullptr )
    {
        *stream_in = nullptr;
        return -ENOMEM;
    }

    res = stream->init(dev, handle, devices, config, stream_in, flags, address, source);

    if( res )
    {
        ALOGE("%s: Failed to open an ouput stream", __func__);
        delete stream;
        *stream_in = nullptr;
    }
    else
    {
        (*stream_in)->common.get_sample_rate     = in_get_sample_rate;
        (*stream_in)->common.set_sample_rate     = in_set_sample_rate;
        (*stream_in)->common.get_buffer_size     = in_get_buffer_size;
        (*stream_in)->common.get_channels        = in_get_channels;
        (*stream_in)->common.get_format          = in_get_format;
        (*stream_in)->common.standby             = in_standby;
        (*stream_in)->common.dump                = in_dump;
        (*stream_in)->common.set_parameters      = in_set_parameters;
        (*stream_in)->common.get_parameters      = in_get_parameters;
        (*stream_in)->common.add_audio_effect    = in_add_audio_effect;
        (*stream_in)->common.remove_audio_effect = in_remove_audio_effect;
        (*stream_in)->set_gain                   = in_set_gain;
        (*stream_in)->read                       = in_read;
        (*stream_in)->get_input_frames_lost      = in_get_input_frames_lost;
    }
    return res;
}

static void adev_close_input_stream(struct audio_hw_device *dev __unused,
                                    struct audio_stream_in *stream)
{
    stream_input *in = (stream_input*)stream;
    in->close();
    delete in;
}
//////////////////////////////////////////////////////////
//                                                      //
//                  HAL AUDIO DEVICE                    //
//                                                      //
//////////////////////////////////////////////////////////

static int adev_set_parameters(struct audio_hw_device *dev, const char *kvpairs)
{
    return 0;
}

static char* adev_get_parameters(const struct audio_hw_device *dev,
                                 const char *keys)
{
    return strdup("");
}

static int adev_init_check(const struct audio_hw_device *dev)
{
    return 0;
}

static int adev_set_voice_volume(struct audio_hw_device *dev, float volume)
{
    return 0;
}

static int adev_set_master_volume(struct audio_hw_device *dev, float volume)
{
    return -ENOSYS;
}

static int adev_get_master_volume(struct audio_hw_device *dev, float *volume)
{
    return -ENOSYS;
}

static int adev_set_master_mute(struct audio_hw_device *dev, bool muted)
{
    return -ENOSYS;
}

static int adev_get_master_mute(struct audio_hw_device *dev, bool *muted)
{
    return -ENOSYS;
}

static int adev_set_mode(struct audio_hw_device *dev, audio_mode_t mode)
{
    return 0;
}

static int adev_set_mic_mute(struct audio_hw_device *dev, bool state)
{
    return 0;
}

static int adev_get_mic_mute(const struct audio_hw_device *dev, bool *state)
{
    return adev->get_mic_mute(state);
}

static size_t adev_get_input_buffer_size(const struct audio_hw_device *dev __unused,
                                         const struct audio_config *config)
{
    return adev->get_input_buffer_size(config);
}

static int adev_dump(const audio_hw_device_t *device __unused, int fd )
{
    return adev->dump(fd);
}

static int adev_close(hw_device_t *dev)
{
    pthread_mutex_lock(&init_lock);
    if( ref_count == 0 )
    {
        pthread_mutex_unlock(&init_lock);
        return 0;
    }
    if( --ref_count == 0 )
    {
        adev->close();
        delete adev;
    }
    pthread_mutex_unlock(&init_lock);
    return 0;
}

static int hw_dev_open(const hw_module_t *module, const char *name,
                            hw_device_t **device)
{
    int ret;
    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0) return -EINVAL;

    pthread_mutex_lock(&init_lock);
    if( ref_count != 0 )
    {
        ++ref_count;
        *device = adev->get_hw_device();
        pthread_mutex_unlock(&init_lock);
        return 0;
    }

    adev = new audio_device();
    if( !adev )
    {
        pthread_mutex_unlock(&init_lock);
        return -ENOMEM;
    }

    ret = adev->init(module, name, device);
    if( *device )
    {
        audio_hw_device *dev = (audio_hw_device*)*device;
        dev->common.close          = adev_close;
        dev->init_check            = adev_init_check;
        dev->set_voice_volume      = adev_set_voice_volume;
        dev->set_master_volume     = adev_set_master_volume;
        dev->get_master_volume     = adev_get_master_volume;
        dev->set_master_mute       = adev_set_master_mute;
        dev->get_master_mute       = adev_get_master_mute;
        dev->set_mode              = adev_set_mode;
        dev->set_mic_mute          = adev_set_mic_mute;
        dev->get_mic_mute          = adev_get_mic_mute;
        dev->set_parameters        = adev_set_parameters;
        dev->get_parameters        = adev_get_parameters;
        dev->get_input_buffer_size = adev_get_input_buffer_size;
        dev->open_output_stream    = adev_open_output_stream;
        dev->close_output_stream   = adev_close_output_stream;
        dev->open_input_stream     = adev_open_input_stream;
        dev->close_input_stream    = adev_close_input_stream;
        dev->dump                  = adev_dump;
    }

    return ret;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = hw_dev_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common =
    {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version= AUDIO_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "Marvll audio HW HAL",
        .author = "Nemirtingas (Maxime P)",
        .methods = &hal_module_methods,
        .dso = 0,
        .reserved = {0},
    },
};
