#pragma once

#include "../common/include/renderer.h"
#include "../01_lighting/lighting_shader.h"

class C03GimbalLock final : public RendererGL
{
public:
    C03GimbalLock();
    ~C03GimbalLock() override = default;

    C03GimbalLock(const C03GimbalLock&) = delete;
    C03GimbalLock(const C03GimbalLock&&) = delete;
    C03GimbalLock& operator=(const C03GimbalLock&) = delete;
    C03GimbalLock& operator=(const C03GimbalLock&&) = delete;

    void play();

private:
    struct Animation
    {
        bool AnimationMode;
        double AnimationDuration;
        double TimePerSection;
        double StartTiming;
        double ElapsedTime;
        uint CurrentFrameIndex;

        Animation()
            : AnimationMode( false ),
              AnimationDuration( 10000.0 ),
              TimePerSection( 0.0 ),
              StartTiming( 0.0 ),
              ElapsedTime( 0.0 ),
              CurrentFrameIndex( 0 ) {}
    };

    int CapturedFrameIndex;
    glm::vec3 EulerAngle;
    std::vector<glm::vec3> CapturedEulerAngles;
    std::vector<glm::quat> CapturedQuaternions;
    std::unique_ptr<Animation> Animator;
    std::unique_ptr<ShaderGL> ObjectShader;
    std::unique_ptr<ObjectGL> AxisObject;
    std::unique_ptr<ObjectGL> TeapotObject;
    std::unique_ptr<LightGL> Lights;

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