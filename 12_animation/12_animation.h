#pragma once

#include "../common/include/renderer.h"
#include "../common/include/shader.h"
#include "animator.h"

class C12Animation final : public RendererGL
{
public:
    C12Animation();
    ~C12Animation() override = default;

    C12Animation(const C12Animation&) = delete;
    C12Animation(const C12Animation&&) = delete;
    C12Animation& operator=(const C12Animation&) = delete;
    C12Animation& operator=(const C12Animation&&) = delete;

    void play();

private:
    enum UNIFORM { ModelViewProjectionMatrix = 0, Color };

    double StartTiming;
    std::unique_ptr<ShaderGL> ObjectShader;
    std::vector<std::unique_ptr<ObjectGL>> Objects;
    std::unique_ptr<Animator2D> Animator;

    void cursor(GLFWwindow* window, double xpos, double ypos) override {}
    void mouse(GLFWwindow* window, int button, int action, int mods) override {}
    void mousewheel(GLFWwindow* window, double xoffset, double yoffset) const override {}
    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void setObjects();
    void render() const;
    [[nodiscard]] static Animator2D::Keyframe getStartKeyframe();
    [[nodiscard]] static Animator2D::Keyframe getEndKeyframe();
};