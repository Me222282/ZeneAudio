#include <stdio.h>
#include <stdint.h>
#include <stdexcept>

#include "readBuffer.h"

#define DllExport   __declspec( dllexport )

extern "C" {

typedef void (*audioCallback)(float*, uint32_t, uint32_t);

DllExport uint64_t getASSampleRate(void* audioSys);
DllExport uint32_t getASBlockSize(void* audioSys);
DllExport uint16_t getASNumChannels(void* audioSys);

DllExport bool startAS(void* audioSys);
DllExport bool isASRunning(void* audioSys);
DllExport void stopAS(void* audioSys);

DllExport void setASCallback(void* audioSys, audioCallback callback);

DllExport void* getAudioSystemDevice(void* audioSys);
DllExport void* createAudioSystem(void* outputDevice, uint32_t blockSize);
DllExport void deleteAudioSystem(void* audioSys);

DllExport const char* getDeviceName(void* device, uint32_t* size);
DllExport const wchar_t* getDeviceId(void* device, uint32_t* size);

DllExport bool initialise(bool captures, void** deviceCollection);

DllExport void* getDefaultOutput(void* deviceCollection);
DllExport void** getOutputs(void* deviceCollection, int* size);

DllExport void* getDefaultInput(void* deviceCollection);
DllExport void** getInputs(void* deviceCollection, int* size);

DllExport void deleteInitialiser(void* deviceCollection);


DllExport uint64_t getARSampleRate(void* audioRead);
DllExport uint32_t getARBlockSize(void* audioRead);
DllExport uint16_t getARNumChannels(void* audioRead);

DllExport bool startAR(void* audioRead);
DllExport bool isARRunning(void* audioRead);
DllExport void stopAR(void* audioRead);

DllExport void setARBuffer(void* audioRead, readBuffer* buffer);

DllExport float* readAudioSource(void* audioRead, uint32_t size, uint32_t* sizeOut);

DllExport void* getAudioReaderDevice(void* audioRead);
DllExport void* createAudioReader(void* inputDevice, uint32_t blockSize);
DllExport void deleteAudioReader(void* audioRead);

}