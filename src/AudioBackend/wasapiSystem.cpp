#include <stdio.h>
#include <stdint.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <thread>
#include <cmath>
#include <corecrt_math_defines.h>
#include <stdexcept>
#include <vector>

#include "system.h"

template<typename T>
class auto_release
{
public:
    auto_release(T*& value) :
        _value(value)
    {}

    ~auto_release()
    {
        if (_value != nullptr)
            _value->Release();
    }

private:
    T*& _value;
};

typedef struct
{
    IMMDevice* _source;
    std::string _name;
    std::wstring _id;
} audioDevice;

typedef struct
{
    audioDevice* _device;
    WAVEFORMATEXTENSIBLE* _format;
    IAudioClient* _client;
    IAudioRenderClient* _rc;
    HANDLE _event;
    uint32_t _blockSize;
    bool _running;
    audioCallback _callback;
    std::thread _thread;
} audioSystem;

typedef struct
{
    audioDevice* _device;
    WAVEFORMATEXTENSIBLE* _format;
    IAudioClient* _client;
    IAudioCaptureClient* _cc;
    HANDLE _event;
    uint32_t _blockSize;
    bool _running;
    readBuffer* _buffer;
    std::thread _thread;
} audioReader;

typedef struct
{
    audioDevice* _device_render_default;
    audioDevice* _device_capture_default;

    audioDevice** _device_render_array;
    int _device_render_count;
    audioDevice** _device_capture_array;
    int _device_capture_count;
} deviceInitialiser;

uint64_t getASSampleRate(void* audioSys)
{
    return ((audioSystem*)audioSys)->_format->Format.nSamplesPerSec;
}
uint32_t getASBlockSize(void* audioSys)
{
    return ((audioSystem*)audioSys)->_blockSize;
}
uint16_t getASNumChannels(void* audioSys)
{
    return ((audioSystem*)audioSys)->_format->Format.nChannels;
}

void threadFunction(IAudioClient* client, IAudioRenderClient* iarc, audioCallback callback, UINT blockSize, uint32_t channels)
{
    if (!callback) { return; }
    
    UINT32 current_padding = 0;
    client->GetCurrentPadding(&current_padding);

    UINT32 num_frames_available = blockSize - current_padding;
    if (num_frames_available == 0) { return; }
    
    BYTE* data = NULL;
    iarc->GetBuffer(num_frames_available, &data);
    if (data == NULL) { return; }
    
    callback(reinterpret_cast<float*>(data), num_frames_available, channels);
    
    iarc->ReleaseBuffer(num_frames_available, 0);
}
bool startAS(void* audioSys)
{
    audioSystem* sys = (audioSystem*)audioSys;
    
    if (sys->_running) { return false; }
    
    HRESULT hr = sys->_client->Start();
    if (FAILED(hr)) { return false; }
    
    sys->_running = true;
    sys->_thread = std::thread{ [sys]()
    {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

        while (sys->_running)
        {
            threadFunction(sys->_client, sys->_rc, sys->_callback, sys->_blockSize, sys->_format->Format.nChannels);
            
            // Wait
            WaitForSingleObject(sys->_event, INFINITE);
        }
    } };
    
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
    
    sys->_running = false;
    if (sys->_thread.joinable())
    {
        sys->_thread.join();
    }
    sys->_client->Stop();
    sys->_thread.~thread();
}

void setASCallback(void* audioSys, audioCallback callback)
{
    audioSystem* sys = (audioSystem*)audioSys;
    sys->_callback = callback;
}

void* getAudioSystemDevice(void* audioSys)
{
    return ((audioSystem*)audioSys)->_device;
}

void fixup_format(WAVEFORMATEXTENSIBLE* format)
{
    format->Format.nBlockAlign = format->Format.nChannels * format->Format.wBitsPerSample / 8;
    format->Format.nAvgBytesPerSec = format->Format.nSamplesPerSec * format->Format.wBitsPerSample * format->Format.nChannels / 8;
}
void* createAudioSystem(void* outputDevice, uint32_t blockSize)
{
    audioSystem* sys = new audioSystem();
    sys->_device = (audioDevice*)outputDevice;
    
    HRESULT hr;
    IMMDevice* device = sys->_device->_source;
    
    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&(sys->_client));
    if (FAILED(hr)) { return NULL; }
    
    WAVEFORMATEX* wfex = NULL;
    hr = sys->_client->GetMixFormat(&wfex);
    if (FAILED(hr)) { return NULL; }
    sys->_format = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(wfex);
    
    REFERENCE_TIME duration = (REFERENCE_TIME)((10'000'000.0 * blockSize) / sys->_format->Format.nSamplesPerSec);
    hr = sys->_client->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        duration, 0, &(sys->_format->Format), NULL);
    if (FAILED(hr)) { return NULL; }
    
    hr = sys->_client->GetService(__uuidof(IAudioRenderClient), (void**)&(sys->_rc));
    if (FAILED(hr)) { return NULL; }
    
    hr = sys->_client->GetBufferSize(&(sys->_blockSize));
    if (FAILED(hr)) { return NULL; }
    
    sys->_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = sys->_client->SetEventHandle(sys->_event);
    if (FAILED(hr)) { return NULL; }
    
    // Ensure using floating point samples
    sys->_format->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    sys->_format->Format.wBitsPerSample = sizeof(float) * 8;
    sys->_format->Samples.wValidBitsPerSample = sys->_format->Format.wBitsPerSample;
    fixup_format(sys->_format);
    
    return sys;
}

bool trySetDevName(audioDevice* dv, PROPVARIANT* property_variant, IPropertyStore* property_store, PROPERTYKEY key)
{
    HRESULT hr = property_store->GetValue(key, property_variant);
    if(FAILED(hr)) { return false; }

    std::wstring name = property_variant->pwszVal;
    
    dv->_name = {};
    int required_characters = WideCharToMultiByte(CP_UTF8, 0, name.c_str(), static_cast<int>(name.size()), NULL, 0, NULL, NULL);
    if (required_characters <= 0) { return true; }
    
    dv->_name.resize(static_cast<size_t>(required_characters));
    WideCharToMultiByte(CP_UTF8, 0, name.c_str(), static_cast<int>(name.size()), (LPSTR)dv->_name.data(), static_cast<int>(dv->_name.size()), NULL, NULL);
    return true;
}
audioDevice* createDevice(IMMDevice* source)
{
    audioDevice* device = new audioDevice();
    device->_source = source;
    
    LPWSTR device_id = nullptr;
    HRESULT hr = source->GetId(&device_id);
    if (SUCCEEDED(hr))
    {
        device->_id = device_id;
        CoTaskMemFree(device_id);
    }

    IPropertyStore* property_store = nullptr;
    auto_release<IPropertyStore> release{ property_store };

    hr = source->OpenPropertyStore(STGM_READ, &property_store);
    if (SUCCEEDED(hr))
    {
        PROPVARIANT property_variant;
        PropVariantInit(&property_variant);

        trySetDevName(device, &property_variant, property_store, PKEY_Device_FriendlyName)
            || trySetDevName(device, &property_variant, property_store, PKEY_DeviceInterface_FriendlyName)
            || trySetDevName(device, &property_variant, property_store, PKEY_Device_DeviceDesc);

        PropVariantClear(&property_variant);
    }
    
    return device;
}

const char* getDeviceName(void* device, uint32_t* size)
{
    audioDevice* ad = (audioDevice*)device;
    if (size)
    {
        *size = ad->_name.size();
    }
    return ad->_name.c_str();
}
const wchar_t* getDeviceId(void* device, uint32_t* size)
{
    audioDevice* ad = (audioDevice*)device;
    if (size)
    {
        *size = ad->_id.size();
    }
    return ad->_id.c_str();
}

bool initialise(bool captures, void** deviceCollection)
{
    deviceInitialiser* dc = new deviceInitialiser();
    *deviceCollection = dc;
    
    HRESULT hr;
    IMMDeviceEnumerator* enumerator = NULL;
    auto_release<IMMDeviceEnumerator> releaseE{ enumerator };
    
    hr = CoInitialize(NULL);
    //if (FAILED(hr)) { return false; }
    
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    if (FAILED(hr)) { return false; }
    
    // Get render devices
    IMMDeviceCollection* device_collection = NULL;
    auto_release<IMMDeviceCollection> releaseDC{ device_collection };
    hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &device_collection);
    if (FAILED(hr)) { return false; }
    
    UINT device_count = 0;
    hr = device_collection->GetCount(&device_count);
    if (FAILED(hr)) { return false; }
    
    IMMDevice* device = NULL;
    
    dc->_device_render_count = device_count;
    dc->_device_render_array = new audioDevice*[device_count];
    for (UINT i = 0; i < device_count; i++)
    {
        hr = device_collection->Item(i, &device);
        if (FAILED(hr))
        {
            if (device != NULL)
            {
                device->Release();
            }
            continue;
        }

        dc->_device_render_array[i] = createDevice(device);
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
        
        dc->_device_capture_count = device_count;
        dc->_device_capture_array = new audioDevice*[device_count];
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

            dc->_device_capture_array[i] = createDevice(device);
        }
    }
    
    // Get defaults
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    if (FAILED(hr)) { return false; }
    dc->_device_render_default = createDevice(device);
    
    if (captures)
    {
        hr = enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &device);
        if (FAILED(hr)) { return false; }
        dc->_device_capture_default = createDevice(device);
    }
    
    return true;
}

void* getDefaultOutput(void* deviceCollection)
{
    return ((deviceInitialiser*)deviceCollection)->_device_render_default;
}
void** getOutputs(void* deviceCollection, int* size)
{
    if (size)
    {
        *size = ((deviceInitialiser*)deviceCollection)->_device_render_count;
    }
    return (void**)((deviceInitialiser*)deviceCollection)->_device_render_array;
}

void* getDefaultInput(void* deviceCollection)
{
    return ((deviceInitialiser*)deviceCollection)->_device_capture_default;
}
void** getInputs(void* deviceCollection, int* size)
{
    if (size)
    {
        *size = ((deviceInitialiser*)deviceCollection)->_device_capture_count;
    }
    return (void**)((deviceInitialiser*)deviceCollection)->_device_capture_array;
}

void deleteInitialiser(void* deviceCollection)
{
    deviceInitialiser* dc = (deviceInitialiser*)deviceCollection;
    
    // Release render devices
    for (int i = 0; i < dc->_device_render_count; i++)
    {
        dc->_device_render_array[i]->_source->Release();
        delete dc->_device_render_array[i];
    }
    
    // Release capture devices
    for (int i = 0; i < dc->_device_capture_count; i++)
    {
        dc->_device_capture_array[i]->_source->Release();
        delete dc->_device_capture_array[i];
    }
    
    delete dc->_device_render_array;
    delete dc->_device_capture_array;
    
    delete dc;
}
void deleteAudioSystem(void* audioSys)
{
    audioSystem* sys = (audioSystem*)audioSys;
    
    sys->_rc->Release();
    sys->_client->Release();
    //delete sys->_format;
    
    CloseHandle(sys->_event);
    delete sys;
}


uint64_t getARSampleRate(void* audioRead)
{
    return ((audioReader*)audioRead)->_format->Format.nSamplesPerSec;
}
uint32_t getARBlockSize(void* audioRead)
{
    return ((audioReader*)audioRead)->_blockSize;
}
uint16_t getARNumChannels(void* audioRead)
{
    return ((audioReader*)audioRead)->_format->Format.nChannels;
}

void threadReadFunction(audioReader* reader)
{
    if (!reader->_buffer) { return; }
    
    IAudioCaptureClient* iacc = reader->_cc;
    HRESULT hr;
    
    UINT32 next_packet_size = 0;
    hr = iacc->GetNextPacketSize(&next_packet_size);
    if (FAILED(hr)) { return; }
    
    while (next_packet_size != 0 )
    {
        DWORD flags = 0;
        BYTE* data = NULL;
        hr = iacc->GetBuffer(&data, &next_packet_size, &flags, NULL, NULL);
        if (FAILED(hr)) { return; }
        
        UINT32 buffRead = next_packet_size * reader->_format->Format.nChannels;
        
        if (next_packet_size > reader->_buffer->_blockSize)
        {
            throw new std::overflow_error("Packet size ran over blcok size.");
        }
        
        if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
        {
            data = NULL;
        }
        
        float* block = reinterpret_cast<float*>(data);
        readBuffer* buffer = reader->_buffer;
        for (uint32_t i = 0; i < buffRead; i++)
        {
            float f = data ? block[i] : 0;
            buffer->_data[buffer->_writeIndex] = f;
            incBuffWrite(buffer);
        }
        
        hr = iacc->ReleaseBuffer(next_packet_size);
        if (FAILED(hr)) { return; }
        
        hr = iacc->GetNextPacketSize(&next_packet_size);
        if (FAILED(hr)) { return; }
    }
}
bool startAR(void* audioRead)
{
    audioReader* reader = (audioReader*)audioRead;
    
    if (reader->_running) { return false; }
    
    HRESULT hr = reader->_client->Start();
    if (FAILED(hr)) { return false; }
    
    reader->_running = true;
    
    reader->_thread = std::thread{ [reader]()
    {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

        while (reader->_running)
        {
            threadReadFunction(reader);
            
            // Wait
            WaitForSingleObject(reader->_event, INFINITE);
        }
    } };
    
    return true;
}
bool isARRunning(void* audioRead)
{
    return ((audioReader*)audioRead)->_running;
}
void stopAR(void* audioRead)
{
    audioReader* read = (audioReader*)audioRead;
    
    if (!read->_running) { return; }
    
    read->_running = false;
    if (read->_thread.joinable())
    {
        read->_thread.join();
    }
    read->_client->Stop();
    read->_thread.~thread();
}

void setARBuffer(void* audioRead, readBuffer* buffer)
{
    audioReader* read = (audioReader*)audioRead;
    read->_buffer = buffer;
}

void* getAudioReaderDevice(void* audioRead)
{
    return ((audioReader*)audioRead)->_device;
}

void* createAudioReader(void* inputDevice, uint32_t blockSize)
{
    audioReader* read = new audioReader();
    read->_device = (audioDevice*)inputDevice;
    
    HRESULT hr;
    IMMDevice* device = read->_device->_source;
    
    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&(read->_client));
    if (FAILED(hr)) { return NULL; }
    
    WAVEFORMATEX* wfex = NULL;
    hr = read->_client->GetMixFormat(&wfex);
    if (FAILED(hr)) { return NULL; }
    read->_format = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(wfex);
    
    REFERENCE_TIME duration = (REFERENCE_TIME)((10'000'000.0 * blockSize) / read->_format->Format.nSamplesPerSec);
    hr = read->_client->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        duration, 0, &(read->_format->Format), NULL);
    if (FAILED(hr)) { return NULL; }
    
    hr = read->_client->GetService(__uuidof(IAudioCaptureClient), (void**)&(read->_cc));
    if (FAILED(hr)) { return NULL; }
    
    hr = read->_client->GetBufferSize(&(read->_blockSize));
    if (FAILED(hr)) { return NULL; }
    
    read->_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = read->_client->SetEventHandle(read->_event);
    if (FAILED(hr)) { return NULL; }
    
    // Ensure using floating point samples
    read->_format->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    read->_format->Format.wBitsPerSample = sizeof(float) * 8;
    read->_format->Samples.wValidBitsPerSample = read->_format->Format.wBitsPerSample;
    fixup_format(read->_format);
    
    return read;
}
void deleteAudioReader(void* audioRead)
{
    audioReader* read = (audioReader*)audioRead;
    
    read->_cc->Release();
    read->_client->Release();
    //delete sys->_format;
    
    CloseHandle(read->_event);
    delete read;
}

float* readAudioSource(void* audioRead, uint32_t size, uint32_t* sizeOut)
{
    audioReader* reader = (audioReader*)audioRead;
    
    IAudioCaptureClient* iacc = reader->_cc;
    HRESULT hr;
    /*
    UINT32 next_packet_size = 0;
    hr = iacc->GetNextPacketSize(&next_packet_size);
    if (FAILED(hr)) { return NULL; }
    
    if (next_packet_size < size)
    {
        *sizeOut = next_packet_size;
    }
    else
    {
        *sizeOut = size;
    }*/
    
    *sizeOut = size;
    
    DWORD flags = 0;
    BYTE* data = NULL;
    hr = iacc->GetBuffer(&data, sizeOut, &flags, NULL, NULL);
    if (FAILED(hr)) { return NULL; }
    
    UINT32 buffRead = *sizeOut * reader->_format->Format.nChannels;
    
    if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
    {
        data = NULL;
    }
    if (data == NULL)
    {
        *sizeOut = size;
    }
    
    float* block = reinterpret_cast<float*>(data);
    float* ret = new float[buffRead];
    for (uint32_t i = 0; i < buffRead; i++)
    {
        float f = data ? block[i] : 0;
        ret[i] = f;
        //incBuffWrite(buffer);
    }
    
    hr = iacc->ReleaseBuffer(*sizeOut);
    if (FAILED(hr)) { return NULL; }
    
    return ret;
}
