#include <corecrt_math_defines.h>
#include "system.h"
#include <thread>
#include <cmath>

float frequency_hz = 440.0f;
int phase = 0;
readBuffer buff;
void* audioReader;

float noteToFreq(int n)
{
    return 27.5f * std::pow(2.0f, n / 12.0f);
}

void Func(float* block, uint32_t size, uint32_t nChannels)
{
    /*
    uint32_t avai = 0;
    float* source = readAudioSource(audioReader, size, &avai);
    // 2 channels
    avai *= 2;
    
    uint32_t readPoint = 0;*/
    for (uint32_t i = 0; i < size; i++)
    {
        phase++;
        
        // wait until no more overflows
        if (isOverflow(&buff))
        {
            printf("there is an overflow");
            printf(" R: %d", buff._readIndex);
            printf(" W: %d", buff._writeIndex);
            printf("\n");
            continue;
        }
        /*if (readPoint >= avai)
        {
            printf("%d ", size);
            printf("%d; ", avai / 2);
            return;
        }*/
        
        float v = (float)sin(((M_PI * phase * 2) / 44100) * frequency_hz) * 0.3f;
        // Read twice for both channels
        /*float v = source[readPoint];
        readPoint++;
        v += source[readPoint];
        readPoint++;*/
        v += buff._data[buff._readIndex];
        incBuffRead(&buff);
        v += buff._data[buff._readIndex];
        incBuffRead(&buff);
        
        //block[i] = sin(((M_PI * i * 2) / 44100) * 200) * 0.2f;
        block[i * 2] = v;
        block[(i * 2) + 1] = v;
    }
}

int main()
{
    void* deviceCollection;
    initialise(true, &deviceCollection);
    void* device = getDefaultOutput(deviceCollection);
    void* audioSystem = createAudioSystem(device, 1024);
    /*
    int devCount;
    void** outs = getOutputs(deviceCollection, &devCount);
    for (int i = 0; i < devCount; i++)
    {
        printf("%s\n", getDeviceName(outs[i]).c_str());
    }*/
    
    //frequency_hz = 440.0f;
    
    void* mic = getDefaultInput(deviceCollection);
    audioReader = createAudioReader(mic, 1024 * 2);
    buff = *createReadBuffer(getARBlockSize(audioReader) * 4 * getARNumChannels(audioReader));
    setARBuffer(audioReader, &buff);
    printf("%s\n", getDeviceName(mic).c_str());
    printf("%d\n", getARSampleRate(audioReader));
    
    int notes[8] = {
        27, 31, 34, 38, 39, 38, 34, 31
    };
    int bpm = 140 * 2;
    
    setASCallback(audioSystem, Func);
    
    startAR(audioReader);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    startAS(audioSystem);
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            frequency_hz = noteToFreq(notes[j]);
            std::this_thread::sleep_for(std::chrono::milliseconds(60'000 / bpm));
        }
    }
    stopAS(audioSystem);
    stopAR(audioReader);
    
    deleteAudioSystem(audioSystem);
    deleteAudioSystem(audioReader);
    delete buff._data;
    deleteInitialiser(deviceCollection);
    return 0;
}