#pragma once

#include "../common/include/shader.h"

class LightingShader final : public ShaderGL
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

    LightingShader() = default;
    ~LightingShader() override = default;

    LightingShader(const LightingShader&) = delete;
    LightingShader(const LightingShader&&) = delete;
    LightingShader& operator=(const LightingShader&) = delete;
    LightingShader& operator=(const LightingShader&&) = delete;
};