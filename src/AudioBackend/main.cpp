#include <corecrt_math_defines.h>
#include "system.h"
#include <thread>
#include <cmath>

float frequency_hz = 440.0f;
int phase = 0;

float noteToFreq(int n)
{
    return 27.5f * std::pow(2.0f, n / 12.0f);
}

void Func(float* block, uint32_t size, uint32_t nChannels, uint32_t current)
{
    for (uint32_t i = 0; i < size; i++)
    {
        phase++;
        //block[i] = sin(((M_PI * i * 2) / 44100) * 200) * 0.2f;
        block[i * 2] = (float)sin(((M_PI * phase * 2) / 44100) * frequency_hz) * 0.2f;
        block[(i * 2) + 1] = (float)sin(((M_PI * phase * 2) / 44100) * frequency_hz) * 0.2f;
    }
}

int main()
{
    void* deviceCollection;
    initialise(false, &deviceCollection);
    void* device = getDefaultOutput(deviceCollection);
    void* audioSystem = createAudioSystem(device, 1024);
    
    //frequency_hz = 440.0f;
    
    int notes[8] = {
        27, 31, 34, 38, 39, 38, 34, 31
    };
    int bpm = 140 * 2;
    
    setASCallback(audioSystem, Func);
    
    startAS(audioSystem);
    for (int i = 0; i < 4; i++)
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