#pragma once

#include "../common/include/renderer.h"
#include "lighting_shader.h"

class C01Lighting final : public RendererGL
{
public:
    C01Lighting();
    ~C01Lighting() override = default;

    C01Lighting(const C01Lighting&) = delete;
    C01Lighting(const C01Lighting&&) = delete;
    C01Lighting& operator=(const C01Lighting&) = delete;
    C01Lighting& operator=(const C01Lighting&&) = delete;

    void play();

private:
    bool DrawMovingObject;
    int ObjectRotationAngle;
    std::unique_ptr<ShaderGL> ObjectShader;
    std::unique_ptr<ObjectGL> Object;
    std::unique_ptr<LightGL> Lights;

    void cursor(GLFWwindow* window, double xpos, double ypos) override;
    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void mouse(GLFWwindow* window, int button, int action, int mods) override;
    void mousewheel(GLFWwindow* window, double xoffset, double yoffset) const override;

    void setLights() const;
    void setObject() const;
    void drawObject(const float& scale_factor = 1.0f) const;
    void render() const;
    void update();
};