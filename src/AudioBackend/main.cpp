#include <stdio.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <thread>
#include <cmath>
#include <corecrt_math_defines.h>

typedef void (*callback)(__int32*, UINT32);

IMMDevice* device_render_default = NULL;
IMMDevice* device_capture_default = NULL;
std::thread _processing_thread;

IMMDevice** device_render_array;
int device_render_count = 0;
IMMDevice** device_capture_array;
int device_capture_count = 0;

WAVEFORMATEX* pwfx = NULL;

bool Initialise(bool captures);
IAudioClient* RunDevice(IMMDevice* device, callback callback);
bool shouldRun = true;

void Func(__int32* block, UINT32 size)
{
    for (UINT32 i = 0; i < size; i++)
    {
        block[i] = (UINT32)(sin(((M_PI * i * 2) / 44100) * 200) * 1073741823.75) + 2147483647;
    }
}

int playSound(IMMDevice* dev, callback cb, DWORD length)
{
    IAudioClient* c = RunDevice(dev, cb);
    if (c == NULL)
    {
        printf("ERROR");
        return 1;
    }
    
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
    
    for (int i = 0; i < device_render_count; i++)
    {
        printf("\nDevice: %d", i);
        if (pwfx == NULL)
        {
            printf("U O");
        }
        printf("\nSample Rate: %d", pwfx->nSamplesPerSec);
        printf("\nChannels: %d", pwfx->nChannels);
        printf("\ncbsize: %d", pwfx->cbSize);
        printf("\nba: %d", pwfx->nBlockAlign);
        
        playSound(device_render_array[i], Func, 5000);
    }
    
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

IAudioClient* RunDevice(IMMDevice* device, callback callback)
{
    HRESULT hr;
    IAudioClient* client = NULL;
    
    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&client);
    if (FAILED(hr)) { return NULL; }
    
    pwfx = NULL;
    hr =  client->GetMixFormat(&pwfx);
    if (FAILED(hr)) { return NULL; }
    
    client->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        10'000'000,
        0, pwfx, NULL);
    
    IAudioRenderClient* iarc;
    client->GetService(__uuidof(IAudioRenderClient), (void**)&iarc);
    
    UINT32 _buffer_frame_count;
    hr = client->GetBufferSize(&_buffer_frame_count);
    if (FAILED(hr)) { return NULL; }
    
    HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = client->SetEventHandle(event);
    if (FAILED(hr)) { return NULL; }
    
    hr = client->Start();
    if (FAILED(hr)) { return NULL; }
    
    _processing_thread = std::thread{ [event, _buffer_frame_count, client, iarc, callback]()
    {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

        while (shouldRun)
        {
            UINT32 current_padding = 0;
			client->GetCurrentPadding(&current_padding);

			UINT32 num_frames_available = _buffer_frame_count - current_padding;
			if (num_frames_available == 0)
				return;

			BYTE* data = NULL;
			iarc->GetBuffer(num_frames_available, &data);
			if (data != NULL)
            {
                callback(reinterpret_cast<__int32*>(data), num_frames_available);

			    iarc->ReleaseBuffer(num_frames_available, 0);
            }
            
            // Wait
            WaitForSingleObject(event, INFINITE);
        }
    } };
    
    return client;
}
