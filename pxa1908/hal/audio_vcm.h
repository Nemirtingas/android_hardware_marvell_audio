#ifndef __INCLUDED_VCM__
#define __INCLUDED_VCM__

#ifdef __cplusplus
extern "C" {
#endif

int convert2_profile(audio_devices_t device, int vcm_param);
int vcm_check_init();
void vcm_mute_all(int direction, int reserved);
void vcm_mute_mic(int direction, int reserved);
void vcm_setvolume(uint8_t input_gain1, uint8_t input_gain2, uint8_t output_gain1, uint8_t output_gain2, uint32_t volume);
void vcm_set_user_eq(void* parameter, size_t parameter_size);
void vcm_select_path(audio_devices_t device, int a2, int params);
void vcm_recording_stop();
int vcm_recording_read(void *buffer, size_t size);
int vcm_recording_start();
int vcm_set_loopback(audio_devices_t device, bool arg2);

#ifdef __cplusplus
}
#endif

#endif
