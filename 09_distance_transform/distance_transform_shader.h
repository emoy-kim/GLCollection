#pragma once

#include "../common/include/shader.h"

class MVPShader final : public ShaderGL
{
public:
    enum UNIFORM { ModelViewProjectionMatrix = 0 };

    MVPShader() = default;
    ~MVPShader() override = default;

    MVPShader(const MVPShader&) = delete;
    MVPShader(const MVPShader&&) = delete;
    MVPShader& operator=(const MVPShader&) = delete;
    MVPShader& operator=(const MVPShader&&) = delete;
};

class DistanceTransformShader final : public ShaderGL
{
public:
    enum UNIFORM { Phase = 0, DistanceType };

    DistanceTransformShader() = default;
    ~DistanceTransformShader() override = default;

    DistanceTransformShader(const DistanceTransformShader&) = delete;
    DistanceTransformShader(const DistanceTransformShader&&) = delete;
    DistanceTransformShader& operator=(const DistanceTransformShader&) = delete;
    DistanceTransformShader& operator=(const DistanceTransformShader&&) = delete;
};