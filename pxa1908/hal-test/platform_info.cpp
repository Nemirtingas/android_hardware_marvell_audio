#include "common.h"
#include "platform.h"
#include "audio_device.h"

#include <libxml/tree.h>

#define PLATFORM_INFO_XML_PATH      "/system/etc/audio_platform_info.xml"

struct platform_info
{
    platform_data    *platform;
    std::map<std::string, std::string> backend_names;
    std::map<std::string, std::string> device_names;
};

static struct platform_info my_data;

static void parse_device_names(xmlNodePtr node)
{
    for( ; node; node = node->next )
    {
        if( !xmlStrcmp(node->name, (xmlChar*)"device") )
        {
            xmlChar *name, *alias;
            name = xmlGetProp(node, (xmlChar*)"name");
            alias = xmlGetProp(node, (xmlChar*)"alias");
            if( name && alias )
                my_data.device_names[reinterpret_cast<char*>(name)] = reinterpret_cast<char*>(alias);

            if( name ) xmlFree(name);
            if( alias ) xmlFree(alias);
        }
    }
}

static void parse_pcm_ids(xmlNodePtr node)
{
/*
    int index;

    if (strcmp(attr[0], "name") != 0) {
        ALOGE("%s: 'name' not found, no pcm_id set!", __func__);
        goto done;
    }

    index = platform_get_usecase_index((char *)attr[1]);
    if (index < 0) {
        ALOGE("%s: usecase %s in %s not found!",
              __func__, attr[1], PLATFORM_INFO_XML_PATH);
        goto done;
    }

    if (strcmp(attr[2], "type") != 0) {
        ALOGE("%s: usecase type not mentioned", __func__);
        goto done;
    }

    int type = -1;

    if (!strcasecmp((char *)attr[3], "in")) {
        type = 1;
    } else if (!strcasecmp((char *)attr[3], "out")) {
        type = 0;
    } else {
        ALOGE("%s: type must be IN or OUT", __func__);
        goto done;
    }

    if (strcmp(attr[4], "id") != 0) {
        ALOGE("%s: usecase id not mentioned", __func__);
        goto done;
    }

    int id = atoi((char *)attr[5]);

    if (platform_set_usecase_pcm_id(index, type, id) < 0) {
        ALOGE("%s: usecase %s in %s, type %d id %d was not set!",
              __func__, attr[1], PLATFORM_INFO_XML_PATH, type, id);
        goto done;
    }

done:
    return;
*/
}

static void parse_backend_names(xmlNodePtr node)
{
    for( ; node; node = node->next )
    {
        if( !xmlStrcmp(node->name, (xmlChar*)"device") )
        {
            xmlChar *name, *backend;
            name = xmlGetProp(node, (xmlChar*)"name");
            backend = xmlGetProp(node, (xmlChar*)"backend");
            if( name && backend )
                my_data.backend_names[reinterpret_cast<char*>(name)] = reinterpret_cast<char*>(backend);

            if( name ) xmlFree(name);
            if( backend ) xmlFree(backend);
        }
    }
}

static void parse_config_params(xmlNodePtr node)
{
/*
    if (strcmp(attr[0], "key") != 0) {
        ALOGE("%s: 'key' not found", __func__);
        goto done;
    }

    if (strcmp(attr[2], "value") != 0) {
        ALOGE("%s: 'value' not found", __func__);
        goto done;
    }

    str_parms_add_str(my_data.kvpairs, (char*)attr[1], (char*)attr[3]);
    platform_set_parameters(my_data.platform, my_data.kvpairs);
done:
    return;
*/
}

static void parse_operator_specific(xmlNodePtr node)
{
    /*
    snd_device_t snd_device = SND_DEVICE_NONE;

    if (strcmp(attr[0], "name") != 0) {
        ALOGE("%s: 'name' not found", __func__);
        goto done;
    }

    snd_device = platform_get_snd_device_index((char *)attr[1]);
    if (snd_device < 0) {
        ALOGE("%s: Device %s in %s not found, no ACDB ID set!",
              __func__, (char *)attr[3], PLATFORM_INFO_XML_PATH);
        goto done;
    }

    if (strcmp(attr[2], "operator") != 0) {
        ALOGE("%s: 'operator' not found", __func__);
        goto done;
    }

    if (strcmp(attr[4], "mixer_path") != 0) {
        ALOGE("%s: 'mixer_path' not found", __func__);
        goto done;
    }

    if (strcmp(attr[6], "acdb_id") != 0) {
        ALOGE("%s: 'acdb_id' not found", __func__);
        goto done;
    }

    platform_add_operator_specific_device(snd_device, (char *)attr[3], (char *)attr[5], atoi((char *)attr[7]));

done:
    return;
    */
}

int platform_info_init(platform_data *data)
{
    const char *config_file = PLATFORM_INFO_XML_PATH;
    xmlDocPtr xml_doc;
    xmlNodePtr root_elem;
    xmlNodePtr node;

    my_data.platform = data;

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

    if( xmlStrcmp(root_elem->name, (xmlChar*)"audio_platform_info") )
    {
        ALOGE("%s: wrong type document, root node != audio_platform_info", __FUNCTION__);
        xmlFreeDoc(xml_doc);
        return -1;
    }

    for( node = root_elem->children; node; node = node->next )
    {
        if( !xmlStrcmp(node->name, (xmlChar*)"backend_names") )
        {
            parse_backend_names(node->children);
        }
        else if( !xmlStrcmp(node->name, (xmlChar*)"pcm_ids") )
        {
            parse_pcm_ids(node->children);
        }
        else if( !xmlStrcmp(node->name, (xmlChar*)"device_names") )
        {
            parse_device_names(node->children);
        }
        else if( !xmlStrcmp(node->name, (xmlChar*)"config_params") )
        {
            parse_config_params(node->children);
        }
        else if( !xmlStrcmp(node->name, (xmlChar*)"operator_specific") )
        {
            parse_operator_specific(node->children);
        }
    }

    xmlFreeDoc(xml_doc);
    return 0;
}
