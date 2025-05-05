#pragma once

#include <vector>
#include <string>
#include <portaudio.h>

class SoundBuffer {
private:
	std::vector<float> buffer;
	unsigned int channelCount = 0;
	unsigned int sampleRate = 0;
	float duration = 0.0f;
	PaStream* stream = NULL;

public:
	~SoundBuffer();

	float getDuration();
	unsigned int getChannelCount();
	unsigned int getSampleRate();
	std::vector<float>& getSamples();
	unsigned long long getSampleCount();
	int loadFromFile(std::string path);
	void play(unsigned long long samplePos, unsigned long long length);
	bool isPlaying();
	void stop();

	static bool writeFile(std::filesystem::path path, unsigned int sampleRate, unsigned int channelCount, float *buffer, size_t bufSize);
};
