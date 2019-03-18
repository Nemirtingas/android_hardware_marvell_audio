/*
 * Copyright (C) 2016 The CyanogenMod Project
 *               2017 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __INCLUDED_VCM__
#define __INCLUDED_VCM__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif 

int VCMInit();
void VCMDeinit();
int VCMAudioProfileSet(int , int arg2, int arg3);
int VCMAudioProfileMute(uint8_t dir, uint32_t msg_id, uint32_t res);
int VCMAudioProfileVolumeSet(uint8_t dir, uint8_t gain1, uint8_t gain2, uint32_t volume);
int VCMAudioStreamDrain(int res, int timeout);
int VCMAudioParameterSet(int param_id, void *parameter, size_t parameter_size);
int VCMAudioParameterGet( int request, void *param, int32_t *out );
int VCMAudioStreamOutStart(int start, int near_far_end, int comb_with_call, int8_t codec, int32_t * stream);
int VCMAudioStreamOutStop(int32_t stream_id);
int VCMAudioStreamWrite(int start, const void* buff, size_t *size);
int VCMAudioStreamInStop(int stream_id);
int VCMAudioStreamInStart(int start, int near_far_end, int codec, int32_t * stream);
int VCMAudioStreamRead(int32_t stream_id, void *buff, size_t* size);

#ifdef __cplusplus
}
#endif

#endif
