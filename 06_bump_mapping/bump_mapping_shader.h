#pragma once

#include "../common/include/shader.h"

class BumpMappingShader final : public ShaderGL
{
public:
    enum UNIFORM
    {
        WorldMatrix = 0,
        ViewMatrix,
        ModelViewProjectionMatrix,
        Lights,
        Material = 291,
        UseBumpMapping = 296,
        LightNum,
        GlobalAmbient
    };

    BumpMappingShader() = default;
    ~BumpMappingShader() override = default;

    BumpMappingShader(const BumpMappingShader&) = delete;
    BumpMappingShader(const BumpMappingShader&&) = delete;
    BumpMappingShader& operator=(const BumpMappingShader&) = delete;
    BumpMappingShader& operator=(const BumpMappingShader&&) = delete;
};

class BoxBlurShader final : public ShaderGL
{
public:
    enum UNIFORM { IsHorizontal = 0, BlurRadius };

    BoxBlurShader() = default;
    ~BoxBlurShader() override = default;

    BoxBlurShader(const BoxBlurShader&) = delete;
    BoxBlurShader(const BoxBlurShader&&) = delete;
    BoxBlurShader& operator=(const BoxBlurShader&) = delete;
    BoxBlurShader& operator=(const BoxBlurShader&&) = delete;
};