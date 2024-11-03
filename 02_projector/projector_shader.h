#pragma once

#include "../common/include/shader.h"

class ProjectorShaderGL final : public ShaderGL
{
public:
    enum UNIFORM
    {
        WorldMatrix = 0,
        ViewMatrix,
        ModelViewProjectionMatrix,
        ProjectorViewMatrix,
        ProjectorProjectionMatrix,
        Lights,
        Material = 293,
        WhichObject = 298,
        UseLight,
        LightNum,
        GlobalAmbient
    };

    ProjectorShaderGL() = default;
    ~ProjectorShaderGL() override = default;

    ProjectorShaderGL(const ProjectorShaderGL&) = delete;
    ProjectorShaderGL(const ProjectorShaderGL&&) = delete;
    ProjectorShaderGL& operator=(const ProjectorShaderGL&) = delete;
    ProjectorShaderGL& operator=(const ProjectorShaderGL&&) = delete;
};