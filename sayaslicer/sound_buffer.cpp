#include <filesystem>
#include <sndfile.h>
#include <stdexcept>
#include <cstring>

#include "sound_buffer.hpp"

struct callbackData {
	unsigned long long samplePos;
	unsigned long long length;
	unsigned long long currentPos;
	std::vector<float>* buffer;
	SoundBuffer *sound;
};

void callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	float* out = (float*)pOutput;
	struct callbackData* callbackData = (struct callbackData*)pDevice->pUserData;
	unsigned int channelCount = callbackData->sound->getChannelCount();

	memset(out, 0, sizeof(float) * frameCount * channelCount);
	float* buffer;
	if (!callbackData->buffer)
		buffer = callbackData->sound->getSamples().data();
	else
		buffer = callbackData->buffer->data();

	if (callbackData->currentPos >= callbackData->length) {
		callbackData->sound->updatePlayProgress(callbackData->length);
		callbackData->sound->setPlaying(false);
		return;
	}

	unsigned long long numSamplesToCopy = frameCount * channelCount;
	bool complete = false;
	if (callbackData->currentPos + numSamplesToCopy >= callbackData->length) {
		numSamplesToCopy = callbackData->length - callbackData->currentPos;
		complete = true;
	}

	memcpy(out, &buffer[callbackData->samplePos + callbackData->currentPos], numSamplesToCopy * sizeof(float));

	callbackData->currentPos += numSamplesToCopy;
	callbackData->sound->updatePlayProgress(callbackData->currentPos);

	if (complete) {
		callbackData->sound->setPlaying(false);
		return;
	}
	
	(void)pInput;
}

void notificationCallback(const ma_device_notification* pNotification) {
	if (pNotification->type == ma_device_notification_type_stopped) {
		struct callbackData* callbackData = (struct callbackData*)pNotification->pDevice->pUserData;
		if (callbackData->buffer)
			delete callbackData->buffer;
		callbackData->sound->setPlaying(false);
		free(callbackData);
	}
}

SoundBuffer::~SoundBuffer() {
	ma_device_uninit(&stream);
}

unsigned int SoundBuffer::getChannelCount() {
	return this->channelCount;
}

unsigned int SoundBuffer::getSampleRate() {
	return this->sampleRate;
}

float SoundBuffer::getDuration() {
	return this->duration;
}

std::vector<float>& SoundBuffer::getSamples() {
	return this->buffer;
}

unsigned long long SoundBuffer::getSampleCount() {
	return this->buffer.size();
}

int SoundBuffer::loadFromFile(std::string path) {
	auto fp = std::filesystem::u8path(path);
	if (!std::filesystem::exists(fp))
		return -1;

	SF_INFO info = { 0 };

#if _WIN32
	SNDFILE *file = sf_wchar_open(fp.wstring().c_str(), SFM_READ, &info);
#else
	SNDFILE *file = sf_open(fp.u8string().c_str(), SFM_READ, &info);
#endif
	if (sf_error(file) != SF_ERR_NO_ERROR)
		return -2;

	ma_device_uninit(&stream);
	this->playing = false;

	this->channelCount = info.channels;
	this->sampleRate = info.samplerate;
	this->duration = (float)info.frames / (float)info.samplerate;

	this->buffer.clear();
	this->buffer.shrink_to_fit();
	this->buffer.reserve(info.frames * info.channels);

	size_t totalFramesRead = 0;
	float* tmp = (float*)malloc(sizeof(float) * 4096 * info.channels);

	while (totalFramesRead < info.frames) {
		size_t framesRead = sf_readf_float(file, tmp, 4096);
		if (framesRead == 0)
			break;
		totalFramesRead += framesRead;
		for (size_t i = 0; i < framesRead * info.channels; i++)
			this->buffer.push_back(tmp[i]);
	}

	free(tmp);
	sf_close(file);

	return 0;
}

bool SoundBuffer::writeFile(std::filesystem::path path, unsigned int sampleRate, unsigned int channelCount, float *buffer, size_t bufSize) {
	SF_INFO info = { 0 };
	info.samplerate = sampleRate;
	info.channels = channelCount;
	info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

#if _WIN32
	SNDFILE* file = sf_wchar_open(path.wstring().c_str(), SFM_WRITE, &info);
#else
	SNDFILE* file = sf_open(path.u8string().c_str(), SFM_WRITE, &info);
#endif
	if (sf_error(file) != SF_ERR_NO_ERROR)
		return false;

	if (sf_write_float(file, buffer, bufSize) != bufSize) {
		// Buffer wasn't entirely written and I can't bother to implement the loop yet (it shouldn't happen)
		return false;
	}
	sf_close(file);

	return true;
}

void SoundBuffer::play(unsigned long long samplePos, unsigned long long length, const float* buffer) {
	ma_device_uninit(&stream);
	playing = false;
	
	struct callbackData* data = (struct callbackData*)calloc(1, sizeof(struct callbackData));
	if (!data)
		throw std::runtime_error("Cannot allocate miniaudio user data (this should not happen)");
	data->samplePos = samplePos;
	data->length = length;
	data->currentPos = 0;
	data->sound = this;

	if (buffer)
		data->buffer = new std::vector<float>(buffer, buffer + length);

	ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format = ma_format_f32;
	deviceConfig.playback.channels = channelCount;
	deviceConfig.sampleRate = sampleRate;
	deviceConfig.dataCallback = callback;
	deviceConfig.notificationCallback = notificationCallback;
	deviceConfig.pUserData = data;

	if (ma_device_init(NULL, &deviceConfig, &stream) != MA_SUCCESS) {
		throw std::runtime_error("Error opening miniaudio device");
	}

	if (ma_device_start(&stream) != MA_SUCCESS) {
		throw std::runtime_error("Error starting miniaudio device");
	}

	playing = true;
}

void SoundBuffer::play(unsigned long long samplePos, unsigned long long length) {
	this->play(samplePos, length, nullptr);
}

void SoundBuffer::play(std::vector<float>& buffer) {
	this->play(0, buffer.size(), buffer.data());
}

bool SoundBuffer::isPlaying() {
	return ma_device_is_started(&stream) && playing;
}

void SoundBuffer::stop() {
	ma_device_stop(&stream);
	playing = false;
}

void SoundBuffer::setStartPlayPos(unsigned long long value) {
	this->startPlayPos = value;
	this->relativePlayPos = 0;
}

void SoundBuffer::updatePlayProgress(unsigned long long value) {
	this->relativePlayPos = value;
}

void SoundBuffer::setPlaying(bool value) {
	this->playing = value;
}

unsigned long long SoundBuffer::getLastKnownPlayPos() {
	return this->startPlayPos + this->relativePlayPos;
}

