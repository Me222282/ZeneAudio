// #include <corecrt_math_defines.h>
// #include <thread>
// #include <cmath.h>
#include <stdio.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <pthread.h>
#include <thread>

unsigned int channels = 2;
const float pi2 = M_PI * 2;
uint64_t phase = 0;

void fillBuffer(snd_pcm_t* pcm_handle, snd_pcm_uframes_t period)
{
    snd_pcm_uframes_t frames;
    while (period > 0)
    {
        frames = period;
        
        const snd_pcm_channel_area_t* areas = NULL;
        snd_pcm_uframes_t offset = 0;
        int r = snd_pcm_mmap_begin(pcm_handle, &areas, &offset, &frames);
        const char* ptr = snd_strerror(r);
        if (r < 0) { return; }
        
        float* map = (float*)((unsigned char *)areas[0].addr + (areas[0].first / 8) + (offset * (areas[0].step / 8)));
        
        for (uint64_t i = 0; i < frames; i++)
        {
            float v = sin(((pi2 * (float)phase * 2) / 44100) * 200) * 0.8f;
            phase++;
            
            map[i * 2] = v;
            map[(i * 2) + 1] = v;
        }
        
        snd_pcm_mmap_commit(pcm_handle, offset, frames);
        
        period -= frames;
    }
}

void threadFunc(snd_async_handler_t* ahandler)
{
    snd_pcm_t* pcm_handle = snd_async_handler_get_pcm(ahandler);
    
    snd_pcm_uframes_t period = (snd_pcm_uframes_t)snd_pcm_avail_update(pcm_handle);
    
    fillBuffer(pcm_handle, period);
}

int main()
{
    printf("test");
    
    // device list name test
    void** hints;
    /* Enumerate sound devices */
    int err = snd_device_name_hint(-1, "pcm", &hints);
    if (err < 0) { return(-1); }

    for (void** n = hints; *n != NULL; n++)
    {
        char* name = snd_device_name_get_hint(*n, "NAME");
        
        if (name == NULL) { continue; }
        char* t_name = strdup(name);
        free(name);
        snd_pcm_t* pcm;
        int er = snd_pcm_open(&pcm, t_name, SND_PCM_STREAM_PLAYBACK, 0);
        if (er < 0) { continue; }
        snd_pcm_info_t* info;
        snd_pcm_info_alloca(&info);
        er = snd_pcm_info(pcm, info);
        if (er < 0) { continue; }
        const char* name2 = snd_pcm_info_get_name(info);
        printf("%s\n", name2);
        //snd_pcm_info_free(info);
        snd_pcm_close(pcm);
    }
    //Free hint buffer too
    snd_device_name_free_hint(hints);
    
    char* pcm_name = strdup("plughw:0,0");
    snd_pcm_hw_params_t* hwparams;
    snd_pcm_sw_params_t* swparams;
    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);
    
    snd_pcm_t* pcm_handle;
    
    /* Open PCM. The last parameter of this function is the mode. */
    /* If this is set to 0, the standard mode is used. Possible   */
    /* other values are SND_PCM_NONBLOCK and SND_PCM_ASYNC.       */ 
    /* If SND_PCM_NONBLOCK is used, read / write access to the    */
    /* PCM device will return immediately. If SND_PCM_ASYNC is    */
    /* specified, SIGIO will be emitted whenever a period has     */
    /* been completely processed by the soundcard.                */
    if (snd_pcm_open(&pcm_handle, pcm_name, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        fprintf(stderr, "Error opening PCM device %s\n", pcm_name);
        return(-1);
    }
    
    if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0) {
        fprintf(stderr, "Can not configure this PCM device.\n");
        return(-1);
    }
    
    /*
    snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_FLOAT_BE,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        channels, 44100, 1,
        (8192 * 2) / (44100 * 8));*/
    
    int e = snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_FLOAT_LE);
    if (e < 0) { return(-1); }
    e = snd_pcm_hw_params_set_rate_resample(pcm_handle, hwparams, 1);
    if (e < 0) { return(-1); }
    e = snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_MMAP_INTERLEAVED);
    if (e < 0) { return(-1); }
    e = snd_pcm_hw_params_set_channels(pcm_handle, hwparams, channels);
    if (e < 0) { return(-1); }
    e = snd_pcm_hw_params_set_rate(pcm_handle, hwparams, 44100, 0);
    if (e < 0) { return(-1); }
    //e = snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, 1024);
    unsigned int bt = 500000;
    e = snd_pcm_hw_params_set_buffer_time_near(pcm_handle, hwparams, &bt, NULL);
    if (e < 0) { return(-1); }
    snd_pcm_uframes_t ps = 1024 * sizeof(float) * 2;
    unsigned int pt = 100000;
    //e = snd_pcm_hw_params_set_period_size_near(pcm_handle, hwparams, &ps, NULL);
    e = snd_pcm_hw_params_set_period_time_near(pcm_handle, hwparams, &pt, NULL);
    if (e < 0) { return(-1); }
    
    e = snd_pcm_hw_params_get_period_size(hwparams, &ps, NULL);
    if (e < 0) { return(-1); }
    
    e = snd_pcm_hw_params(pcm_handle, hwparams);
    if (e < 0) { return(-1); }
    
    // e = snd_pcm_sw_params_current(pcm_handle, swparams);
    // if (e < 0) { return(-1); }
    // /* start the transfer when the buffer is almost full: */
    // /* (buffer_size / avail_min) * avail_min */
    // e = snd_pcm_sw_params_set_start_threshold(pcm_handle, swparams, (1024 / ps) * ps);
    // if (e < 0) { return(-1); }
    // /* allow the transfer when at least period_size samples can be processed */
    // /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
    // e = snd_pcm_sw_params_set_avail_min(pcm_handle, swparams, ps);
    // if (e < 0) { return(-1); }
    // /* write the parameters to the playback device */
    // e = snd_pcm_sw_params(pcm_handle, swparams);
    // if (e < 0) { return(-1); }
    
    snd_async_handler_t *ahandler;
    e = snd_async_add_pcm_handler(&ahandler, pcm_handle, threadFunc, NULL);
    if (e < 0) { return(-1); }
    
    fillBuffer(pcm_handle, ps);
    fillBuffer(pcm_handle, ps);
    
    e = snd_pcm_start(pcm_handle);
    const char* str = snd_strerror(e);
    if (e < 0) { return(-1); }
    
    /*
    bool running = true;
    std::thread thread = std::thread{ [pcm_handle, &running ]()
    {
        while (running)
        {
            threadFunc(pcm_handle);
            
            //snd_pcm_wait(pcm_handle, -1 /* wait infinit );
        }
    } };
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    running = false;
    
    if (thread.joinable())
    {
        thread.join();
    }*/
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    snd_pcm_drop(pcm_handle);
    
    snd_pcm_close(pcm_handle);
    
    printf("\n");
    return 0;
}