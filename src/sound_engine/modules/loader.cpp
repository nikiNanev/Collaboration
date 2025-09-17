#include "sound_engine/modules/loader.h"

// Basic WAV loader for uncompressed mono WAVs
bool Loader::loadWAVFile(const std::string &filename, std::vector<char> &data,
				 ALenum &format, ALsizei &freq)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file)
	{
		std::cerr << "Failed to open WAV file.\n";
		return false;
	}

	char riff[4];
	file.read(riff, 4); // "RIFF"
	file.ignore(4);		// Chunk size
	char wave[4];
	file.read(wave, 4); // "WAVE"

	// Read chunks
	char chunkId[4];
	int chunkSize;
	short audioFormat, numChannels, bitsPerSample;
	int sampleRate;

	while (file.read(chunkId, 4))
	{
		file.read(reinterpret_cast<char *>(&chunkSize), 4);

		if (std::strncmp(chunkId, "fmt ", 4) == 0)
		{
			file.read(reinterpret_cast<char *>(&audioFormat), 2);
			file.read(reinterpret_cast<char *>(&numChannels), 2);
			file.read(reinterpret_cast<char *>(&sampleRate), 4);
			file.ignore(6); // Byte rate + block align
			file.read(reinterpret_cast<char *>(&bitsPerSample), 2);
			file.ignore(chunkSize - 16); // Skip the rest
		}
		else if (std::strncmp(chunkId, "data", 4) == 0)
		{
			data.resize(chunkSize);
			file.read(data.data(), chunkSize);
			break;
		}
		else
		{
			file.ignore(chunkSize);
		}
	}

	freq = sampleRate;
	if (numChannels == 1)
	{
		format = (bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
	}
	else if (numChannels == 2)
	{
		format = (bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
	}
	else
	{
		std::cerr << "Unsupported channel count: " << numChannels << "\n";
		return false;
	}

	return true;
}

int Loader::sound(const std::string &filename)
{
	ALCdevice *device = alcOpenDevice(nullptr);
	if (!device)
	{
		std::cerr << "Failed to open device.\n";
		return -1;
	}

	ALCcontext *context = alcCreateContext(device, nullptr);
	if (!context || !alcMakeContextCurrent(context))
	{
		std::cerr << "Failed to set OpenAL context.\n";
		alcCloseDevice(device);
		return -1;
	}

	// Load sound
	std::vector<char> bufferData;
	ALenum format;
	ALsizei freq;
	if (!loadWAVFile(filename, bufferData, format, freq))
	{
		std::cerr << "Failed to load WAV.\n";
		alcDestroyContext(context);
		alcCloseDevice(device);
		return -1;
	}

	// Generate buffer
	ALuint buffer;
	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, bufferData.data(), bufferData.size(), freq);

	// Generate source
	ALuint source;
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);

	if (this->sound_state)
	{
		alSourcePlay(source);
	}
	else
	{
		alSourceStop(source);
		return 0;
	}

	std::cout << "Playing sound...\n";

	// Wait until done
	ALint state;
	do
	{
		alGetSourcei(source, AL_SOURCE_STATE, &state);
	} while (state == AL_PLAYING);
	

	std::cout << "Done.\n";

	this->sound_state = false;

	// Cleanup
	alDeleteSources(1, &source);
	alDeleteBuffers(1, &buffer);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);
	
	return 0;
}

bool Loader::getState()
{
	return this->sound_state;
}

void Loader::setState(bool state)
{
	this->sound_state = state;
}


