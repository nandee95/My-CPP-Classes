#pragma once

#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>

#include <al.h>
#include <alc.h>

template<typename T = int16_t>
class AudioCapture
{
public:
	class Exception
	{
	public:
		enum Type
		{
			FailedToInitDevice,
			FailedToStartCapture,
			FailedToStopCapture,
			CaptureIsInProgress,
			CaptureIsNotInProgress
		};
		const std::string message;
		const Type type;
		Exception(Type type, const std::string message) : type(type), message(message)
		{

		}
	};

	typedef std::function<void(T*, const size_t&)> Callback;
protected:
	ALenum error = NULL;
	std::thread thread;
	Callback callback;
	bool capturing = false;

	const size_t bufferSize = 0;
	const uint32_t freq;
	const bool stereo = false;
	const std::string deviceName;

	ALCdevice* device;

	enum ThreadStatus
	{
		Thread_Waiting,
		Thread_Initializing,
		Thread_FailedToInitDevice,
		Thread_FailedToStartCapture,
		Thread_Done
	};

	ThreadStatus threadStatus;
public:
	Capture(const std::string deviceName, const uint32_t freq, const uint32_t bufferSize, const bool stereo = false) : deviceName(deviceName), freq(freq), bufferSize(bufferSize), stereo(stereo)
	{
		static_assert(std::is_same<T, int8_t>() || std::is_same<T, int16_t>(), "[class Capture] Unknown template type (Supported: int8_t, int16_t)");
	}

	~Capture()
	{
		if (capturing) Stop();
	}

	const void SetCallback(Callback function)
	{
		callback = function;
	}

	const void Start()
	{
		if (capturing) throw Exception(Exception::CaptureIsInProgress, "Failed to start capture, capture is in progress");

		capturing = true;
		threadStatus = Thread_Waiting;
		thread = std::thread(&Capture::CaptureThread, this);

		while (threadStatus <= Thread_Initializing)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		switch (threadStatus)
		{
		case Thread_FailedToInitDevice:
			throw Exception(Exception::FailedToInitDevice, "Failed to init device");
		case Thread_FailedToStartCapture:
			throw Exception(Exception::FailedToStartCapture, "Failed to init device");
		}
	}
	const void Stop()
	{
		if (!capturing) throw Exception(Exception::CaptureIsNotInProgress, "Failed to stop capture, capture is not in progress");

		capturing = false;
		if (thread.joinable()) thread.join();
	}

	const bool& isCapturing() const
	{
		return capturing;
	}

protected:
	virtual void CaptureThread()
	{
		device = alcCaptureOpenDevice(deviceName.c_str(), freq, stereo ? (std::is_same<T, int8_t>() ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16) : (std::is_same<T, int8_t>() ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16), bufferSize);

		if (alcGetError(device) != ALC_NO_ERROR)
		{
			capturing = false;
			threadStatus = Thread_FailedToInitDevice;
			return;
		}
		alcCaptureStart(device);

		if (alcGetError(device) != ALC_NO_ERROR)
		{
			capturing = false;
			threadStatus = Thread_FailedToInitDevice;
			alcCaptureCloseDevice(device);
			return;
		}
		ALCint samples;
		T* buffer = new T[bufferSize];
		threadStatus = Thread_Done;
		while (capturing)
		{
			alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, 1, &samples);
			if (samples >= bufferSize)
			{
				alcCaptureSamples(device, buffer, bufferSize);
				callback(buffer, bufferSize);
			}
		}
		alcCaptureStop(device);
		alcCaptureCloseDevice(device);
	}

public: //Static functions
	static std::vector<std::string> GetCaptureDevices()
	{
		const ALCchar* devices = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
		std::vector<std::string> result;
		int i = 0;
		int start = 0;
		while (devices[i] != '\0')
		{
			while (devices[i++] != '\0') {}
			result.push_back(std::string(devices + start));
			start = i;
		}
		return result;
	}
};