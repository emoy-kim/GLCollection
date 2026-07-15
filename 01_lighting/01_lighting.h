#pragma once

#include "../common/include/renderer.h"
#include "../common/include/shader.h"

namespace lighting
{
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
}

class C01Lighting final : public RendererGL
{
public:
    C01Lighting();
    ~C01Lighting() override = default;

    C01Lighting(C01Lighting&&) = delete;
    C01Lighting(const C01Lighting&) = delete;
    C01Lighting& operator=(C01Lighting&&) = delete;
    C01Lighting& operator=(const C01Lighting&) = delete;

    void play();

private:
    bool DrawMovingObject = false;
    int ObjectRotationAngle = 0;
    std::unique_ptr<ShaderGL> ObjectShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ObjectGL> Object = std::make_unique<ObjectGL>();
    std::unique_ptr<LightGL> Lights = std::make_unique<LightGL>();

    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void setLights() const;
    void setObject() const;
    void drawObject(const float& scale_factor = 1.0f) const;
    void render() const;
    void update();
};