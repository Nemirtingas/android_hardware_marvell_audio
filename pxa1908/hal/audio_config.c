/*
 * Copyright (C)  2005. Marvell International Ltd
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

#define LOG_TAG "audio_hw_config"
#define LOG_NDEBUG 0

#include <stdio.h>
#include <hardware/hardware.h>
#include <hardware/audio.h>
#include <system/audio.h>
#include <cutils/log.h>
#include <libxml/tree.h>

#include "acm_api.h"
#include "audio_path.h"

#define AUDIO_PLATFORM_CONFIG_FILE "/etc/platform_audio_config.xml"
#define DEVICE_NAME_LEN_MAX 32

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

struct app_cfg_t {
  virtual_mode_t v_mode;
  unsigned int device;  // support multi configurations
  struct app_cfg_t *next;
};

struct android_dev_cfg_t {
  unsigned int android_dev;
  struct app_cfg_t *app_cfg;
  struct android_dev_cfg_t *next;
};

struct board_dev_cfg_t {
  unsigned int hw_dev;
  unsigned int connectivity;  // connectivity type
  unsigned int coupling;      // coupling type
  struct board_dev_cfg_t *next;
};

struct device_name_t {
  char name[DEVICE_NAME_LEN_MAX];
  int len;
};

struct platform_config_t {
  struct board_dev_cfg_t *board_dev_cfg;    // support board device list
  struct android_dev_cfg_t *droid_dev_cfg;  // support android device list

  struct board_dev_cfg_t *current_board_device;
  struct device_name_t current_device_name;
  struct android_dev_cfg_t *current_droid_device;
  struct app_cfg_t *current_app_cfg;
};

typedef enum {
  PARSING_OK = 0,
  PARSING_UNKNOWN_ERROR,
  PARSING_ERROR_IO,
  PARSING_ERROR_MALFORMED,
} parsing_check_t;

typedef enum {
  SECTION_UNKNOWN = -1,
  SECTION_TOP_LEVEL,
  SECTION_BOARD_DEVICE_LIST,
  SECTION_ANDROID_DEVICE,
  SECTION_APPLICATION,
  SECTION_DEVICE_IN_BOARD,
  SECTION_DEVICE_IN_ANDROID,
} section_layout_t;

static struct platform_config_t *mrvl_platform_cfg = NULL;
static section_layout_t current_section = SECTION_UNKNOWN;

// get mic device from platform_audio_config.xml config
unsigned int get_mic_dev(virtual_mode_t v_mode, unsigned int android_dev)
{
  struct android_dev_cfg_t *droiddev_cfg = mrvl_platform_cfg->droid_dev_cfg;
  unsigned int default_dev = HWDEV_INVALID;

  while (droiddev_cfg)
  {
    if (droiddev_cfg->android_dev == android_dev)
    {
      struct app_cfg_t *app_cfg = droiddev_cfg->app_cfg;
      while (app_cfg)
      {
        if (app_cfg->v_mode == v_mode)
        {
          ALOGD("%s: find matched dev 0x%x", __FUNCTION__, app_cfg->device);
          return app_cfg->device;
        }

        if (app_cfg->v_mode == V_MODE_DEF)
        {
          default_dev = app_cfg->device;
        }
        app_cfg = app_cfg->next;
      }

      // if none matched app found, use default device
      ALOGD("%s: cannot find matched app, use default dev 0x%x", __FUNCTION__,
            default_dev);
      return default_dev;
    }

    droiddev_cfg = droiddev_cfg->next;
  }
  // return default device
  return HWDEV_AMIC1;
}

// get mic hw flag for connectivity and coupling
unsigned int get_mic_hw_flag(unsigned int hw_dev)
{
  unsigned int flags = 0;
  struct board_dev_cfg_t *dev_cfg = mrvl_platform_cfg->board_dev_cfg;

  // for TTY, use the equivalent device
  switch (hw_dev)
  {
    case HWDEV_IN_TTY:
      hw_dev = HWDEV_HSMIC;
      break;
    case HWDEV_IN_TTY_VCO_DUAL_AMIC:
      hw_dev = HWDEV_DUAL_AMIC;
      break;
    case HWDEV_IN_TTY_VCO_DUAL_AMIC_SPK_MODE:
      hw_dev = HWDEV_DUAL_AMIC_SPK_MODE;
      break;
    case HWDEV_IN_TTY_VCO_DUAL_DMIC1:
      hw_dev = HWDEV_DUAL_DMIC1;
      break;
    case HWDEV_IN_TTY_VCO_AMIC1:
      hw_dev = HWDEV_AMIC1;
      break;
    case HWDEV_IN_TTY_VCO_AMIC2:
      hw_dev = HWDEV_AMIC2;
      break;
    default:
      // keep the current hw device.
      break;
  }

  while (dev_cfg) {
    if (dev_cfg->hw_dev == hw_dev) {
      flags = (dev_cfg->coupling | dev_cfg->connectivity);
    }
    dev_cfg = dev_cfg->next;
  }
  return flags;
}

static void get_android_dev_by_user_selection(char *dev_name)
{
  uint32_t mic_mode = get_mic_mode();

  ALOGD("%s mic_mode= %d", __FUNCTION__, mic_mode);

  switch (mic_mode) {
    case MIC_MODE_MIC1:
    case MIC_MODE_MIC2:
    case MIC_MODE_DUALMIC:
      if (strstr(dev_name, "_SPK_MODE")) {
        strcpy(dev_name, mic_mode_to_dev_name[mic_mode]);
        strcat(dev_name, "_SPK_MODE");
      } else
        strcpy(dev_name, mic_mode_to_dev_name[mic_mode]);
      break;

    case MIC_MODE_NONE:
    default:
      break;
  }
}

static unsigned int get_android_dev_byname(char *dev_name)
{
  if (!strcmp(dev_name, "AUDIO_DEVICE_IN_BUILTIN_MIC"))
    return AUDIO_DEVICE_IN_BUILTIN_MIC;

  if (!strcmp(dev_name, "AUDIO_DEVICE_IN_BACK_MIC"))
    return AUDIO_DEVICE_IN_BACK_MIC;

  return 0;
}

static virtual_mode_t get_mode_byname(char *app_name)
{
  int i = 0;

  for (i = 0; i < (int)(sizeof(vtrl_mode_name) / sizeof(char *)); i++)
  {
    if (!strcmp(vtrl_mode_name[i], app_name))
    {
      return i;
    }
  }
  return V_MODE_INVALID;
}

static unsigned int get_hwdev_byname(char *dev_name)
{
  int i = 0;

  for (i = 0; i < (int)(sizeof(input_devname) / sizeof(char *)); i++)
  {
    if (!strcmp(input_devname[i], dev_name))
    {
      return (HWDEV_BIT_IN | (HWDEV_IN_BASE << i));
    }
  }
  return HWDEV_INVALID;
}

static void parse_board_devlist(xmlNodePtr node, struct platform_config_t *config)
{
  struct board_dev_cfg_t *dev_cfg;
  xmlChar* connectivity, *coupling, *content;

  for( ; node; node = node->next )
  {
    if( !xmlStrcmp(node->name, (xmlChar*)"Device") )
    {
      dev_cfg = (struct board_dev_cfg_t*)calloc(1, sizeof(struct board_dev_cfg_t));
      if( !dev_cfg )
      {
        ALOGE("%s/L%d: out of memory", __FUNCTION__, __LINE__);
        break;
      }
      memset(dev_cfg, 0, sizeof(struct board_dev_cfg_t));
      content = xmlNodeGetContent(node);
      if( content )
      {
        connectivity = xmlGetProp(node, (xmlChar*)"connectivity");
        coupling = xmlGetProp(node, (xmlChar*)"coupling");
        ALOGI("%s: find board config device %s", __FUNCTION__, content);
        dev_cfg->hw_dev = get_hwdev_byname((char*)content);
        xmlFree(content);
        if( connectivity )
        {
          ALOGI("%s: connectivity = \"%s\"", __FUNCTION__, connectivity);
          if( !strcmp((char*)connectivity, "diff") )
          {
            dev_cfg->connectivity = 0x20000;
          }
          else if( !strcmp((char*)connectivity, "quasi_diff") )
          {
            dev_cfg->connectivity = 0x40000;
          }
          else if( !strcmp((char*)connectivity, "single_ended") )
          {
            dev_cfg->connectivity = 0x80000;
          }
          else
          {
            dev_cfg->connectivity = 0;
          }
          xmlFree(connectivity);
        }
        if( coupling )
        {
          ALOGI("%s: coupling = \"%s\"", __FUNCTION__, coupling);
          if( !strcmp((char*)coupling, "ac") )
          {
            dev_cfg->coupling = 0x100000;
          }
          else if( !strcmp((char*)coupling, "dc") )
          {
            dev_cfg->coupling = 0x200000;
          }
          else
          {
            dev_cfg->coupling = 0;
          }
          xmlFree(coupling);
        }
      }
      if( config->board_dev_cfg )
      {
        struct board_dev_cfg_t *tmp_cfg = config->board_dev_cfg;
        while( tmp_cfg->next )
          tmp_cfg = tmp_cfg->next;
        tmp_cfg->next = dev_cfg;
      }
      else
      {
        config->board_dev_cfg = dev_cfg;
      }
    }
  }
}

static int parse_app_config(xmlNodePtr node, struct android_dev_cfg_t *config)
{
  struct app_cfg_t *app_cfg;
  xmlNodePtr app_node;
  xmlChar *xml_prop;
  xmlChar *value;

  app_cfg = (struct app_cfg_t*)calloc(1, sizeof(struct app_cfg_t));
  if( !app_cfg )
  {
    ALOGE("%s: find app config, identifier %s", __FUNCTION__, xml_prop);
    return -1;
  }

  xml_prop = xmlGetProp(node, (xmlChar*)"identifier");
  if( xml_prop )
  {
    ALOGI("%s: find app config, identifier %s", __FUNCTION__, xml_prop);
    app_cfg->v_mode = get_mode_byname((char*)xml_prop);
    xmlFree(xml_prop);
  }

  for( app_node = node->children; app_node; app_node = app_node->next )
  {
    if( !xmlStrcmp(app_node->name, (xmlChar*)"Device") )
    {
      value = xmlNodeGetContent(app_node);
      if( value )
      {
        ALOGI("%s: find device %s", __FUNCTION__, value);
        app_cfg->device |= get_hwdev_byname((char*)value);
        xmlFree(value);
      }
    }
  }

  if( config->app_cfg )
  {
    struct app_cfg_t* tmp_cfg = config->app_cfg;
    while( tmp_cfg->next )
        tmp_cfg = tmp_cfg->next;
    tmp_cfg->next = app_cfg;
  }
  else
  {
    config->app_cfg = app_cfg;
  }

  return 0;
}

int init_platform_config()
{
  const char *config_file = AUDIO_PLATFORM_CONFIG_FILE;
  xmlDocPtr xml_doc;
  xmlNodePtr root_elem;
  xmlNodePtr node;

  ALOGI("%s: config file %s", __FUNCTION__, config_file);

  xml_doc = xmlParseFile(config_file);
  if( !xml_doc )
  {
    ALOGE("%s: failed to parse xml file %s: %s", __FUNCTION__, config_file, strerror(errno));
    return -1;
  }

  root_elem = xmlDocGetRootElement(xml_doc);
  if( !root_elem )
  {
    ALOGE("%s: failed to get root element in %s", __FUNCTION__, config_file);
    xmlFreeDoc(xml_doc);
    return -1;
  }

  if( xmlStrcmp(root_elem->name, (xmlChar*)"MarvellPlatformAudioConfiguration") )
  {
    ALOGE("%s: wrong type document, root node != MarvellPlatformAudioConfiguration", __FUNCTION__);
    xmlFreeDoc(xml_doc);
    return -1;
  }

  mrvl_platform_cfg = (struct platform_config_t*)calloc(1, sizeof(struct platform_config_t));
  if( !mrvl_platform_cfg )
  {
    ALOGE("%s/L%d: out of memory", __FUNCTION__, __LINE__);
    xmlFreeDoc(xml_doc);
    return -1;
  }

  for( node = root_elem->children; node; node = node->next )
  {
    if( !xmlStrcmp(node->name, (xmlChar*)"BoardDeviceList") )
    {
      parse_board_devlist(node->children, mrvl_platform_cfg);
    }
    else if( !xmlStrcmp(node->name, (xmlChar*)"AndroidDevice") )
    {
      xmlNodePtr app_node;
      xmlChar* xml_prop;
      struct android_dev_cfg_t *droid_dev_cfg = (struct android_dev_cfg_t*)calloc(1, sizeof(struct android_dev_cfg_t));

      if( !droid_dev_cfg )
      {
        ALOGE("%s/L%d: out of memory", __FUNCTION__, __LINE__);
        free(mrvl_platform_cfg);
        mrvl_platform_cfg = NULL;
        xmlFreeDoc(xml_doc);
        return -1;
      }

      xml_prop = xmlGetProp(node, (xmlChar*)"identifier");
      if( xml_prop )
      {
        droid_dev_cfg->android_dev = get_android_dev_byname((char*)xml_prop);
        ALOGI("%s: find android dev identifier %s", __FUNCTION__, xml_prop);
        xmlFree(xml_prop);
      }
      for( app_node = node->children; app_node; app_node = app_node->next )
      {
        if( !xmlStrcmp(app_node->name, (xmlChar*)"Application") )
        {
          parse_app_config(app_node, droid_dev_cfg);
        }
      }
      if( mrvl_platform_cfg->droid_dev_cfg )
      {
        struct android_dev_cfg_t *tmp_dev_cfg = mrvl_platform_cfg->droid_dev_cfg;
        while( tmp_dev_cfg->next )
          tmp_dev_cfg = tmp_dev_cfg->next;
        tmp_dev_cfg->next = droid_dev_cfg;
      }
      else
      {
          mrvl_platform_cfg->droid_dev_cfg = droid_dev_cfg;
      }
    }
  }

  xmlFreeDoc(xml_doc);
  return 0;
}

// free global platform configuration
void deinit_platform_config()
{
  struct android_dev_cfg_t *tmp_droiddev_cfg = NULL;
  struct board_dev_cfg_t *tmp_board_dev_cfg = NULL;

  if (mrvl_platform_cfg == NULL) {
    ALOGE("%s: mrvl_platform_cfg is not initialized.", __FUNCTION__);
    return;
  }

  tmp_board_dev_cfg = mrvl_platform_cfg->board_dev_cfg;
  while (tmp_board_dev_cfg) {
    mrvl_platform_cfg->board_dev_cfg = mrvl_platform_cfg->board_dev_cfg->next;
    free(tmp_board_dev_cfg);
    tmp_board_dev_cfg = mrvl_platform_cfg->board_dev_cfg;
  }

  tmp_droiddev_cfg = mrvl_platform_cfg->droid_dev_cfg;
  while (tmp_droiddev_cfg) {
    struct app_cfg_t *tmp_app_cfg = tmp_droiddev_cfg->app_cfg;
    while (tmp_app_cfg) {
      tmp_droiddev_cfg->app_cfg = tmp_droiddev_cfg->app_cfg->next;
      free(tmp_app_cfg);
      tmp_app_cfg = tmp_droiddev_cfg->app_cfg;
    }
    mrvl_platform_cfg->droid_dev_cfg = mrvl_platform_cfg->droid_dev_cfg->next;
    free(tmp_droiddev_cfg);
    tmp_droiddev_cfg = mrvl_platform_cfg->droid_dev_cfg;
  }

  free(mrvl_platform_cfg);
  mrvl_platform_cfg = NULL;
}
