#ifndef __INCLUDED_ACOUSTIC__
#define __INCLUDED_ACOUSTIC__

int acoustic_manager_destroy(int acoustic_manager);
int acoustic_manager_reset(int acoustic_manager);
int acoustic_manager_process(int acoustic_manager, signed short *buffer, size_t buf_size);
int acoustic_manager_init(int *acoustic_manager, uint32_t sample_rate, uint32_t mask, uint32_t device);

//int acoustic_manager_force_reload();
//int acoustic_manager_load_config();

#endif
