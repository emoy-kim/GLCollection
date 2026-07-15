#pragma once

#include "../01_lighting/01_lighting.h"
#include "../common/include/shader.h"

class C03GimbalLock final : public RendererGL
{
public:
    C03GimbalLock();
    ~C03GimbalLock() override = default;

    C03GimbalLock(C03GimbalLock&&) = delete;
    C03GimbalLock(const C03GimbalLock&) = delete;
    C03GimbalLock& operator=(C03GimbalLock&&) = delete;
    C03GimbalLock& operator=(const C03GimbalLock&) = delete;

    void play();

private:
    struct Animation
    {
        bool AnimationMode = false;
        double AnimationDuration = 10000.0;
        double TimePerSection = 0.0;
        double StartTiming = 0.0;
        double ElapsedTime = 0.0;
        uint CurrentFrameIndex = 0;

        Animation() = default;
    };

    int CapturedFrameIndex = 0;
    glm::vec3 EulerAngle{};
    std::vector<glm::vec3> CapturedEulerAngles{ 5 };
    std::vector<glm::quat> CapturedQuaternions{ 5 };
    std::unique_ptr<Animation> Animator = std::make_unique<Animation>();
    std::unique_ptr<ShaderGL> ObjectShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ObjectGL> AxisObject = std::make_unique<ObjectGL>();
    std::unique_ptr<ObjectGL> TeapotObject = std::make_unique<ObjectGL>();
    std::unique_ptr<LightGL> Lights = std::make_unique<LightGL>();

    void cursor(GLFWwindow* window, double xpos, double ypos) override;
    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void captureFrame();
    void setLights() const;
    void setAxisObject() const;
    void setTeapotObject() const;
    void drawAxisObject(float scale_factor = 1.0f) const;
    void drawTeapotObject(const glm::mat4& to_world) const;
    void displayEulerAngleMode();
    void displayQuaternionMode() const;
    void displayCapturedFrames() const;
    void render();
};