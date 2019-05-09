#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <sstream>

using namespace std;

#include <dlfcn.h>
#include <tinyalsa/asoundlib.h>

#define LOG_TAG "mytinyalsa"
#include <cutils/log.h>

class tinyalsa_lib
{
    void *_lib;
    public:
        #define LOAD_FUNC(x) *((int*)&x) = (int)dlsym(_lib, #x);
        tinyalsa_lib()
        {
            _lib = dlopen("/system/lib/libtinyalsa_stock.so", RTLD_NOW);
            LOAD_FUNC(mixer_open);
            LOAD_FUNC(mixer_close);
            LOAD_FUNC(mixer_get_num_ctls);
            LOAD_FUNC(mixer_get_ctl);
            LOAD_FUNC(mixer_get_ctl_by_name);
            LOAD_FUNC(mixer_get_name);
            LOAD_FUNC(mixer_ctl_get_name);
            LOAD_FUNC(mixer_ctl_get_type);
            LOAD_FUNC(mixer_ctl_get_value);
            LOAD_FUNC(mixer_ctl_get_array);
            LOAD_FUNC(mixer_ctl_set_value);
            LOAD_FUNC(mixer_ctl_set_array);
            LOAD_FUNC(mixer_ctl_get_type_string);
            LOAD_FUNC(mixer_ctl_get_num_values);
            LOAD_FUNC(mixer_ctl_get_num_enums);
            LOAD_FUNC(mixer_ctl_get_enum_string);
            LOAD_FUNC(mixer_ctl_set_enum_by_string);
            LOAD_FUNC(mixer_ctl_get_range_min);
            LOAD_FUNC(mixer_ctl_get_range_max);
            LOAD_FUNC(pcm_open);
            LOAD_FUNC(pcm_close);
            LOAD_FUNC(pcm_is_ready);
            LOAD_FUNC(pcm_get_htimestamp);
            LOAD_FUNC(pcm_write);
            LOAD_FUNC(pcm_read);
            LOAD_FUNC(pcm_get_error);
            LOAD_FUNC(pcm_params_get);
            LOAD_FUNC(pcm_params_free);
            LOAD_FUNC(pcm_params_get_mask);
            LOAD_FUNC(pcm_params_get_min);
            LOAD_FUNC(pcm_params_set_min);
            LOAD_FUNC(pcm_params_get_max);
            LOAD_FUNC(pcm_params_set_max);
            LOAD_FUNC(pcm_params_to_string);
            LOAD_FUNC(pcm_params_format_test);
            LOAD_FUNC(pcm_get_config);
            LOAD_FUNC(pcm_set_config);
            LOAD_FUNC(pcm_format_to_bits);
            LOAD_FUNC(pcm_get_buffer_size);
            LOAD_FUNC(pcm_frames_to_bytes);
            LOAD_FUNC(pcm_bytes_to_frames);
            LOAD_FUNC(pcm_get_latency);
            LOAD_FUNC(pcm_mmap_write);
            LOAD_FUNC(pcm_mmap_read);
            LOAD_FUNC(pcm_mmap_begin);
            LOAD_FUNC(pcm_mmap_commit);
            LOAD_FUNC(pcm_mmap_avail);
            LOAD_FUNC(pcm_prepare);
            LOAD_FUNC(pcm_start);
            LOAD_FUNC(pcm_stop);
            LOAD_FUNC(pcm_ioctl);
            LOAD_FUNC(pcm_wait);
            LOAD_FUNC(pcm_get_poll_fd);
            LOAD_FUNC(pcm_set_avail_min);
        }
        struct mixer*       (*mixer_open               )(unsigned int card);
        void                (*mixer_close              )(struct mixer *mixer);
        const char*         (*mixer_get_name           )(struct mixer *mixer);
        unsigned int        (*mixer_get_num_ctls       )(struct mixer *mixer);
        struct mixer_ctl*   (*mixer_get_ctl            )(struct mixer *mixer, unsigned int id);
        struct mixer_ctl*   (*mixer_get_ctl_by_name    )(struct mixer *mixer, const char *name);

        const char*         (*mixer_ctl_get_name       )(struct mixer_ctl *ctl);
        enum mixer_ctl_type (*mixer_ctl_get_type       )(struct mixer_ctl *ctl);
        int                 (*mixer_ctl_get_value      )(struct mixer_ctl *ctl, unsigned int id);
        int                 (*mixer_ctl_get_array      )(struct mixer_ctl *ctl, void *array, size_t count);
        int                 (*mixer_ctl_set_value      )(struct mixer_ctl *ctl, unsigned int id, int value);
        int                 (*mixer_ctl_set_array      )(struct mixer_ctl *ctl, const void *array, size_t count);
        const char*         (*mixer_ctl_get_type_string)(struct mixer_ctl *ctl);
        unsigned int        (*mixer_ctl_get_num_values )(struct mixer_ctl *ctl);
        unsigned int        (*mixer_ctl_get_num_enums  )(struct mixer_ctl *ctl);
        const char*         (*mixer_ctl_get_enum_string)(struct mixer_ctl *ctl, unsigned int enum_id);
        int                 (*mixer_ctl_set_enum_by_string)(struct mixer_ctl *ctl, const char *string);
        int                 (*mixer_ctl_get_range_min  )(struct mixer_ctl *ctl);
        int                 (*mixer_ctl_get_range_max  )(struct mixer_ctl *ctl);

        struct pcm*        (*pcm_open              )(unsigned int card, unsigned int device, unsigned int flags, struct pcm_config *config);
        int                (*pcm_close             )(struct pcm *pcm);
        int                (*pcm_is_ready          )(struct pcm *pcm);
        struct pcm_params* (*pcm_params_get        )(unsigned int card, unsigned int device, unsigned int flags);
        void               (*pcm_params_free       )(struct pcm_params *pcm_params);
        struct pcm_mask*   (*pcm_params_get_mask   )(struct pcm_params *pcm_params, enum pcm_param param);
        unsigned int       (*pcm_params_get_min    )(struct pcm_params *pcm_params, enum pcm_param param);
        void               (*pcm_params_set_min    )(struct pcm_params *pcm_params, enum pcm_param param, unsigned int val);
        unsigned int       (*pcm_params_get_max    )(struct pcm_params *pcm_params, enum pcm_param param);
        void               (*pcm_params_set_max    )(struct pcm_params *pcm_params, enum pcm_param param, unsigned int val);
        int                (*pcm_params_to_string  )(struct pcm_params *params, char *string, unsigned int size);
        int                (*pcm_params_format_test)(struct pcm_params *params, enum pcm_format format);
        int                (*pcm_get_config        )(struct pcm *pcm, struct pcm_config *config);
        int                (*pcm_set_config        )(struct pcm *pcm, struct pcm_config *config);
        const char*        (*pcm_get_error         )(struct pcm *pcm);
        unsigned int       (*pcm_format_to_bits    )(enum pcm_format format);
        unsigned int       (*pcm_get_buffer_size   )(struct pcm *pcm);
        unsigned int       (*pcm_frames_to_bytes   )(struct pcm *pcm, unsigned int frames);
        unsigned int       (*pcm_bytes_to_frames   )(struct pcm *pcm, unsigned int bytes);
        unsigned int       (*pcm_get_latency       )(struct pcm *pcm);
        int                (*pcm_get_htimestamp    )(struct pcm *pcm, unsigned int *avail, struct timespec *tstamp);
        int                (*pcm_write             )(struct pcm *pcm, const void *data, unsigned int count);
        int                (*pcm_read              )(struct pcm *pcm, void *data, unsigned int count);
        int (*pcm_mmap_write   )(struct pcm *pcm, const void *data, unsigned int count);
        int (*pcm_mmap_read    )(struct pcm *pcm, void *data, unsigned int count);
        int (*pcm_mmap_begin   )(struct pcm *pcm, void **areas, unsigned int *offset, unsigned int *frames);
        int (*pcm_mmap_commit  )(struct pcm *pcm, unsigned int offset, unsigned int frames);
        int (*pcm_mmap_avail   )(struct pcm *pcm);
        int (*pcm_prepare      )(struct pcm *pcm);
        int (*pcm_start        )(struct pcm *pcm);
        int (*pcm_stop         )(struct pcm *pcm);
        int (*pcm_ioctl        )(struct pcm *pcm, int request, ...);
        int (*pcm_wait         )(struct pcm *pcm, int timeout);
        int (*pcm_get_poll_fd  )(struct pcm *pcm);
        int (*pcm_set_avail_min)(struct pcm *pcm, int avail_min);
};

static tinyalsa_lib lib;

void log(std::string const&msg)
{
    ofstream ff("/system/logs/tinyalsa.log2", ios::out|ios::app);
    ff << msg.c_str();
}

extern "C"
{
struct mixer *mixer_open(unsigned int card)
{
    stringstream logger;
    struct mixer *mixer = nullptr;
    logger << "Oppening card " << card;
    mixer = lib.mixer_open(card);
    logger << " (" << mixer << ')' << endl;

    log(logger.str());

    return mixer;
}

void mixer_close(struct mixer *mixer)
{
    stringstream logger;
    logger << "Closing card " << mixer;
    lib.mixer_close(mixer);
    log(logger.str());
}

struct mixer_ctl *mixer_get_ctl(struct mixer *mixer, unsigned int id)
{
    stringstream logger;
    mixer_ctl *mctl = lib.mixer_get_ctl(mixer, id);
    logger << "Openning mixer control (" << mixer << ") " << lib.mixer_ctl_get_name(mctl) << endl;
    log(logger.str());
    return mctl;
}

enum mixer_ctl_type mixer_ctl_get_type(struct mixer_ctl *ctl)
{
    return lib.mixer_ctl_get_type(ctl);
}

int mixer_ctl_get_value(struct mixer_ctl *ctl, unsigned int id)
{
    stringstream logger;
    int res = lib.mixer_ctl_get_value(ctl, id);
    logger << "Control get value " << lib.mixer_ctl_get_name(ctl) << " at index " << id << " = " << res << endl;
    log(logger.str());
    return res;
}

int mixer_ctl_get_array(struct mixer_ctl *ctl, void *array, size_t count)
{
    stringstream logger;
    int res = lib.mixer_ctl_get_array(ctl, array, count);

    logger << "Get control array " << lib.mixer_ctl_get_name(ctl) << ": ";
    switch(lib.mixer_ctl_get_type(ctl))
    {
        case MIXER_CTL_TYPE_BOOL:
            for( int i = 0; i < count; ++i )
            {
                logger << ((bool*)array)[i] << " ";
            }
            break;
        case MIXER_CTL_TYPE_INT:
            for( int i = 0; i < count; ++i )
            {
                logger << ((unsigned int*)array)[i] << " ";
            }
            break;
        case MIXER_CTL_TYPE_ENUM:
            for( int i = 0; i < count; ++i )
            {
                logger << ((int*)array)[i] << "(" << lib.mixer_ctl_get_enum_string(ctl, i) << ") ";
            }
            break;
        case MIXER_CTL_TYPE_BYTE:
            for( int i = 0; i < count; ++i )
            {
                logger << (int)((uint8_t*)array)[i] << " ";
            }
            break;
        case MIXER_CTL_TYPE_INT64:
            for( int i = 0; i < count; ++i )
            {
                logger << ((int64_t*)array)[i] << " ";
            }
            break;
    }
    logger << endl;
    log(logger.str());
    return res;
}

int mixer_ctl_set_value(struct mixer_ctl *ctl, unsigned int id, int value)
{
    stringstream logger;
    logger << "Setting control " << lib.mixer_ctl_get_name(ctl) << " id " << id << ": ";
    switch(lib.mixer_ctl_get_type(ctl))
    {
        case MIXER_CTL_TYPE_ENUM:
                logger << value << "(" << lib.mixer_ctl_get_enum_string(ctl, value) << ")" << endl;
                break;
        default:
                logger << value << endl;
    }

    int res = lib.mixer_ctl_set_value(ctl, id, value);

    log(logger.str());
    return res;
}

int mixer_ctl_set_array(struct mixer_ctl *ctl, const void *array, size_t count)
{
    stringstream logger;
    logger << "Setting control array" << lib.mixer_ctl_get_name(ctl) << ": ";
    switch(lib.mixer_ctl_get_type(ctl))
    {
        case MIXER_CTL_TYPE_ENUM:
                for( int i = 0; i < count; ++i )
                {
                    logger << ((int*)array)[i] << "(" << lib.mixer_ctl_get_enum_string(ctl, ((int*)array)[i]) << ")" << " ";
                }
                break;
        case MIXER_CTL_TYPE_INT64:
                for( int i = 0; i < count; ++i )
                {
                    logger << ((int64_t*)array)[i] << " ";
                }
                break;
        case MIXER_CTL_TYPE_BOOL:
                for( int i = 0; i < count; ++i )
                {
                    logger << ((bool*)array)[i] << " ";
                }
                break;
        case MIXER_CTL_TYPE_BYTE:
                for( int i = 0; i < count; ++i )
                {
                    logger << (unsigned int)((uint8_t*)array)[i] << " ";
                }
                break;
        default:
                for( int i = 0; i < count; ++i )
                {
                    logger << ((int*)array)[i] << " ";
                }
                break;
    }
    logger << endl;

    int res = lib.mixer_ctl_set_array(ctl, array, count);
    log(logger.str());
    return res;
}

const char*         mixer_get_name           (struct mixer *mixer){return lib.mixer_get_name(mixer);}
unsigned int        mixer_get_num_ctls       (struct mixer *mixer){return lib.mixer_get_num_ctls(mixer);}
struct mixer_ctl*   mixer_get_ctl_by_name    (struct mixer *mixer, const char *name){return lib.mixer_get_ctl_by_name(mixer, name);}
const char*         mixer_ctl_get_name       (struct mixer_ctl *ctl){return lib.mixer_ctl_get_name(ctl);}
const char*         mixer_ctl_get_type_string(struct mixer_ctl *ctl){return lib.mixer_ctl_get_type_string(ctl);}
unsigned int        mixer_ctl_get_num_values (struct mixer_ctl *ctl){return lib.mixer_ctl_get_num_values(ctl);}
unsigned int        mixer_ctl_get_num_enums  (struct mixer_ctl *ctl){return lib.mixer_ctl_get_num_enums(ctl);}
const char*         mixer_ctl_get_enum_string(struct mixer_ctl *ctl, unsigned int enum_id){return lib.mixer_ctl_get_enum_string(ctl, enum_id);}
int                 mixer_ctl_set_enum_by_string(struct mixer_ctl *ctl, const char *string){return lib.mixer_ctl_set_enum_by_string(ctl, string);}
int                 mixer_ctl_get_range_min  (struct mixer_ctl *ctl){return lib.mixer_ctl_get_range_min(ctl);}
int                 mixer_ctl_get_range_max  (struct mixer_ctl *ctl){return lib.mixer_ctl_get_range_max(ctl);}

struct pcm*        pcm_open              (unsigned int card, unsigned int device, unsigned int flags, struct pcm_config *config){return lib.pcm_open(card, device, flags, config);}
int                pcm_close             (struct pcm *pcm){return lib.pcm_close(pcm);}
int                pcm_is_ready          (struct pcm *pcm){return lib.pcm_is_ready(pcm);}
struct pcm_params* pcm_params_get        (unsigned int card, unsigned int device, unsigned int flags){return lib.pcm_params_get(card, device, flags);}
void               pcm_params_free       (struct pcm_params *pcm_params){return lib.pcm_params_free(pcm_params);}
struct pcm_mask*   pcm_params_get_mask   (struct pcm_params *pcm_params, enum pcm_param param){return lib.pcm_params_get_mask(pcm_params, param);}
unsigned int       pcm_params_get_min    (struct pcm_params *pcm_params, enum pcm_param param){return lib.pcm_params_get_min(pcm_params, param);}
void               pcm_params_set_min    (struct pcm_params *pcm_params, enum pcm_param param, unsigned int val){return lib.pcm_params_set_min(pcm_params, param, val);}
unsigned int       pcm_params_get_max    (struct pcm_params *pcm_params, enum pcm_param param){return lib.pcm_params_get_max(pcm_params, param);}
void               pcm_params_set_max    (struct pcm_params *pcm_params, enum pcm_param param, unsigned int val){return lib.pcm_params_set_max(pcm_params, param,  val);}
int                pcm_params_to_string  (struct pcm_params *params, char *string, unsigned int size){return lib.pcm_params_to_string(params, string, size);}
int                pcm_params_format_test(struct pcm_params *params, enum pcm_format format){return lib.pcm_params_format_test(params, format);}
int                pcm_get_config        (struct pcm *pcm, struct pcm_config *config){return lib.pcm_get_config(pcm, config);}
int                pcm_set_config        (struct pcm *pcm, struct pcm_config *config){return lib.pcm_set_config(pcm, config);}
const char*        pcm_get_error         (struct pcm *pcm){return lib.pcm_get_error(pcm);}
unsigned int       pcm_format_to_bits    (enum pcm_format format){return lib.pcm_format_to_bits(format);}
unsigned int       pcm_get_buffer_size   (struct pcm *pcm){return lib.pcm_get_buffer_size(pcm);}
unsigned int       pcm_frames_to_bytes   (struct pcm *pcm, unsigned int frames){return lib.pcm_frames_to_bytes(pcm, frames);}
unsigned int       pcm_bytes_to_frames   (struct pcm *pcm, unsigned int bytes){return lib.pcm_bytes_to_frames(pcm, bytes);}
unsigned int       pcm_get_latency       (struct pcm *pcm){return lib.pcm_get_latency(pcm);}
int                pcm_get_htimestamp    (struct pcm *pcm, unsigned int *avail, struct timespec *tstamp){return lib.pcm_get_htimestamp(pcm, avail, tstamp);}
int                pcm_write             (struct pcm *pcm, const void *data, unsigned int count){return lib.pcm_write(pcm, data, count);}
int                pcm_read              (struct pcm *pcm, void *data, unsigned int count){return lib.pcm_read(pcm, data, count);}
int pcm_mmap_write   (struct pcm *pcm, const void *data, unsigned int count){return lib.pcm_mmap_write(pcm, data, count);}
int pcm_mmap_read    (struct pcm *pcm, void *data, unsigned int count){return lib.pcm_mmap_read(pcm, data, count);}
int pcm_mmap_begin   (struct pcm *pcm, void **areas, unsigned int *offset, unsigned int *frames){return lib.pcm_mmap_begin(pcm, areas, offset, frames);}
int pcm_mmap_commit  (struct pcm *pcm, unsigned int offset, unsigned int frames){return lib.pcm_mmap_commit(pcm, offset, frames);}
int pcm_mmap_avail   (struct pcm *pcm){return lib.pcm_mmap_avail(pcm);}
int pcm_prepare      (struct pcm *pcm){return lib.pcm_prepare(pcm);}
int pcm_start        (struct pcm *pcm){return lib.pcm_start(pcm);}
int pcm_stop         (struct pcm *pcm){return lib.pcm_stop(pcm);}
//int pcm_ioctl        (struct pcm *pcm, int request, ...){return lib.pcm_ioctl(pcm, request, );}
int pcm_wait         (struct pcm *pcm, int timeout){return lib.pcm_wait(pcm, timeout);}
int pcm_get_poll_fd  (struct pcm *pcm){return lib.pcm_get_poll_fd(pcm);}
int pcm_set_avail_min(struct pcm *pcm, int avail_min){return lib.pcm_set_avail_min(pcm, avail_min);}
}

__attribute__((constructor)) void __so_load__()
{
    log("Loaded library");
}
