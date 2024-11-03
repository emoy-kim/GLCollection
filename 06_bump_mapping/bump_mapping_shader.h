#pragma once

#include "../common/include/shader.h"

class BumpMappingShaderGL final : public ShaderGL
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

    BumpMappingShaderGL() = default;
    ~BumpMappingShaderGL() override = default;

    BumpMappingShaderGL(const BumpMappingShaderGL&) = delete;
    BumpMappingShaderGL(const BumpMappingShaderGL&&) = delete;
    BumpMappingShaderGL& operator=(const BumpMappingShaderGL&) = delete;
    BumpMappingShaderGL& operator=(const BumpMappingShaderGL&&) = delete;
};

class BoxBlurShaderGL final : public ShaderGL
{
public:
    enum UNIFORM { IsHorizontal = 0, BlurRadius };

    BoxBlurShaderGL() = default;
    ~BoxBlurShaderGL() override = default;

    BoxBlurShaderGL(const BoxBlurShaderGL&) = delete;
    BoxBlurShaderGL(const BoxBlurShaderGL&&) = delete;
    BoxBlurShaderGL& operator=(const BoxBlurShaderGL&) = delete;
    BoxBlurShaderGL& operator=(const BoxBlurShaderGL&&) = delete;
};