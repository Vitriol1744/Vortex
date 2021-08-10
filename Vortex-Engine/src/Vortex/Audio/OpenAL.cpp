//
// Created by vitriol1744 on 8/4/21.
//
#include "OpenAL.hpp"

#include "AL/al.h"
#include "AL/alc.h"

#define AL_LOG_ERROR(errorcode) \
    VTCoreLogError("OpenAL: ErrorCode: {}\n\r Function: {}\n\r File: {}\n\r Line: {}", errorcode, funcname, filename, line)

namespace Vortex::Internal
{
    ALvoid alClearError()
    {
        while (alGetError() != AL_NO_ERROR);
    }
    ALboolean alLogCall(std::string_view funcname, std::string_view filename, uint32 line)
    {
        bool result = true;
        while (ALenum error = alGetError())
        {
            result = false;
            switch (error)
            {
                case AL_INVALID_NAME:
                    AL_LOG_ERROR("AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function!");
                    break;
                case AL_INVALID_ENUM:
                    AL_LOG_ERROR("AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function!");
                    break;
                case AL_INVALID_VALUE:
                    AL_LOG_ERROR("AL_INVALID_VALUE: an invalid value was passed to an OpenAL function!");
                    break;
                case AL_INVALID_OPERATION:
                    AL_LOG_ERROR("AL_INVALID_OPERATION: the requested operation is not valid!");
                    break;
                case AL_OUT_OF_MEMORY:
                    AL_LOG_ERROR("AL_INVALID_MEMORY: the requested operation resulted in OpenAL running out of memory!");
                    break;

                default:
                    AL_LOG_ERROR("AL_UNKNOWN_ERROR!");
                    break;
            }
        }

        return result;
    }

    ALCvoid alcClearError(ALCdevice* device)
    {
        while (alcGetError(device) != ALC_NO_ERROR);
    }
    ALCboolean alcLogCall(ALCdevice* device, std::string_view funcname, std::string_view filename, ALCuint line)
    {
        bool result = true;
        while (ALCenum error = alcGetError(device))
        {
            result = false;
            switch (error)
            {
                case ALC_INVALID_DEVICE:
                    AL_LOG_ERROR("ALC_INVALID_DEVICE: A bad device was passed to an OpenAL function!");
                    break;
                case ALC_INVALID_CONTEXT:
                    AL_LOG_ERROR("ALC_INVALID_CONTEXT: A bad context was passed to an OpenAL function!");
                    break;
                case ALC_INVALID_ENUM:
                    AL_LOG_ERROR("ALC_INVALID_ENUM: An unknown enum value was passed to an OpenAL function!");
                    break;
                case ALC_INVALID_VALUE:
                    AL_LOG_ERROR("ALC_INVALID_VALUE: An invalid value was passed to an OpenAL function!");
                    break;
                case ALC_OUT_OF_MEMORY:
                    AL_LOG_ERROR("ALC_OUT_OF_MEMORY: The requested operation resulted in OpenAL running out of memory!");
                    break;

                default:
                    AL_LOG_ERROR("ALC_UNKNOWN_ERROR!");
                    break;
            }
        }

        return result;
    }
}