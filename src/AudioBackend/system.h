#include <stdio.h>
#include <stdexcept>

typedef void (*audioCallback)(float*, uint32_t, uint32_t, uint32_t);

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

void deleteInitialiser(void* deviceCollection);