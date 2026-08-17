#pragma once

#include <vector>
#include <string>
#include <miniaudio.h>

class SoundBuffer {
private:
	std::vector<float> buffer;
	unsigned int channelCount = 0;
	unsigned int sampleRate = 0;
	float duration = 0.0f;
	ma_device stream = { 0 };
	unsigned long long startPlayPos = 0;
	unsigned long long relativePlayPos = 0;
	bool playing = false;

	void play(unsigned long long samplePos, unsigned long long length, const float* buffer);

public:

	~SoundBuffer();

	float getDuration();
	unsigned int getChannelCount();
	unsigned int getSampleRate();
	std::vector<float>& getSamples();
	unsigned long long getSampleCount();
	int loadFromFile(std::string path);
	void play(unsigned long long samplePos, unsigned long long length);
	void play(std::vector<float>& buffer);
	bool isPlaying();
	void setPlaying(bool value);
	void stop();
	void setStartPlayPos(unsigned long long value);
	void updatePlayProgress(unsigned long long value);
	unsigned long long getLastKnownPlayPos();

	static bool writeFile(std::filesystem::path path, unsigned int sampleRate, unsigned int channelCount, float *buffer, size_t bufSize);
};