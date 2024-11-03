#pragma once

#include "../common/include/shader.h"

class ShadowShaderGL final : public ShaderGL
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

    ShadowShaderGL() = default;
    ~ShadowShaderGL() override = default;

    ShadowShaderGL(const ShadowShaderGL&) = delete;
    ShadowShaderGL(const ShadowShaderGL&&) = delete;
    ShadowShaderGL& operator=(const ShadowShaderGL&) = delete;
    ShadowShaderGL& operator=(const ShadowShaderGL&&) = delete;
};

class SimpleShaderGL final : public ShaderGL
{
public:
    enum UNIFORM { ModelViewProjectionMatrix = 0 };

    SimpleShaderGL() = default;
    ~SimpleShaderGL() override = default;

    SimpleShaderGL(const SimpleShaderGL&) = delete;
    SimpleShaderGL(const SimpleShaderGL&&) = delete;
    SimpleShaderGL& operator=(const SimpleShaderGL&) = delete;
    SimpleShaderGL& operator=(const SimpleShaderGL&&) = delete;
};