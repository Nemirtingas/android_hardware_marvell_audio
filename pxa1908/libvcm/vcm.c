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

#include <audio_stub.h>

#include "vcm.h"

#define LOG_TAG "VCMAPI"
#include <utils/Log.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>

#include <errno.h>

#define OUTPUT_STREAM_ID 1
#define INPUT_STREAM_ID  2

#define AUDIOCTL_DEV    "/dev/audiostub_ctl"
#define AUDIOOUTPCM_DEV "/dev/audiostub_pcm"
#define AUDIOINPCM_DEV  "/dev/audiostub_pcm"

static int fdaudiostub_ctl = -1;
static int fdoutaudiostub_pcm = -1;
static int fdinaudiostub_pcm = -1;

static size_t dword_3010;
static size_t outBytesWrote = 0;
static int stream_out_written = 0;
static struct pcm_playback_ctlmsg out_playbackctl;

static int stream_in_read = 0;
static struct pcm_record_ctlmsg in_recordctl;

static int VCMAudioIoctl( unsigned int request, void *param )
{
    if( ioctl(fdaudiostub_ctl, request, param) < 0 )
    {
        ALOGE("audio_stub ioctl error: %s", strerror(errno));
        return EACCES;
    }

    return 1;
}

int VCMInit()
{
    if( fdaudiostub_ctl >= 0 || (fdaudiostub_ctl = open(AUDIOCTL_DEV, O_RDONLY)) >= 0 )
        return 1;

    ALOGE("failed to open %s:%s", AUDIOCTL_DEV, strerror(errno));

    return EACCES;
}

void VCMDeinit()
{
    if( fdaudiostub_ctl >= 0 )
    {
        close(fdaudiostub_ctl);
        fdaudiostub_ctl = -1;
    }
}

int VCMAudioProfileSet(int arg1, int path, int arg3)
{
    int request;
    union
    {
        struct path_ctlmsg pathctl;
        struct loop_ctlmsg loopctl;
    } param;


    if( arg3&3 )
    {
        memset(&param, 0, sizeof(param));
        param.loopctl.path = path;
        if( (arg3&3) != 1 )
        {
            if( (arg3&3) == 2 )
                param.loopctl.test_mode = 1;
            else
                param.loopctl.test_mode = 2;
        }

        request = AUDIOSTUB_LOOPBACKCTL;
    }
    else
    {
        param.pathctl.path   = path;
        request = AUDIOSTUB_PATHCTL;
    }

    return VCMAudioIoctl(request, &param);
}

int VCMAudioProfileMute(uint8_t dir, uint32_t msg_id, uint32_t res)
{
    struct mute_ctlmsg param;
    memset(&param, 0, sizeof(param));

    ALOGE("%s: dir=%d, msg_id=%d, res=%d", __func__, (int)dir, (int)msg_id, (int)res);

    // 0 input, 1 output
    param.direction = dir;
    // 0 off, 1 on
    param.mute = msg_id;
    param.reserved = (uint8_t)res;

    return VCMAudioIoctl(AUDIOSTUB_MUTECTL, &param);
}

int VCMAudioProfileVolumeSet(uint8_t dir, uint8_t gain1, uint8_t gain2, uint32_t volume)
{
    struct volume_ctlmsg param;
    memset(&param, 0, sizeof(param));

    param.direction = dir;
    param.gain = gain1 | (gain2 << 8);
    param.misc_volume = volume;

    return VCMAudioIoctl(AUDIOSTUB_VOLUMECTL, &param);
}

int VCMAudioStreamDrain(int res, int timeout)
{
    int16_t param = timeout;
    if( res != 1 )
        return EFAULT;

    return VCMAudioIoctl(AUDIOSTUB_PCMPLAYBACK_DRAIN, &param);
}

int VCMAudioParameterSet(int param_id, void *parameter, size_t parameter_size)
{
    switch(param_id)
    {
        case 0:
            if(parameter_size != 4)
                return EFAULT;
            return VCMAudioIoctl(AUDIOSTUB_SET_PKTSIZE, parameter);

        case 1:
            dword_3010 = *(int32_t*)parameter;
            return EFAULT;

        // Reset mute count
        case 2:
            return VCMAudioIoctl(AUDIOSTUB_SET_CALLSTART, NULL);

        case 3:
        {
            struct eq_ctlmsg param;
            char *strparam = (char*)parameter;
            char *token = strtok(strparam, ",");
            int32_t hearing_level[6];
            uint16_t *phearing_level = (uint16_t*)hearing_level;
            int i;

            param.reserved = 0;
            if( token == NULL )
                return 0;
            param.dha_mode = atoi(token);

            token = strtok(NULL, ",");
            if( token == NULL )
                return 0;

            param.dha_ch_flag = atoi(token);
            for( i = 0; i < 12; ++i )
            {
                token = strtok(NULL, ",");
                if( token == NULL )
                {
                    return 0;
                }
                *(phearing_level+i) = atoi(token);
            }
            memcpy(param.dha_hearing_level, hearing_level, sizeof(param.dha_hearing_level));
            return VCMAudioIoctl(AUDIOSTUB_EQCTL, &param);
        }


        default:
            ALOGE("Unknown parameter id %d", param_id);
            return EFAULT;
    }
}

int VCMAudioParameterGet( int request, void *param, int32_t *out )
{
    *out = 0;
    return VCMAudioIoctl(request, param);
}

int VCMAudioStreamOutStart(int start, int near_far_end, int comb_with_call, int8_t codec, int32_t * stream)
{
    if( start > 3 )
        return EFAULT;

    if( fdoutaudiostub_pcm >= 0 )
        return 0;

    if( (fdoutaudiostub_pcm = open(AUDIOOUTPCM_DEV, O_WRONLY)) < 0 )
    {
        ALOGE("failed to open %s for write:%s",AUDIOOUTPCM_DEV, strerror(errno));
        return EACCES;
    }

    outBytesWrote = 0;
    memset(&out_playbackctl, 0, sizeof(out_playbackctl));
    out_playbackctl.callback = OUTPUT_STREAM_ID;
    out_playbackctl.comb_with_call = comb_with_call;
    out_playbackctl.near_far_end = near_far_end;
    out_playbackctl.on_off = (start==3 ? 7 : 3);
    out_playbackctl.near_codec_vocoder = (codec&1 ? 2 : 1);
    *stream = 1;

    return 1;
}

int VCMAudioStreamOutStop(int32_t stream_id)
{
    struct pcm_playback_ctlmsg param;
    int res;

    if( stream_id != OUTPUT_STREAM_ID )
        return EFAULT;

    memset(&param, 0, sizeof(param));
    param.callback = OUTPUT_STREAM_ID;
    res = VCMAudioIoctl(AUDIOSTUB_PCMPLAYBACKCTL, &param);
    if( fdoutaudiostub_pcm >= 0 )
    {
        close(fdoutaudiostub_pcm);
        fdoutaudiostub_pcm = -1;
    }
    stream_out_written = 0;

    return res;
}

int VCMAudioStreamWrite(int start, const void* buff, size_t *size)
{
    int res;
    if( start != 1 )
        return EFAULT;

    ssize_t wrote = write(fdoutaudiostub_pcm, buff, *size);
    if( wrote >= 0 )
    {
        *size = wrote;
        outBytesWrote += wrote;
        if( stream_out_written || outBytesWrote < dword_3010 )
        {
            res = 1;
        }
        else
        {
            stream_out_written = 1;
            res = VCMAudioIoctl(AUDIOSTUB_PCMPLAYBACKCTL, &out_playbackctl);
        }
    }
    else
    {
        ALOGE("failed to write stream:%s", strerror(errno));
        res = EFAULT;
    }

    return res;
}

int VCMAudioStreamInStop(int stream_id)
{
    struct pcm_record_ctlmsg param;
    int res;

    if( stream_id != INPUT_STREAM_ID )
        return EFAULT;

    memset(&param, 0, sizeof(param));
    param.callback = INPUT_STREAM_ID;
    res = VCMAudioIoctl(AUDIOSTUB_PCMRECCTL, &param);
    if( fdinaudiostub_pcm >= 0 )
    {
        close(fdinaudiostub_pcm);
        fdinaudiostub_pcm = -1;
    }
    stream_in_read = 0;
    return res;
}

int VCMAudioStreamInStart(int start, int near_far_end, int codec, int32_t * stream)
{
    if( start > 3 )
        return EFAULT;

    if( fdinaudiostub_pcm >= 0 )
        return 0;

    if( (fdinaudiostub_pcm = open(AUDIOINPCM_DEV, O_RDONLY)) < 0 )
    {
        ALOGE("failed to open %s for read:%s", AUDIOINPCM_DEV, strerror(errno));
        return EACCES;
    }

    memset(&in_recordctl, 0, sizeof(in_recordctl));
    in_recordctl.callback = INPUT_STREAM_ID;
    in_recordctl.near_far_end = near_far_end;
    in_recordctl.on_off = (start == 3 ? 7 : 3);
    in_recordctl.near_codec_vocoder = (codec & 1 ? 2 : 1);
    *stream = INPUT_STREAM_ID;

    return 1;
}

int VCMAudioStreamRead(int32_t stream_id, void *buff, size_t* size) 
{
    struct pollfd inpoll;
    const int poll_timeout = 2000;

    if( stream_id != INPUT_STREAM_ID )
        return EFAULT;

    if( !stream_in_read )
    {
        int res = VCMAudioIoctl(AUDIOSTUB_PCMRECCTL, &in_recordctl);
        if( res != 1 )
            return res;

        stream_in_read = 1;
    }

    inpoll.events = POLLIN;
    inpoll.revents = 0;
    inpoll.fd = fdinaudiostub_pcm;

    int res = poll(&inpoll, 1, poll_timeout);
    if( res == 0 )
    {
        ALOGE("polling timeout, CP has no response");
        return 2;
    }
    if( res < 0 )
    {
        ALOGE("poll failed:%s", strerror(errno));
        return 0;
    }
    
    if( inpoll.revents & POLLIN )
    {
        int bytes_read = read(fdinaudiostub_pcm, buff, *size);
        if( bytes_read < 0 )
        {
            ALOGE("failed to read stream:%s", strerror(errno));
            return EACCES;
        }
        *size = bytes_read;
    }
    else
    {
        ALOGE("Warning: driver is not ready for poll");
    }

    return 1;
}


