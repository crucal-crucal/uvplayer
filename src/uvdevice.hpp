#pragma once

#include <QCameraInfo>
#include <QCoreApplication>
#include <QAudioDeviceInfo>

using CameraList = QList<QCameraInfo>;
using AudioInputList = QList<QAudioDeviceInfo>;
using AudioOutputList = QList<QAudioDeviceInfo>;


class CUVDevice {
	Q_DISABLE_COPY_MOVE(CUVDevice)

public:
	explicit CUVDevice();
	~CUVDevice();

	static CameraList GetCameraList();
	static AudioInputList GetAudioInputList();
	static AudioOutputList GetAudioOutputList();
};

inline CameraList CUVDevice::GetCameraList() {
	return QCameraInfo::availableCameras();
}

inline AudioInputList CUVDevice::GetAudioInputList() {
	return QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
}

inline AudioOutputList CUVDevice::GetAudioOutputList() {
	return QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
}
