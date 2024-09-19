#pragma once

#include "../common/include/shader.h"

class ProjectorShader final : public ShaderGL
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

    ProjectorShader() = default;
    ~ProjectorShader() override = default;

    ProjectorShader(const ProjectorShader&) = delete;
    ProjectorShader(const ProjectorShader&&) = delete;
    ProjectorShader& operator=(const ProjectorShader&) = delete;
    ProjectorShader& operator=(const ProjectorShader&&) = delete;
};