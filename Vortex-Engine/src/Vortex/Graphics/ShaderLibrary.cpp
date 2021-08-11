//
// Created by Vitriol1744 on 10.08.2021.
//
#include "ShaderLibrary.hpp"

namespace Vortex::Graphics
{
    Ref<IShader> ShaderLibrary::CreateShader(strview name, strview vertexPath, strview pixelPath, bool precompiled)
    {
        Ref<IShader> result = IShader::Create(name, vertexPath, pixelPath, precompiled);
        shaders[result->GetName().stringID] = result;

        return result;
    }
    Ref<IShader> ShaderLibrary::CreateShader(strview vertexPath, strview pixelPath, bool precompiled)
    {
        Ref<IShader> result = IShader::Create(vertexPath, pixelPath, precompiled);
        shaders[result->GetName().stringID] = result;

        return result;
    }

    void ShaderLibrary::AddShader(strview name, Ref<IShader> shader)
    {
        shaders[VT_HASH(name.data())] = shader;
    }
    void ShaderLibrary::AddShader(Ref<IShader> shader)
    {
        AddShader(shader->GetName().string, shader);
    }

    Ref<IShader> ShaderLibrary::GetShader(strview name)
    {
        HashedString hashedName(name);
        if (shaders.find(hashedName.stringID) != shaders.end()) return shaders[hashedName.stringID];

        VTCoreLogWarn("Could not find shader with name: {}!", name);
        return nullptr;
    }
}
