#include "audio_aec.h"

#include <stdlib.h>
#include <cutils/log.h>

#if 1
    #ifdef ALOGE
    #undef ALOGE
    #endif
    #define ALOGE(...) {}
    #ifdef ALOGW
    #undef ALOGW
    #endif
    #define ALOGW(...) {}
    #ifdef ALOGI
    #undef ALOGI
    #endif
    #define ALOGI(...) {}
    #ifdef ALOGV
    #undef ALOGV
    #endif
    #define ALOGV(...) {}
    #ifdef ALOGD
    #undef ALOGD
    #endif
    #define ALOGD(...) {}
#endif

extern int EffectRelease(effect_handle_t handle);

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

void echo_ref_rx_write(mrvl_stream_out *out, void *buffer, size_t buff_size)
{
    echo_reference_buffer erbuff;
    unsigned int avail = 0;
    mrvl_audio_device *madev = out->dev;

    if( madev->echo_reference )
    {
        erbuff.raw = buffer;
        // count frames (PCM_16bits)
        erbuff.frame_count = (buff_size / 2) / popcount(out->channel_mask);
        pcm_get_htimestamp(out->handle, &avail, &erbuff.time_stamp);
        erbuff.delay_ns = 1000000000
                          * (out->period_count * out->period_size - avail)
                          / out->sample_rate;
        madev->echo_reference->write(madev->echo_reference, &erbuff);
    }
}

int effect_rx_process()
{
    return 0;
}

int effect_set_profile()
{
    return 0;
}

struct mrvl_audio_effect* out_get_effect(struct audio_stream *out, effect_uuid_t *effect)
{
    struct mrvl_stream_out *maout = (struct mrvl_stream_out*)out;
    struct listnode *node;
    effect_descriptor_t desc;

    pthread_mutex_lock(&maout->lock);

    /*
    list_for_each(node, &maout->effect_interfaces)
    {
        struct mrvl_audio_effect *tmp_item = node_to_item(node, struct mrvl_audio_effect, link);
        lib_entry_t *tmp_lib = (lib_entry_t*)tmp_item->effect->itfe;
        struct effect_interface_s *tmp_itfe = (struct effect_interface_s*)tmp_lib->desc;
        tmp_itfe->get_descriptor(effect, &desc);
        //if( !memcmp(effect, &desc.type, sizeof(effect_uuid_t)) )
        if( !memcmp(effect, &desc.uuid, sizeof(effect_uuid_t)) )
        {
            pthread_mutex_unlock(&maout->lock);
            return tmp_item;
        }
    }
    */

    pthread_mutex_unlock(&maout->lock);
    return 0;
}


void out_load_effect(struct audio_stream *out, effect_uuid_t *effect, uint32_t profile)
{
}

int out_release_effect()
{
    return 0;
}

int in_pre_process()
{
    return 0;
}

int in_add_audio_effect(struct audio_stream *in, effect_entry_t *effect )
{
    struct mrvl_stream_in *mrvl_in = (struct mrvl_stream_in*)in;
    struct mrvl_audio_effect *mrvl_effect;
    struct listnode *node;

    ALOGI("%s: add effect %p", __FUNCTION__, effect);


    if( effect == NULL )
        return 0;

    pthread_mutex_lock(&mrvl_in->lock);

    /*
    list_for_each(node, &mrvl_in->effect_interfaces)
    {
        if( node_to_item(node, struct mrvl_audio_effect, link) == effect )
        {
            pthread_mutex_unlock(&mrvl_in->lock);
            return 0;
        }
    }

    mrvl_effect = (struct mrvl_audio_effect*)malloc(sizeof(struct mrvl_audio_effect));
    if( !mrvl_effect )
    {
        ALOGE("%s: no memory to malloc", __FUNCTION__);
        pthread_mutex_unlock(&mrvl_in->lock);
        return -ENOMEM;
    }

    mrvl_effect->effect = effect;
    list_add_tail(i, mrvl_effect);
    */

    pthread_mutex_unlock(&mrvl_in->lock);
    return 0;
}

int in_load_effect()
{
    return 0;
}

struct mrvl_audio_effect* in_get_effect(struct audio_stream *in, effect_uuid_t *effect)
{
    struct mrvl_stream_in *ma_in = (struct mrvl_stream_in*)in;
    struct listnode *node;
    effect_descriptor_t desc;

    pthread_mutex_lock(&ma_in->lock);

    /*
    list_for_each(node, &ma_in->effect_interfaces)
    {
        struct mrvl_audio_effect *tmp_item = node_to_item(node, struct mrvl_audio_effect, link);
        lib_entry_t *tmp_lib = (lib_entry_t*)(((effect_entry_t*)tmp_item->effect)->itfe);
        struct effect_interface_s *tmp_itfe = (struct effect_interface_s*)(tmp_lib->desc);
        tmp_itfe->get_descriptor(effect, &desc);
        //if( !memcmp(effect, &desc.type, sizeof(effect_uuid_t)) )
        if( !memcmp(effect, &desc.uuid, sizeof(effect_uuid_t)) )
        {
            pthread_mutex_unlock(&ma_in->lock);
            return tmp_item;
        }
    }
    */

    pthread_mutex_unlock(&ma_in->lock);
    return 0;
}

int in_remove_audio_effect(struct audio_stream *in, struct mrvl_audio_effect *effect)
{
    struct mrvl_stream_in *ma_in = (struct mrvl_stream_in*)in;
    struct listnode *plist;
    struct listnode *tmp_node;
    struct mrvl_audio_effect *tmp_effect;

    pthread_mutex_lock(&ma_in->lock);

    list_for_each_safe(plist, tmp_node, &ma_in->effect_interfaces)
    {
        tmp_effect = node_to_item(plist, struct mrvl_audio_effect, link);
        if( tmp_effect != NULL && tmp_effect->effect == effect->effect )
        {
            list_remove(&tmp_effect->link);
            free(tmp_effect);
        }
    }

    pthread_mutex_unlock(&ma_in->lock);

    return 0;
}

int in_release_effect(struct audio_stream * in, effect_uuid_t *effect)
{
    int res = 0;
    struct mrvl_audio_effect *maeffect;

    maeffect = in_get_effect(in, effect);
    if( maeffect )
    {
        ALOGI("%s: release effect for input stream %p", __FUNCTION__, in);
        in_remove_audio_effect(in, maeffect);
        res = EffectRelease(maeffect->effect);
    }

    return res;
}
