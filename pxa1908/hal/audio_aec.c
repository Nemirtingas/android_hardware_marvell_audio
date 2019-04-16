#include "audio_aec.h"

#define LOG_TAG "audio_hw_mrvl"
#define LOG_NDEBUG 0

#include <stdlib.h>
#include <cutils/log.h>

void create_echo_ref(struct mrvl_stream_in *in, struct mrvl_stream_out *out)
{
    struct mrvl_audio_device *dev;

    if( !in || !out )
    {
        ALOGE("%s/%d:in %p or out %p pointer is null", __FUNCTION__, __LINE__, in, out);
        return;
    }

    ALOGI("%s/%d:in %p out %p", __FUNCTION__, __LINE__, in, out);

    dev = out->dev;
    pthread_mutex_lock(&out->lock);
    if( !dev->echo_reference )
    {
        struct echo_reference_itfe *itfe = NULL;
        ALOGI("%s/%d: create echo reference, in->format %d, in->channel_count %d, in->sample_rate %d, out->format %d, out->chan"
              "nel_count %d, out->sample_rate %d, max_frame_count %d", __FUNCTION__, __LINE__,
              in->format, popcount(in->channel_mask), in->sample_rate,
              out->format, popcount(out->channel_mask), out->sample_rate,
              9984);

        create_echo_reference(in->format, popcount(in->channel_mask), in->sample_rate,
                              out->format, popcount(out->channel_mask), out->sample_rate,
                              &itfe);

        if( itfe )
        {
            ALOGI("%s/%d: echo reference %p create success.", __FUNCTION__, __LINE__, itfe);

            in->ref_buffer.raw = malloc(popcount(in->channel_mask) * sizeof(uint16_t) *in->period_size);
            if( !in->ref_buffer.raw )
            {
                ALOGI("%s/%d: malloc failed!", __FUNCTION__, __LINE__);
                release_echo_reference(itfe);
            }
            dev->echo_reference = itfe;
            in->ref_buffer.frame_count = in->period_size;
        }
    }
    pthread_mutex_unlock(&out->lock);
}

void remove_echo_ref(struct mrvl_stream_in *in, struct mrvl_stream_out *out)
{
    struct mrvl_audio_device *dev;
    if( !in || !out )
    {
        ALOGI("%s/%d:in %p or out %p pointer is null", __FUNCTION__, __LINE__, in, out);
        return;
    }
    dev = out->dev;
    ALOGI("%s/%d:in %p out %p echo_ref %p", __FUNCTION__, __LINE__, in, out, dev->echo_reference);
    pthread_mutex_lock(&out->lock);
    if( dev->echo_reference )
    {
        release_echo_reference(dev->echo_reference);
        dev->echo_reference = NULL;
        if( in->ref_buffer.raw )
        {
            free(in->ref_buffer.raw);
            in->ref_buffer.raw = NULL;
        }
    }
    pthread_mutex_unlock(&out->lock);
}

int echo_ref_rx_write()
{
    return 0;
}

int effect_rx_process()
{
    return 0;
}

int effect_set_profile()
{
    return 0;
}

void out_load_effect(struct audio_stream *out, effect_uuid_t *effect, uint32_t profile);
{
    return 0;
}

int out_release_effect()
{
    return 0;
}

int in_pre_process()
{
    return 0;
}

int in_load_effect()
{
    return 0;
}

int in_release_effect(struct audio_stream * in, effect_uuid_t *effect)
{
    return 0;
}
