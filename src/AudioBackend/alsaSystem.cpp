#include <alsa/asoundlib.h>

#include "system.h"

typedef struct
{
    snd_pcm_t* _handle;
    snd_pcm_info_t* _info;
} pcmDevice;

typedef struct
{
    pcmDevice* _device;
    uint32_t _sampleRate;
    uint32_t _blockSize;
    uint16_t _nChannels;
    audioCallback _callback;
    bool _running;
} audioSystem;

typedef struct
{
    pcmDevice* _device_render_default;
    pcmDevice* _device_capture_default;

    pcmDevice** _device_render_array;
    int _device_render_count;
    pcmDevice** _device_capture_array;
    int _device_capture_count;
} deviceInitialiser;

uint64_t getASSampleRate(void* audioSys)
{
    return ((audioSystem*)audioSys)->_sampleRate;
}
uint32_t getASBlockSize(void* audioSys)
{
    return ((audioSystem*)audioSys)->_blockSize;
}
uint16_t getASNumChannels(void* audioSys)
{
    return ((audioSystem*)audioSys)->_nChannels;
}

void fillFrame(snd_pcm_t* pcm, audioCallback callback, uint16_t channels, snd_pcm_uframes_t size)
{
    snd_pcm_uframes_t frames;
    while (size > 0)
    {
        frames = size;
        
        const snd_pcm_channel_area_t* areas = NULL;
        snd_pcm_uframes_t offset = 0;
        int r = snd_pcm_mmap_begin(pcm, &areas, &offset, &frames);
        if (r < 0 || frames <= 0) { return; }
        
        float* map = (float*)((unsigned char *)areas[0].addr + (areas[0].first / 8) + (offset * (areas[0].step / 8)));
        
        callback(map, frames, channels);
        
        snd_pcm_mmap_commit(pcm, offset, frames);
        
        size -= frames;
    }
}
void renderThreadFunc(snd_async_handler_t* ahandler)
{
    snd_pcm_t* pcm_handle = snd_async_handler_get_pcm(ahandler);
    snd_pcm_uframes_t period = (snd_pcm_uframes_t)snd_pcm_avail_update(pcm_handle);
    audioSystem* sys = (audioSystem*)snd_async_handler_get_callback_private(ahandler);
    fillFrame(pcm_handle, sys->_callback, sys->_nChannels, period);
}

bool startAS(void* audioSys)
{
    audioSystem* sys = (audioSystem*)audioSys;
    
    if (sys->_running) { return false; }
    
    fillFrame(sys->_device->_handle, sys->_callback, sys->_nChannels, sys->_blockSize);
    
    int e = snd_pcm_start(sys->_device->_handle);
    if (e < 0) { return false; }
    
    sys->_running = true;
    return true;
}
bool isASRunning(void* audioSys)
{
    return ((audioSystem*)audioSys)->_running;
}
void stopAS(void* audioSys)
{
    audioSystem* sys = (audioSystem*)audioSys;
    
    if (!sys->_running) { return; }
    
    snd_pcm_drop(sys->_device->_handle);
    sys->_running = false;
}

void setASCallback(void* audioSys, audioCallback callback)
{
    ((audioSystem*)audioSys)->_callback = callback;
}

void* getAudioSystemDevice(void* audioSys)
{
    return *(pcmDevice**)audioSys;
}
void* createAudioSystem(void* outputDevice, uint32_t blockSize)
{
    pcmDevice* device = (pcmDevice*)outputDevice;
    snd_pcm_t* pcm = device->_handle;
    
    snd_pcm_hw_params_t* hwparams = (snd_pcm_hw_params_t*)calloc(1, snd_pcm_hw_params_sizeof());
    
    int e = snd_pcm_hw_params_any(pcm, hwparams);
    if (e < 0) { return NULL; }
    
    snd_pcm_chmap_query_t** cm = snd_pcm_query_chmaps(pcm);
    uint32_t channels = cm[0]->map.channels;
    
    e = snd_pcm_hw_params_set_format(pcm, hwparams, SND_PCM_FORMAT_FLOAT_LE);
    if (e < 0) { return NULL; }
    e = snd_pcm_hw_params_set_rate_resample(pcm, hwparams, false);
    if (e < 0) { return NULL; }
    e = snd_pcm_hw_params_set_access(pcm, hwparams, SND_PCM_ACCESS_MMAP_INTERLEAVED);
    if (e < 0) { return NULL; }
    e = snd_pcm_hw_params_set_channels(pcm, hwparams, channels);
    if (e < 0) { return NULL; }
    uint32_t rate = 44100;
    e = snd_pcm_hw_params_set_rate_near(pcm, hwparams, &rate, NULL);
    if (e < 0) { return NULL; }
    //e = snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, 1024);
    e = snd_pcm_hw_params_set_buffer_size_near(pcm, hwparams, (snd_pcm_uframes_t*)&blockSize);
    if (e < 0) { return NULL; }
    snd_pcm_uframes_t ps = blockSize * sizeof(float) * 2;
    e = snd_pcm_hw_params_set_period_size_near(pcm, hwparams, &ps, NULL);
    if (e < 0) { return NULL; }
    
    e = snd_pcm_hw_params(pcm, hwparams);
    if (e < 0) { return NULL; }
    
    audioSystem* sys = new audioSystem();
    sys->_device = device;
    sys->_sampleRate = rate;
    sys->_nChannels = channels;
    sys->_blockSize = blockSize;
    
    snd_async_handler_t *ahandler;
    e = snd_async_add_pcm_handler(&ahandler, device->_handle, renderThreadFunc, sys);
    if (e < 0)
    {
        deleteAudioSystem(sys);
        return NULL;
    }
    
    return sys;
}
void deleteAudioSystem(void* audioSys)
{
    delete (audioSystem*)audioSys;
}

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
