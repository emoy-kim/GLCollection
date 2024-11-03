#pragma once

#include "../common/include/shader.h"

class ShadowShader final : public ShaderGL
{
public:
    enum UNIFORM
    {
        WorldMatrix = 0,
        ViewMatrix,
        ModelViewProjectionMatrix,
        LightViewProjectionMatrix,
        Lights,
        Material = 292,
        UseTexture = 297,
        UseLight,
        LightIndex,
        GlobalAmbient
    };

    ShadowShader() = default;
    ~ShadowShader() override = default;

    ShadowShader(const ShadowShader&) = delete;
    ShadowShader(const ShadowShader&&) = delete;
    ShadowShader& operator=(const ShadowShader&) = delete;
    ShadowShader& operator=(const ShadowShader&&) = delete;
};

class SimpleShader final : public ShaderGL
{
public:
    enum UNIFORM { ModelViewProjectionMatrix = 0 };

    SimpleShader() = default;
    ~SimpleShader() override = default;

    SimpleShader(const SimpleShader&) = delete;
    SimpleShader(const SimpleShader&&) = delete;
    SimpleShader& operator=(const SimpleShader&) = delete;
    SimpleShader& operator=(const SimpleShader&&) = delete;
};