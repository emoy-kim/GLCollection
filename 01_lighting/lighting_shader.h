#pragma once

#include "../common/include/shader.h"

class LightingShaderGL final : public ShaderGL
{
public:
    enum UNIFORM
    {
        WorldMatrix = 0,
        ViewMatrix,
        ModelViewProjectionMatrix,
        Lights,
        Material = 291,
        UseTexture = 296,
        UseLight,
        LightNum,
        GlobalAmbient
    };

    LightingShaderGL() = default;
    ~LightingShaderGL() override = default;

    LightingShaderGL(const LightingShaderGL&) = delete;
    LightingShaderGL(const LightingShaderGL&&) = delete;
    LightingShaderGL& operator=(const LightingShaderGL&) = delete;
    LightingShaderGL& operator=(const LightingShaderGL&&) = delete;
};