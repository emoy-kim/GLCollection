#pragma once

#include "../common/include/renderer.h"
#include "../common/include/shader.h"

namespace ray_tracing
{
    enum UNIFORM { FrameIndex = 0, SphereNum, Sphere };

    enum SPHERE_UNIFORM { Type = 0, Radius, Albedo, Center, UniformNum };
}

namespace scene
{
    enum UNIFORM { ModelViewProjectionMatrix = 0 };
}

class C11RayTracing final : public RendererGL
{
public:
    C11RayTracing();
    ~C11RayTracing() override = default;

    C11RayTracing(C11RayTracing&&) = delete;
    C11RayTracing(const C11RayTracing&) = delete;
    C11RayTracing& operator=(C11RayTracing&&) = delete;
    C11RayTracing& operator=(const C11RayTracing&) = delete;

    void play();

private:
    struct Sphere
    {
        enum class TYPE { METAL = 1, LAMBERTIAN };

        TYPE Type = TYPE::METAL;
        float Radius = 0.0f;
        glm::vec3 Center{};
        glm::vec3 Albedo{};

        Sphere() = default;

        Sphere(TYPE type, float radius, const glm::vec3& center, const glm::vec3& albedo)
            : Type( type ), Radius( radius ), Center( center ), Albedo( albedo ) {}
    };

    int FrameIndex = 0;
    std::vector<Sphere> Spheres;
    std::unique_ptr<ShaderGL> RayTracingShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ShaderGL> ScreenShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ObjectGL> ScreenObject = std::make_unique<ObjectGL>();
    std::unique_ptr<CanvasGL> FinalCanvas = std::make_unique<CanvasGL>();

    void cursor(GLFWwindow* window, double xpos, double ypos) override {}
    void mouse(GLFWwindow* window, int button, int action, int mods) override {}
    void mousewheel(GLFWwindow* window, double xoffset, double yoffset) const override {}
    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void render() const;
    void update();
};