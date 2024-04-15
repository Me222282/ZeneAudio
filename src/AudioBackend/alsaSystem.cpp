#include <alsa/asoundlib.h>

#include "system.h"

typedef struct
{
    snd_pcm_t* _handle;
    snd_pcm_info_t* _info;
} pcmDevice;

typedef struct
{
    pcmDevice* _device_render_default;
    pcmDevice* _device_capture_default;

    pcmDevice** _device_render_array;
    int _device_render_count;
    pcmDevice** _device_capture_array;
    int _device_capture_count;
} deviceInitialiser;

const char* getDeviceName(void* device, uint32_t* size)
{
    pcmDevice* pcm = (pcmDevice*)device;
    const char* str = snd_pcm_info_get_name(pcm->_info);
    *size = sizeof(str);
    return str;
}

bool initialise(bool captures, void** deviceCollection)
{
    deviceInitialiser* di = new deviceInitialiser();
    *deviceCollection = di;
    
    void** hints;
    int err = snd_device_name_hint(-1, "pcm", &hints);
    if (err < 0) { return false; }
    
    int startSize = sizeof(hints);
    pcmDevice** ra = (pcmDevice**)malloc(startSize * sizeof(pcmDevice*));
    int drc = 0;
    
    for (void** n = hints; *n != NULL; n++)
    {   
        char* name = snd_device_name_get_hint(*n, "NAME");
        // Returns 1 if false
        if (name == NULL || strncmp(name, "plughw", 6)) { continue; }
        char* t_name = strdup(name);
        free(name);
        snd_pcm_t* pcm;
        err = snd_pcm_open(&pcm, t_name, SND_PCM_STREAM_PLAYBACK, 0);
        if (err < 0) { continue; }
        snd_pcm_info_t* info;
        snd_pcm_info_alloca(&info);
        err = snd_pcm_info(pcm, info);
        
        pcmDevice* device = new pcmDevice();
        device->_handle = pcm;
        device->_info = info;
        
        if (drc == 0)
        {
            di->_device_render_default = device;
        }
        
        ra[drc] = device;
        drc++;
    }
    // Free hint buffer
    snd_device_name_free_hint(hints);
    
    void* tmp = realloc(ra, drc * sizeof(pcmDevice*));
    if (tmp)
    {
        di->_device_render_array = (pcmDevice**)tmp;
        di->_device_render_count = drc;
    }
    else if (drc > 0)
    {
        di->_device_render_array = ra;
        di->_device_render_count = startSize;
    }
    else
    {
        return false;
    }
    
    return true;
}

void* getDefaultOutput(void* deviceCollection)
{
    return ((deviceInitialiser*)deviceCollection)->_device_render_default;
}
void** getOutputs(void* deviceCollection, int* size)
{
    *size = ((deviceInitialiser*)deviceCollection)->_device_render_count;
    return (void**)((deviceInitialiser*)deviceCollection)->_device_render_array;
}

void* getDefaultInput(void* deviceCollection)
{
    return ((deviceInitialiser*)deviceCollection)->_device_capture_default;
}
void** getInputs(void* deviceCollection, int* size)
{
    *size = ((deviceInitialiser*)deviceCollection)->_device_capture_count;
    return (void**)((deviceInitialiser*)deviceCollection)->_device_capture_array;
}

void deleteInitialiser(void* deviceCollection)
{
    deviceInitialiser* dc = (deviceInitialiser*)deviceCollection;
    
    // Release render devices
    for (int i = 0; i < dc->_device_render_count; i++)
    {
        snd_pcm_close(dc->_device_render_array[i]->_handle);
        delete dc->_device_render_array[i];
    }
    
    // Release capture devices
    for (int i = 0; i < dc->_device_capture_count; i++)
    {
        snd_pcm_close(dc->_device_capture_array[i]->_handle);
        delete dc->_device_capture_array[i];
    }
    
    free(dc->_device_render_array);
    free(dc->_device_capture_array);
    
    delete dc;
}