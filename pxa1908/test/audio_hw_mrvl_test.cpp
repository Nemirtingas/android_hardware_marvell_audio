#include <iostream>
#include <map>
#include <fstream>

using namespace std;

#include <stdlib.h>
#include <dlfcn.h>
#include <cutils/log.h>
#include <cutils/str_parms.h>
#include <cutils/properties.h>
#include <tinyalsa/asoundlib.h>
#include <audio_route/audio_route.h>

void print_mixer_ctl_value(mixer_ctl *mctl)
{
    switch(mixer_ctl_get_type(mctl))
    {
        case MIXER_CTL_TYPE_BOOL   :
            for( int i = 0; i < mixer_ctl_get_num_values(mctl); ++i )
            {
                cout << (mixer_ctl_get_value(mctl,i)?"true":"false") << " ";
            }
            break;
        case MIXER_CTL_TYPE_INT    :
        case MIXER_CTL_TYPE_BYTE   :
        case MIXER_CTL_TYPE_INT64  : 
            for( int i = 0; i < mixer_ctl_get_num_values(mctl); ++i )
            {
                cout << mixer_ctl_get_value(mctl,i) << " ";
            }
            break;
        case MIXER_CTL_TYPE_ENUM   :
                cout << mixer_ctl_get_enum_string(mctl, mixer_ctl_get_value(mctl,0)) << " ";
            break;
        default: cout << "Unknown type, can't read value";
    }
    cout << endl;
}

void set_value(mixer *mixer, const char *param, int *values, int size)
{
    mixer_ctl *mctl = mixer_get_ctl_by_name(mixer, param);
    int ret;

    cout << mixer_ctl_get_type_string(mctl) << endl;
    cout << param << ": old value = ";
    print_mixer_ctl_value(mctl);
    if( mixer_ctl_get_num_values(mctl) != 1 )
    {
        ret = mixer_ctl_set_array(mctl, values, size);
    }
    else
    {
        switch(mixer_ctl_get_type(mctl))
        {
            case MIXER_CTL_TYPE_BOOL   :
                ret = mixer_ctl_set_value(mctl, 0, (values[0]?1:0));
                break;
            case MIXER_CTL_TYPE_ENUM   :
                ret = mixer_ctl_set_enum_by_string(mctl, (char*)values);
                break;
            default:
                ret = mixer_ctl_set_value(mctl, 0, values[0]);
        }
    }
    cout << param << ": new value = ";
    print_mixer_ctl_value(mctl);

    cout << "ret = " << ret << endl;
}

int main(int argc, char *argv[])
{
    char value[PROPERTY_VALUE_MAX];
    int retry_num = 0, snd_card_num = 0;
    const char *snd_card_name;
    struct mixer *mixer;
    mixer_ctl *mctl;
    int vals[4];

    mixer = mixer_open(0);

    set_value(mixer, "MAP_TOP_CTRL_REG_1", (int*)"\x03\x00\x00\x00", 4);
    set_value(mixer, "MAP_DSP1_DAC_VOLUME", (int*)"\x01\x00\x00\x00", 4);
    set_value(mixer, "MAP_DSP2_DAC_VOLUME", (int*)"\x01\x00\x00\x00", 4);
    set_value(mixer, "MAP_ADC_VOLUME", (int*)"\x01\x00\x00\x00", 4);
    set_value(mixer, "MAP_DSP1_INMIX_COEF_REG", (int*)"\x00\x00\x00\x00", 4);
    set_value(mixer, "MAP_DSP2_INMIX_COEF_REG", (int*)"\x00\x00\x00\x00", 4);
    set_value(mixer, "MAP_INPUT_MIX_REG", (int*)"\x00\x00\x00\x00", 4);
    set_value(mixer, "MAP_DATAPATH_FLOW_CTRL_REG_1", (int*)"\x00\x00\x3f\x00", 4);
    set_value(mixer, "d1in4_mix_enable Switch", (int*)"\x01\x00\x00\x00", 1);
    set_value(mixer, "MAP_DSP1_DAC_PROCESSING_REG", (int*)"\x00\x10\x00\x00", 4);
    set_value(mixer, "MAP_DSP1_DAC_PROCESSING_REG", (int*)"\x00\x11\x00\x00", 4);
    set_value(mixer, "dsp1_enable Switch", (int*)"\x03\x00\x00\x00", 1);
    set_value(mixer, "MAP_DSP1_DAC_CTRL_REG", (int*)"\x03\x00\x00\x00", 4);
    set_value(mixer, "DAC1 output out1", (int*)"D1OUT", 1);
    set_value(mixer, "MAP_DATAPATH_FLOW_CTRL_REG_1", (int*)"\x00\x00\x07\x00", 4);
    set_value(mixer, "out1_spkr_en Switch", (int*)"\x01\x00\x00\x00", 1);

    set_value(mixer, "PM860_MAIN_POWER_REG", (int*)"\x01\x00\x00\x00", 1);
    //set_value(mixer, "PM860_SEQ_STATUS_REG1", (int*)"\xa9\x00\x00\x00", 1);
    set_value(mixer, "PM860_DIG_BLOCK_EN_REG1", (int*)"\x04\x00\x00\x00", 1);
    set_value(mixer, "PM860_PDM_SETTINGS3", (int*)"\x31\x00\x00\x00", 1);
    set_value(mixer, "PM860_PDM_SETTINGS2", (int*)"\x02\x00\x00\x00", 1);
    set_value(mixer, "PM860_PDM_SETTINGS1", (int*)"\xd1\x00\x00\x00", 1);
    set_value(mixer, "PM822_CLASS_D_1", (int*)"\x06\x00\x00\x00", 1);
    set_value(mixer, "PM822_MIS_CLASS_D_2", (int*)"\x70\x00\x00\x00", 1);
    set_value(mixer, "PM860_VOL_SEL_3", (int*)"\xbd\x00\x00\x00", 1);

    set_value(mixer, "MAP_DSP1_DAC_PROCESSING_REG", (int*)"\x00\x11\x00\x00", 4);
    set_value(mixer, "MAP_DSP1_INMIX_COEF_REG", (int*)"\x00\x00\x00\x80", 4);
    set_value(mixer, "MAP_DSP1_DAC_VOLUME", (int*)"\x01\xff\xff\x00", 4);
    set_value(mixer, "MAP_DSP1_DAC_PROCESSING_REG", (int*)"\x10\x11\x00\x00", 4);

    mixer_close(mixer);

    return 0;
}
