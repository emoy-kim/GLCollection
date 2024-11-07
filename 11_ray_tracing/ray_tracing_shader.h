#pragma once

#include "../common/include/shader.h"

class RayTracingShaderGL final : public ShaderGL
{
public:
    enum UNIFORM { FrameIndex = 0, SphereNum, Sphere };

    enum SPHERE_UNIFORM { Type = 0, Radius, Albedo, Center, UniformNum };

    RayTracingShaderGL() = default;
    ~RayTracingShaderGL() override = default;

    RayTracingShaderGL(const RayTracingShaderGL&) = delete;
    RayTracingShaderGL(const RayTracingShaderGL&&) = delete;
    RayTracingShaderGL& operator=(const RayTracingShaderGL&) = delete;
    RayTracingShaderGL& operator=(const RayTracingShaderGL&&) = delete;
};

class SceneShaderGL final : public ShaderGL
{
public:
    enum UNIFORM { ModelViewProjectionMatrix = 0 };

    SceneShaderGL() = default;
    ~SceneShaderGL() override = default;

    SceneShaderGL(const SceneShaderGL&) = delete;
    SceneShaderGL(const SceneShaderGL&&) = delete;
    SceneShaderGL& operator=(const SceneShaderGL&) = delete;
    SceneShaderGL& operator=(const SceneShaderGL&&) = delete;
};