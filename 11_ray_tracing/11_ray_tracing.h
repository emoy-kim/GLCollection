#pragma once

#include "../common/include/renderer.h"
#include "ray_tracing_shader.h"

class C11RayTracing final : public RendererGL
{
public:
    C11RayTracing();
    ~C11RayTracing() override = default;

    C11RayTracing(const C11RayTracing&) = delete;
    C11RayTracing(const C11RayTracing&&) = delete;
    C11RayTracing& operator=(const C11RayTracing&) = delete;
    C11RayTracing& operator=(const C11RayTracing&&) = delete;

    void play();

private:
    struct Sphere
    {
        enum class TYPE { METAL = 1, LAMBERTIAN };

        TYPE Type;
        float Radius;
        glm::vec3 Center;
        glm::vec3 Albedo;

        Sphere() : Type( TYPE::METAL ), Radius( 0.0f ), Center(), Albedo() {}

        Sphere(TYPE type, float radius, const glm::vec3& center, const glm::vec3& albedo)
            : Type( type ), Radius( radius ), Center( center ), Albedo( albedo ) {}
    };

    int FrameIndex;
    std::vector<Sphere> Spheres;
    std::unique_ptr<RayTracingShaderGL> RayTracingShader;
    std::unique_ptr<ShaderGL> ScreenShader;
    std::unique_ptr<ObjectGL> ScreenObject;
    std::unique_ptr<CanvasGL> FinalCanvas;

    void cursor(GLFWwindow* window, double xpos, double ypos) override {}
    void mouse(GLFWwindow* window, int button, int action, int mods) override {}
    void mousewheel(GLFWwindow* window, double xoffset, double yoffset) const override {}
    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void render() const;
    void update();
};