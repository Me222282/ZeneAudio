#include <stdio.h>
#include <stdint.h>
#include <stdexcept>

#include "readBuffer.h"

typedef void (*audioCallback)(float*, uint32_t, uint32_t);

uint64_t getASSampleRate(void* audioSys);
uint32_t getASBlockSize(void* audioSys);
uint16_t getASNumChannels(void* audioSys);

bool startAS(void* audioSys);
bool isASRunning(void* audioSys);
void stopAS(void* audioSys);

void setASCallback(void* audioSys, audioCallback callback);

void* getAudioSystemDevice(void* audioSys);
void* createAudioSystem(void* outputDevice, uint32_t blockSize);
void deleteAudioSystem(void* audioSys);

std::string getDeviceName(void* device);
std::wstring getDeviceId(void* device);

bool initialise(bool captures, void** deviceCollection);

void* getDefaultOutput(void* deviceCollection);
void** getOutputs(void* deviceCollection, int* size);

void* getDefaultInput(void* deviceCollection);
void** getInputs(void* deviceCollection, int* size);

void deleteInitialiser(void* deviceCollection);


uint64_t getARSampleRate(void* audioRead);
uint32_t getARBlockSize(void* audioRead);
uint16_t getARNumChannels(void* audioRead);

bool startAR(void* audioRead);
bool isARRunning(void* audioRead);
void stopAR(void* audioRead);

void setARBuffer(void* audioRead, readBuffer* buffer);

float* readAudioSource(void* audioRead, uint32_t size, uint32_t* sizeOut);

void* getAudioReaderDevice(void* audioRead);
void* createAudioReader(void* inputDevice, uint32_t blockSize);
void deleteAudioReader(void* audioRead);