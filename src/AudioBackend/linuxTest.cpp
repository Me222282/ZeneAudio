// #include <corecrt_math_defines.h>
// #include <thread>
// #include <cmath.h>
#include <stdio.h>
#include <iostream>
#include <alsa/asoundlib.h>
#include <math.h>
#include <pthread.h>
#include <thread>

#include "system.h"

// unsigned int channels = 2;
// const float pi2 = M_PI * 2;
// uint64_t phase = 0;

// void fillBuffer(snd_pcm_t* pcm_handle, snd_pcm_uframes_t period)
// {
//     snd_pcm_uframes_t frames;
//     while (period > 0)
//     {
//         frames = period;
        
//         const snd_pcm_channel_area_t* areas = NULL;
//         snd_pcm_uframes_t offset = 0;
//         int r = snd_pcm_mmap_begin(pcm_handle, &areas, &offset, &frames);
//         const char* ptr = snd_strerror(r);
//         if (r < 0) { return; }
        
//         float* map = (float*)((unsigned char *)areas[0].addr + (areas[0].first / 8) + (offset * (areas[0].step / 8)));
        
//         for (uint64_t i = 0; i < frames; i++)
//         {
//             float l = ((float)phase / 44100) * 400;
//             float v = sin(pi2 * l) * 0.8f;
//             //float v = (l - (float)((int)l)) * 0.8f;
//             //float v = ((long)(l * 2) % 2 == 0) * 0.8f;
//             phase++;
            
//             map[i * 2] = v;
//             map[(i * 2) + 1] = v;
//         }
        
//         snd_pcm_mmap_commit(pcm_handle, offset, frames);
        
//         period -= frames;
//     }
// }

// void threadFunc(snd_async_handler_t* ahandler)
// {
//     snd_pcm_t* pcm_handle = snd_async_handler_get_pcm(ahandler);
    
//     snd_pcm_uframes_t period = (snd_pcm_uframes_t)snd_pcm_avail_update(pcm_handle);
    
//     fillBuffer(pcm_handle, period);
// }

// int main()
// {
//     printf("test\n");
    
//     void* dc = NULL;
//     initialise(false, &dc);
    
//     snd_pcm_hw_params_t* hwparams = (snd_pcm_hw_params_t*)calloc(1, snd_pcm_hw_params_sizeof());
//     //snd_pcm_sw_params_t* swparams = (snd_pcm_sw_params_t*)calloc(1, snd_pcm_sw_params_sizeof());
    
//     void* device = getDefaultOutput(dc);
//     uint32_t tmp = 0;
//     printf("%s\n", getDeviceName(device, &tmp));
//     snd_pcm_t* pcm_handle = *(snd_pcm_t**)device;
    
//     if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0) {
//         fprintf(stderr, "Can not configure this PCM device.\n");
//         return(-1);
//     }
    
//     /*
//     snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_FLOAT_BE,
//         SND_PCM_ACCESS_RW_INTERLEAVED,
//         channels, 44100, 1,
//         (8192 * 2) / (44100 * 8));*/
    
//     int e = snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_FLOAT_LE);
//     if (e < 0) { return(-1); }
//     e = snd_pcm_hw_params_set_rate_resample(pcm_handle, hwparams, 1);
//     if (e < 0) { return(-1); }
//     e = snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_MMAP_INTERLEAVED);
//     if (e < 0) { return(-1); }
//     e = snd_pcm_hw_params_set_channels(pcm_handle, hwparams, channels);
//     if (e < 0) { return(-1); }
//     e = snd_pcm_hw_params_set_rate(pcm_handle, hwparams, 44100, 0);
//     if (e < 0) { return(-1); }
//     //e = snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, 1024);
//     unsigned int bt = 500000;
//     e = snd_pcm_hw_params_set_buffer_time_near(pcm_handle, hwparams, &bt, NULL);
//     if (e < 0) { return(-1); }
//     snd_pcm_uframes_t ps = 1024 * sizeof(float) * 2;
//     unsigned int pt = 100000;
//     //e = snd_pcm_hw_params_set_period_size_near(pcm_handle, hwparams, &ps, NULL);
//     e = snd_pcm_hw_params_set_period_time_near(pcm_handle, hwparams, &pt, NULL);
//     if (e < 0) { return(-1); }
    
//     e = snd_pcm_hw_params_get_period_size(hwparams, &ps, NULL);
//     if (e < 0) { return(-1); }
    
//     e = snd_pcm_hw_params(pcm_handle, hwparams);
//     if (e < 0) { return(-1); }
    
//     // e = snd_pcm_sw_params_current(pcm_handle, swparams);
//     // if (e < 0) { return(-1); }
//     // /* start the transfer when the buffer is almost full: */
//     // /* (buffer_size / avail_min) * avail_min */
//     // e = snd_pcm_sw_params_set_start_threshold(pcm_handle, swparams, (1024 / ps) * ps);
//     // if (e < 0) { return(-1); }
//     // /* allow the transfer when at least period_size samples can be processed */
//     // /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
//     // e = snd_pcm_sw_params_set_avail_min(pcm_handle, swparams, ps);
//     // if (e < 0) { return(-1); }
//     // /* write the parameters to the playback device */
//     // e = snd_pcm_sw_params(pcm_handle, swparams);
//     // if (e < 0) { return(-1); }
    
//     snd_async_handler_t *ahandler;
//     e = snd_async_add_pcm_handler(&ahandler, pcm_handle, threadFunc, NULL);
//     if (e < 0) { return(-1); }
    
//     fillBuffer(pcm_handle, ps);
//     fillBuffer(pcm_handle, ps);
    
//     e = snd_pcm_start(pcm_handle);
//     const char* str = snd_strerror(e);
//     if (e < 0) { return(-1); }
    
//     /*
//     bool running = true;
//     std::thread thread = std::thread{ [pcm_handle, &running ]()
//     {
//         while (running)
//         {
//             threadFunc(pcm_handle);
            
//             //snd_pcm_wait(pcm_handle, -1 /* wait infinit );
//         }
//     } };
    
//     std::this_thread::sleep_for(std::chrono::seconds(5));
//     running = false;
    
//     if (thread.joinable())
//     {
//         thread.join();
//     }*/
    
//     std::this_thread::sleep_for(std::chrono::seconds(5));
    
//     snd_pcm_drop(pcm_handle);
    
//     //snd_pcm_close(pcm_handle);
    
//     deleteInitialiser(dc);
    
//     printf("\n");
//     return 0;
// }
float frequency_hz = 440.0f;
int phase = 0;

float noteToFreq(int n)
{
    return 27.5f * std::pow(2.0f, n / 12.0f);
}

void Func(float* block, uint32_t size, uint32_t nChannels)
{
    for (uint32_t i = 0; i < size; i++)
    {
        phase++;
        
        float v = (float)sin(((M_PI * phase * 2) / 44100) * frequency_hz * 2) * 0.8f;
        
        //block[i] = sin(((M_PI * i * 2) / 44100) * 200) * 0.2f;
        block[i * 2] = v;
        block[(i * 2) + 1] = v;
    }
    /*
    for (uint64_t i = 0; i < size; i++)
    {
        float l = ((float)phase / 44100) * 400;
        float v = sin(M_PI * l * 2) * 0.8f;
        //float v = (l - (float)((int)l)) * 0.8f;
        //float v = ((long)(l * 2) % 2 == 0) * 0.8f;
        phase++;

        block[i * 2] = v;
        block[(i * 2) + 1] = v;
    }*/
}

int main()
{
    void* deviceCollection;
    initialise(true, &deviceCollection);
    void* device = getDefaultOutput(deviceCollection);
    void* audioSystem = createAudioSystem(device, 1024);
    printf("%d\n", getASBlockSize(audioSystem));
    printf("%d\n", getASNumChannels(audioSystem));
    printf("%lu\n", getASSampleRate(audioSystem));
    /*
    int devCount;
    void** outs = getOutputs(deviceCollection, &devCount);
    for (int i = 0; i < devCount; i++)
    {
        printf("%s\n", getDeviceName(outs[i]).c_str());
    }*/
    
    //frequency_hz = 440.0f;
    
    int notes[8] = {
        27, 31, 34, 38, 39, 38, 34, 31
    };
    int bpm = 140 * 2;
    
    setASCallback(audioSystem, Func);
    
    bool b = startAS(audioSystem);
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            frequency_hz = noteToFreq(notes[j]);
            std::this_thread::sleep_for(std::chrono::milliseconds(60'000 / bpm));
        }
    }
    stopAS(audioSystem);
    
    deleteAudioSystem(audioSystem);
    deleteInitialiser(deviceCollection);
    return 0;
}