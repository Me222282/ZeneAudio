/*#include <stdio.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <thread>
#include <cmath>
#include <corecrt_math_defines.h>
#include <stdexcept>

typedef void (*callback)(float*, UINT32);

typedef struct format
{
    DWORD sampleRate;
    WORD blockSize;
    WORD bitSize;
    unsigned int isStereo;
    unsigned int isFloating;
} format;

IMMDevice* device_render_default = NULL;
IMMDevice* device_capture_default = NULL;
std::thread _processing_thread;

IMMDevice** device_render_array;
int device_render_count = 0;
IMMDevice** device_capture_array;
int device_capture_count = 0;

WAVEFORMATEXTENSIBLE* pwfx = NULL;

void _fixup_pwfx()
{
    pwfx->Format.nBlockAlign = pwfx->Format.nChannels * pwfx->Format.wBitsPerSample / 8;
    pwfx->Format.nAvgBytesPerSec = pwfx->Format.nSamplesPerSec * pwfx->Format.wBitsPerSample * pwfx->Format.nChannels / 8;
}

bool Initialise(bool captures);
IAudioClient* RunDevice(IMMDevice* device, callback callback, UINT blocksize);
bool shouldRun = true;

void Func(float* block, UINT32 size)
{
    for (UINT32 i = 0; i < size; i++)
    {
        //block[i] = sin(((M_PI * i * 2) / 44100) * 200) * 0.2f;
        block[i * 2] = sin(((M_PI * i * 2) / 44100) * 200) * 0.2f;
        block[(i * 2) + 1] = sin(((M_PI * i * 2) / 44100) * 200) * 0.2f;
    }
}

int playSound(IMMDevice* dev, callback cb, DWORD length)
{
    IAudioClient* c = RunDevice(dev, cb, 1024);
    if (c == NULL)
    {
        printf("ERROR");
        return 1;
    }
    
    printf("\nSample Rate: %d", pwfx->Format.nSamplesPerSec);
    printf("\nChannels: %d", pwfx->Format.nChannels);
    printf("\ncbsize: %d", pwfx->Format.cbSize);
    printf("\nba: %d", pwfx->Format.nBlockAlign);
    printf("\nspeak: %d", pwfx->dwChannelMask);
    
    Sleep(length);
    shouldRun = false;
    _processing_thread.join();
    c->Stop();
    return 0;
}

int main()
{
    bool result = Initialise(false);
    if (!result)
    {
        printf("ERROR");
        return 1;
    }
    
    printf("%d", device_render_count);
    /*
    for (int i = 0; i < device_render_count; i++)
    {
        printf("\nDevice: %d", i);
        
        playSound(device_render_array[i], Func, 5000);
    }
    
    playSound(device_render_default, Func, 5000);
    
    return 0;
}

bool Initialise(bool captures)
{
    HRESULT hr;
    IMMDeviceEnumerator* enumerator = NULL;
    
    hr = CoInitialize(NULL);
    if (FAILED(hr)) { return false; }
    
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    if (FAILED(hr)) { return false; }
    
    // Get render devices
    IMMDeviceCollection* device_collection = NULL;
    hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &device_collection);
    if (FAILED(hr)) { return false; }
    
    UINT device_count = 0;
    hr = device_collection->GetCount(&device_count);
    if (FAILED(hr)) { return false; }
    
    device_render_count = device_count;
    device_render_array = new IMMDevice*[device_count];
    for (UINT i = 0; i < device_count; i++)
    {
        IMMDevice* device = NULL;
        hr = device_collection->Item(i, &device);
        if (FAILED(hr))
        {
            if (device != NULL)
            {
                device->Release();
            }
            continue;
        }

        device_render_array[i] = device;
    }
    
    if (captures)
    {
        // Get capture devices
        device_collection = NULL;
        hr = enumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &device_collection);
        if (FAILED(hr)) { return false; }
        
        device_count = 0;
        hr = device_collection->GetCount(&device_count);
        if (FAILED(hr)) { return false; }
        
        device_capture_count = device_count;
        device_capture_array = new IMMDevice*[device_count];
        for (UINT i = 0; i < device_count; i++)
        {
            IMMDevice* device = NULL;
            hr = device_collection->Item(i, &device);
            if (FAILED(hr))
            {
                if (device != NULL)
                {
                    device->Release();
                }
                continue;
            }

            device_capture_array[i] = device;
        }
    }
    
    // Get defaults
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device_render_default);
    if (FAILED(hr)) { return false; }
    if (captures)
    {
        hr = enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &device_capture_default);
        if (FAILED(hr)) { return false; }
    }
    
    return true;
}

WAVEFORMATEXTENSIBLE* getWAVfromFormat(format* format)
{
    WAVEFORMATEXTENSIBLE* wf = new WAVEFORMATEXTENSIBLE();
    
    wf->Format.cbSize = 22;
    wf->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    if (format->isFloating && format->bitSize != 32)
    {
        throw std::invalid_argument("Invalid format.");
    }
    if (format->isFloating)
    {
        wf->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    }
    else
    {
        wf->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    }
    wf->Format.wBitsPerSample = format->bitSize;
    wf->Samples.wValidBitsPerSample = format->bitSize;
    if (format->isStereo)
    {
        wf->Format.nChannels = 2;
        wf->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
    }
    else
    {
        wf->Format.nChannels = 1;
        wf->dwChannelMask = SPEAKER_FRONT_CENTER;
    }
    wf->Format.nSamplesPerSec = format->sampleRate;
    wf->Format.nBlockAlign = wf->Format.nChannels * wf->Format.wBitsPerSample / 8;
    wf->Format.nAvgBytesPerSec = wf->Format.nSamplesPerSec * wf->Format.wBitsPerSample * wf->Format.nChannels / 8;
    wf->Samples.wSamplesPerBlock = format->blockSize;
    
    return wf;
}

void threadFunction(IAudioClient* client, IAudioRenderClient* iarc, callback callback, UINT blockSize)
{
    UINT32 current_padding = 0;
    client->GetCurrentPadding(&current_padding);

    UINT32 num_frames_available = blockSize - current_padding;
    if (num_frames_available == 0) { return; }
    
    BYTE* data = NULL;
    iarc->GetBuffer(num_frames_available, &data);
    if (data == NULL) { return; }
    
    callback(reinterpret_cast<float*>(data), num_frames_available);

    iarc->ReleaseBuffer(num_frames_available, 0);
}

IAudioClient* RunDevice(IMMDevice* device, callback callback, UINT blocksize)
{
    HRESULT hr;
    IAudioClient* client = NULL;
    
    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&client);
    if (FAILED(hr)) { return NULL; }
    
    WAVEFORMATEX* wfex = NULL;
    hr =  client->GetMixFormat(&wfex);
    if (FAILED(hr)) { return NULL; }
    pwfx = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(wfex);
    
    client->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        (10'000'000.0 * blocksize) / pwfx->Format.nSamplesPerSec,
        0, &pwfx->Format, NULL);
    
    IAudioRenderClient* iarc;
    client->GetService(__uuidof(IAudioRenderClient), (void**)&iarc);
    
    UINT32 _buffer_frame_count;
    hr = client->GetBufferSize(&_buffer_frame_count);
    if (FAILED(hr)) { return NULL; }
    
    // if (_buffer_frame_count != blocksize)
    // {
    //     printf("Frame size missmatch\n");
    //     printf("blocksize = %d\n", _buffer_frame_count);
    // }
    
    HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = client->SetEventHandle(event);
    if (FAILED(hr)) { return NULL; }
    
    // Ensure using floating point samples
    pwfx->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    pwfx->Format.wBitsPerSample = sizeof(float) * 8;
    pwfx->Samples.wValidBitsPerSample = pwfx->Format.wBitsPerSample;
    _fixup_pwfx();
    
    hr = client->Start();
    if (FAILED(hr)) { return NULL; }
    
    shouldRun = true;
    _processing_thread = std::thread{ [event, _buffer_frame_count, client, iarc, callback]()
    {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

        while (shouldRun)
        {
            threadFunction(client, iarc, callback, _buffer_frame_count);
            
            // Wait
            WaitForSingleObject(event, INFINITE);
        }
    } };
    
    return client;
}
*/

#include <corecrt_math_defines.h>
#include "system.h"
#include <thread>
#include <cmath>
#include <inttypes.h>

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