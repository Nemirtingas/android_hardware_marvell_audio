#include "stream_input.h"
#include "platform.h"
#include "audio_device.h"

stream_input::stream_input()
{

}

stream_input::~stream_input()
{

}

int stream_input::init(audio_hw_device* adev, audio_io_handle_t handle, audio_devices_t devices, audio_config* config, audio_stream_in** stream_in, audio_input_flags_t flags, const char* address, audio_source_t source)
{


    *stream_in = &_in;

    return 0;
}

void stream_input::close()
{

}

uint32_t stream_input::get_sample_rate()
{
    return 0;
}

int stream_input::set_sample_rate(uint32_t rate)
{
    return -ENOSYS;
}

size_t stream_input::get_buffer_size()
{
    return -ENOSYS;
}

uint32_t stream_input::get_channels()
{
    return -ENOSYS;
}

audio_format_t stream_input::get_format()
{
    return AUDIO_FORMAT_DEFAULT;
}

int stream_input::set_format(audio_format_t format)
{
    return -ENOSYS;
}

int stream_input::standby()
{
    return -ENOSYS;
}

int stream_input::dump(int fd)
{
    return -ENOSYS;
}

int stream_input::set_parameters(const char *kvpairs)
{
    return -ENOSYS;
}

char* stream_input::get_parameters(const char *keys)
{
    return strdup("");
}

int stream_input::set_gain(float gain)
{
    return -ENOSYS;
}

ssize_t stream_input::read(void *buffer, size_t bytes)
{
    return -ENOSYS;
}

uint32_t stream_input::get_input_frames_lost()
{
    return -ENOSYS;
}

int stream_input::add_audio_effect(effect_handle_t effect)
{
    return -ENOSYS;
}

int stream_input::remove_audio_effect(effect_handle_t effect)
{
    return -ENOSYS;
}
