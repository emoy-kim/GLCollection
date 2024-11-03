#pragma once

#include "../common/include/shader.h"

class MVPShaderGL final : public ShaderGL
{
public:
    enum UNIFORM { ModelViewProjectionMatrix = 0 };

    MVPShaderGL() = default;
    ~MVPShaderGL() override = default;

    MVPShaderGL(const MVPShaderGL&) = delete;
    MVPShaderGL(const MVPShaderGL&&) = delete;
    MVPShaderGL& operator=(const MVPShaderGL&) = delete;
    MVPShaderGL& operator=(const MVPShaderGL&&) = delete;
};

class DistanceTransformShaderGL final : public ShaderGL
{
public:
    enum UNIFORM { Phase = 0, DistanceType };

    DistanceTransformShaderGL() = default;
    ~DistanceTransformShaderGL() override = default;

    DistanceTransformShaderGL(const DistanceTransformShaderGL&) = delete;
    DistanceTransformShaderGL(const DistanceTransformShaderGL&&) = delete;
    DistanceTransformShaderGL& operator=(const DistanceTransformShaderGL&) = delete;
    DistanceTransformShaderGL& operator=(const DistanceTransformShaderGL&&) = delete;
};