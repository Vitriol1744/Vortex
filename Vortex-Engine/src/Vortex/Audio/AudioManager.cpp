//
// Created by Vitriol1744 on 06.08.2021.
//
#include "AudioManager.hpp"

#include "AL/al.h"
#include "AL/alc.h"

namespace Vortex::Audio
{
    struct Device
    {
        Device() = default;
        inline Device(ALCdevice* device, ALCcontext* context, AudioDevice id, std::string name)
            : device(device), context(context), id(id), name(name) { }

        inline ~Device()
        {
            if (context) alcDestroyContext(context);
            if (device) alcCloseDevice(device);
            VTCoreLogInfo("Device Destroyed!");
        }

        ALCdevice* device;
        ALCcontext* context;
        AudioDevice id;
        std::string name;
    };

    static std::vector<Device> devices;
    static Device defaultDevice;
    static Device* boundDevice;
    static constexpr const AudioDevice defaultDeviceIndex = 3;

    static AudioDevice GetUniqueID()
    {
        static AudioDevice id = 9;

        return id++;
    }

    ALCvoid AudioManager::Initialize()
    {
        alcCall(nullptr, ALCdevice* device = alcOpenDevice(nullptr));
        alcCall(device, ALCcontext* context = alcCreateContext(device, nullptr));
        alcCall(device, alcMakeContextCurrent(context));

        defaultDevice.device = device;
        defaultDevice.context = context;
        defaultDevice.id = defaultDeviceIndex;
        defaultDevice.name = "";

        VTCoreLogInfo("Version: {}", alGetString(AL_VERSION));
        VTCoreLogInfo("Vendor: {}", alGetString(AL_VENDOR));
        VTCoreLogInfo("Renderer: {}", alGetString(AL_RENDERER));
        VTCoreLogInfo("Extensions: {}", alGetString(AL_EXTENSIONS));
    }

    ALCvoid AudioManager::Shutdown()
    {
        while (!devices.empty()) devices.pop_back();
        if (defaultDevice.context) alcDestroyContext(defaultDevice.context);
        if (defaultDevice.device) alcCloseDevice(defaultDevice.device);
    }

    std::vector<std::string> AudioManager::GetAudioDevices()
    {
        std::vector<std::string> result;
        alcCall(nullptr, const ALCchar* devices = alcGetString(nullptr, ALC_DEVICE_SPECIFIER));

        do
        {
            result.push_back(std::string(devices));
            devices += result.back().size() + 1;
        }
        while (*(devices + 1) != '\0');

        return result;
    }
    ALuint AudioManager::AddAudioDevice(std::string_view name)
    {
        alcCall(nullptr, ALCdevice* device = alcOpenDevice(nullptr));
        alcCall(device, ALCcontext* context = alcCreateContext(device, nullptr));
        alcCall(device, alcMakeContextCurrent(context));

        devices.emplace_back(device, context, GetUniqueID(), std::string(name));
        return devices.back().id;
    }
    ALCvoid AudioManager::RemoveAudioDevice(std::string_view name)
    {
        std::remove_if(devices.begin(), devices.end(), [name](Device& device){ return device.name == name; });
    }
    ALCvoid AudioManager::RemoveAudioDevice(AudioDevice deviceID)
    {
        std::remove_if(devices.begin(), devices.end(), [deviceID](Device& device){ return device.id == deviceID; });
    }

    ALCvoid AudioManager::SetAudioDevice(std::string_view name)
    {
        if (name.empty())
        {
            alcCall(nullptr, bool result = alcMakeContextCurrent(defaultDevice.context));
            VT_CORE_ASSERT_MSG(result, "Failed to make context current!");
            return;
        }

        auto it = std::find_if(devices.begin(), devices.end(), [name](Device& device) { return device.name == name; });
        alcMakeContextCurrent(it->context);
    }

    ALCvoid AudioManager::SetAudioDevice(AudioDevice deviceID)
    {
        if (deviceID == defaultDeviceIndex)
        {
            if (!alcMakeContextCurrent(defaultDevice.context)) VTCoreLogError("Failed to make context current!");
            return;
        }

        auto it = std::find_if(devices.begin(), devices.end(), [deviceID](Device& device) { return device.id == deviceID; });
        alcMakeContextCurrent(it->context);
    }
}