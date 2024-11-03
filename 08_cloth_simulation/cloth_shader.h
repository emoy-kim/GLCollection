#pragma once

#include "../common/include/shader.h"

class ClothShaderGL final : public ShaderGL
{
public:
    enum UNIFORM
    {
        SpringRestLength = 0,
        SpringStiffness,
        SpringDamping,
        ShearStiffness,
        ShearDamping,
        FlexionStiffness,
        FlexionDamping,
        DeltaTime,
        Mass,
        ClothPointNumSize,
        ClothWorldMatrix,
        SphereRadius,
        SpherePosition,
        SphereWorldMatrix
    };

    ClothShaderGL() = default;
    ~ClothShaderGL() override = default;

    ClothShaderGL(const ClothShaderGL&) = delete;
    ClothShaderGL(const ClothShaderGL&&) = delete;
    ClothShaderGL& operator=(const ClothShaderGL&) = delete;
    ClothShaderGL& operator=(const ClothShaderGL&&) = delete;
};