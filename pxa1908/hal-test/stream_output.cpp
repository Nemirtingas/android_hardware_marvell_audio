#include "stream_output.h"
#include "platform.h"
#include "audio_device.h"

using namespace android;

ssize_t stream_output::start_output_stream()
{
    audio_usecase uc_info;

    _pcm_device_id = platform_get_pcm_device_id(_usecase, _devices);

    if( _pcm_device_id < 0 )
    {
        return -EINVAL;
    }

    uc_info.id   = _usecase;
    uc_info.type = PCM_PLAYBACK;
    uc_info.stream.out = this;
    uc_info.in_snd_device = SND_DEVICE_NONE;
    uc_info.out_snd_device = SND_DEVICE_NONE;

    //if( _devices & AUDIO_DEVICE_OUT_AUX_DIGITAL )
        //check_and_set_hdmi_channels(_dev, _config.channels);

    _dev->add_usecase(std::move(uc_info));

    _dev->select_devices(_usecase);

    if( _usecase != USECASE_AUDIO_PLAYBACK_OFFLOAD )
    {
        unsigned int flags = PCM_OUT;
        unsigned int pcm_open_retry_count = 0;
        if( _usecase == USECASE_AUDIO_PLAYBACK_AFE_PROXY )
        {
            flags |= PCM_MMAP | PCM_NOIRQ;
            pcm_open_retry_count = PROXY_OPEN_RETRY_COUNT;
        }
        else
            flags |= PCM_MONOTONIC | PCM_NORESTART;

        while(true)
        {
            _pcm = pcm_open(_dev->_snd_card, _pcm_device_id, flags, &_config);
            if( _pcm == nullptr || !pcm_is_ready(_pcm) )
            {
                if( _pcm != nullptr )
                {
                    pcm_close(_pcm);
                    _pcm = nullptr;
                }
                if( pcm_open_retry_count-- == 0 )
                {
                    stop_output_stream();
                    return -EIO;
                }
                usleep(PROXY_OPEN_WAIT_TIME*1000);
                continue;
            }
            break;
        }
        if( pcm_is_ready(_pcm) )
            pcm_prepare(_pcm);
    }

    return 0;
}

int stream_output::stop_output_stream()
{
    int ret = 0;

    _dev->del_usecase(_usecase);

    //if( _devices & AUDIO_DEVICE_OUT_AUX_DIGITAL )
    //    check_and_set_hdmi_channels(_dev, DEFAULT_HDMI_OUT_CHANNELS);

    return ret;
}

stream_output::stream_output()
{
}

stream_output::~stream_output()
{

}

int stream_output::init(audio_device *adev, audio_io_handle_t handle, audio_devices_t devices, audio_output_flags_t flags, audio_config* config, audio_stream_out** stream_out, const char* address)
{
    _flags = flags;
    _devices = devices;
    _dev = adev;
    _format = config->format;
    _sample_rate = config->sample_rate;
    _channel_mask = AUDIO_CHANNEL_OUT_STEREO;
    _supported_channel_masks[0] = AUDIO_CHANNEL_OUT_STEREO;
    _handle = handle;

    if( _devices == AUDIO_DEVICE_OUT_TELEPHONY_TX )
    {
        if( config->sample_rate == 0 )
            config->sample_rate = AFE_PROXY_SAMPLING_RATE;
        if( config->sample_rate != 48000 && config->sample_rate != 16000 &&
            config->sample_rate != 8000 )
        {
            config->sample_rate = AFE_PROXY_SAMPLING_RATE;
            *stream_out = NULL;
            return -EINVAL;
        }
        _sample_rate = config->sample_rate;
        _config.rate = config->sample_rate;
        if( config->format == AUDIO_FORMAT_DEFAULT )
            config->format = AUDIO_FORMAT_PCM_16_BIT;
        if( config->format != AUDIO_FORMAT_PCM_16_BIT )
        {
            config->format = AUDIO_FORMAT_PCM_16_BIT;
            return -EINVAL;
        }
        _format = config->format;
        _usecase = USECASE_AUDIO_PLAYBACK_AFE_PROXY;
        _config = pcm_config_afe_proxy_playback;
        _dev->_voice_tx_output = this;
    }
    else
    {
        if( _flags & AUDIO_OUTPUT_FLAG_DEEP_BUFFER )
        {
            _usecase = USECASE_AUDIO_PLAYBACK_DEEP_BUFFER;
            _config = pcm_config_deep_buffer;
        }
        else if( _flags & AUDIO_OUTPUT_FLAG_TTS )
        {
            _usecase = USECASE_AUDIO_PLAYBACK_TTS;
            _config = pcm_config_deep_buffer;
        }
        else if( _flags & AUDIO_OUTPUT_FLAG_RAW )
        {
            _usecase = USECASE_AUDIO_PLAYBACK_ULL;
            _config = pcm_config_low_latency;
        }
        else
        {
            _usecase = USECASE_AUDIO_PLAYBACK_LOW_LATENCY;
            _config = pcm_config_low_latency;
        }

        if( _format != audio_format_from_pcm_format(_config.format) )
        {
            _format = audio_format_from_pcm_format(_config.format);
        }
        _sample_rate = _config.rate;
    }

    if( _flags & AUDIO_OUTPUT_FLAG_PRIMARY )
    {
        if( adev->_primary_output == nullptr )
            adev->_primary_output = this;
        else
        {
            ALOGE("%s: Primary output is already opened", __func__);
            return -EEXIST;
        }
    }

    {
        Mutex::Autolock device_lock(adev->_lock);
        if( adev->get_usecase_from_list(_usecase) != nullptr )
        {
            ALOGE("%s: Usecase (%d) is already present", __func__, _usecase);
            return -EEXIST;
        }
    }
    _standby = true;

    *stream_out = &_out;
    config->format = get_format();
    config->channel_mask = get_channels();
    config->sample_rate = get_sample_rate();

    return 0;
}

void stream_output::close()
{
    standby();
    if( _dev->_voice_tx_output == this )
        _dev->_voice_tx_output = nullptr;
}

uint32_t stream_output::get_sample_rate()
{
    return _sample_rate;
}

int stream_output::set_sample_rate(uint32_t rate)
{
    return -ENOSYS;
}

size_t stream_output::get_buffer_size()
{
    return _config.period_size * audio_stream_out_frame_size(&_out);
}

uint32_t stream_output::get_channels()
{
    return _channel_mask;
}

audio_format_t stream_output::get_format()
{
    return _format;
}

int stream_output::set_format(audio_format_t format)
{
    return -ENOSYS;
}

int stream_output::standby()
{
    Mutex::Autolock lock1(_lock);

    if( !_standby )
    {
        Mutex::Autolock lock2(_dev->_lock);
        _standby = true;
        if( _pcm != nullptr )
        {
            pcm_close(_pcm);
            _pcm = nullptr;
        }
    }
    return 0;
}

int stream_output::dump(int fd)
{
    return 0;
}

int stream_output::set_parameters(const char *kvpairs)
{
    struct str_parms *parms;
    char value[32];
    int status = 0;
    int ret;
    int val;

    parms = str_parms_create_str(kvpairs);
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING, value, sizeof(value));
    if( ret >= 0 )
    {
        val = atoi(value);
        Mutex::Autolock lock(_lock);
        Mutex::Autolock lock2(_dev->_lock);

        if( _devices == AUDIO_DEVICE_OUT_AUX_DIGITAL && val == AUDIO_DEVICE_NONE )
        {
            val = AUDIO_DEVICE_OUT_SPEAKER;
        }

        if( val != 0 )
        {
            _devices = val;
            if( !_standby )
                _dev->select_devices(_usecase);

            if( _dev->output_drives_call(this) )
            {
                if( !voice_is_in_call(_dev) )
                {
                    if( _dev->_mode != AUDIO_MODE_IN_CALL )
                    {
                        _dev->_current_call_output = this;
                        ret = voice_start_call(_dev);
                    }
                }
                else
                {
                    _dev->_current_call_output = this;
                    voice_update_devices_for_all_voice_usecases(_dev);
                }
            }
        }
    }

    str_parms_destroy(parms);

    return status;
}

char* stream_output::get_parameters(const char *keys)
{
    struct str_parms *query = str_parms_create_str(keys);
    char *str;
    char value[256];
    struct str_parms *reply = str_parms_create();
    size_t i, j;
    int ret;
    bool first = true;
    ALOGV("%s: enter: keys - %s", __func__, keys);
    ret = str_parms_get_str(query, AUDIO_PARAMETER_STREAM_SUP_CHANNELS, value, sizeof(value));
    if (ret >= 0)
    {
        value[0] = '\0';
        i = 0;
        while (_supported_channel_masks[i] != 0)
        {
            for (j = 0; j < ARRAY_SIZE(out_channels_name_to_enum_table); j++)
            {
                if (out_channels_name_to_enum_table[j].value == _supported_channel_masks[i])
                {
                    if (!first)
                    {
                        strcat(value, "|");
                    }
                    strcat(value, out_channels_name_to_enum_table[j].name);
                    first = false;
                    break;
                }
            }
            i++;
        }
        str_parms_add_str(reply, AUDIO_PARAMETER_STREAM_SUP_CHANNELS, value);
        str = str_parms_to_str(reply);
    }
    else
    {
        str = strdup(keys);
    }
    str_parms_destroy(query);
    str_parms_destroy(reply);
    ALOGV("%s: exit: returns - %s", __func__, str);
    return str;
}

uint32_t stream_output::get_latency()
{
    return _config.period_count * _config.period_size * 1000 / _config.rate;
}

int stream_output::set_volume(float left, float right)
{
    if( _usecase == USECASE_AUDIO_PLAYBACK_MULTI_CH )
    {
        _muted = (left == 0.0);
        return 0;
    }

    return -ENOSYS;
}

ssize_t stream_output::write(const void *buffer, size_t bytes)
{
    ssize_t ret = 0;
    Mutex::Autolock lock_stream(_lock);

    if( _standby )
    {
        _standby = false;
        _dev->_lock.lock();
        ret = start_output_stream();
        _dev->_lock.unlock();
        if( ret != 0 )
        {
            _standby = true;
            return bytes;
        }
    }
    if( _pcm )
    {
        if( _muted )
        {
            memset(const_cast<void*>(buffer), 0, bytes);
        }

        ret = pcm_write(_pcm, buffer, bytes);

        if( ret == 0 )
            _written += bytes / (_config.channels * sizeof(short));
    }

    if( ret != 0 )
    {
        standby();
        usleep(bytes * 1000000 / audio_stream_out_frame_size(&_out) / get_sample_rate());
    }

    return ret;
}

int stream_output::get_render_position(uint32_t *dsp_frames)
{
    *dsp_frames = 0;
    return -EINVAL;
}

int stream_output::add_audio_effect(effect_handle_t effect)
{
    return 0;
}

int stream_output::remove_audio_effect(effect_handle_t effect)
{
    return 0;
}

int stream_output::get_next_write_timestamp(int64_t *timestamp)
{
    return -ENOSYS;
}

int stream_output::get_presentation_position(uint64_t *frames, struct timespec *timestamp)
{
    int ret = -1;
    Mutex::Autolock lock(_lock);

    if( _pcm )
    {
        unsigned int avail;
        if( pcm_get_htimestamp(_pcm, &avail, timestamp) == 0 )
        {
            size_t kernel_buffer_size = _config.period_size * _config.period_count;
            int64_t signed_frames = _written - kernel_buffer_size + avail;

            signed_frames -= platform_render_latency(_usecase) *_sample_rate / 1000000LL;

            if( signed_frames >= 0 )
            {
                *frames = signed_frames;
                ret = 0;
            }
        }
    }

    return ret;
}
