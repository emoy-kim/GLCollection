#pragma once

#include "../common/include/shader.h"

class ClothShader final : public ShaderGL
{
public:
    enum UNIFORM
    {
        SpringRestLength = 0,
        SpringStiffness,
        SpringDamping,
        ShearRestLength,
        ShearStiffness,
        ShearDamping,
        FlexionRestLength,
        FlexionStiffness,
        FlexionDamping,
        GravityDamping,
        DeltaTime,
        Mass,
        ClothPointNumSize,
        ClothWorldMatrix,
        SphereRadius,
        SpherePosition,
        SphereWorldMatrix
    };

    ClothShader() = default;
    ~ClothShader() override = default;

    ClothShader(const ClothShader&) = delete;
    ClothShader(const ClothShader&&) = delete;
    ClothShader& operator=(const ClothShader&) = delete;
    ClothShader& operator=(const ClothShader&&) = delete;
};