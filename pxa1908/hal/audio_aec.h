#ifndef __INCLUDED_AEC__
#define __INCLUDED_AEC__

#include "audio_hw_mrvl.h"
#include <hardware/audio_effect.h>


static effect_uuid_t fx_iid_voiptx = {
    0x9df810e0, 0xedb1, 0x11e1, 0x9d46,
    {0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b}
};

static effect_uuid_t fx_iid_voiprx = {
    0x3015f5e0, 0xedb3, 0x11e1, 0x8f89,
    {0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b}
};

#define FX_IID_VOIPTX &fx_iid_voiptx
#define FX_IID_VOIPRX &fx_iid_voiprx

void create_echo_ref(struct mrvl_stream_in *in, struct mrvl_stream_out *out);
void remove_echo_ref(struct mrvl_stream_in *in, struct mrvl_stream_out *out);
int echo_ref_rx_write();

int effect_rx_process();
int effect_set_profile();

void out_load_effect(struct audio_stream *out, effect_uuid_t *effect, uint32_t profile);
int out_release_effect();

int in_pre_process();
int in_load_effect();
int in_release_effect(struct audio_stream * in, effect_uuid_t *effect);

#endif
