#pragma once

#include "../common/include/renderer.h"
#include "../01_lighting/lighting_shader.h"
#include "cloth_shader.h"

class C08ClothSimulation final : public RendererGL
{
public:
    C08ClothSimulation();
    ~C08ClothSimulation() override = default;

    C08ClothSimulation(const C08ClothSimulation&) = delete;
    C08ClothSimulation(const C08ClothSimulation&&) = delete;
    C08ClothSimulation& operator=(const C08ClothSimulation&) = delete;
    C08ClothSimulation& operator=(const C08ClothSimulation&&) = delete;

    void play();

private:
    bool Moving;
    float SphereRadius;
    uint ClothTargetIndex;
    glm::ivec2 ClothPointNumSize;
    glm::ivec2 ClothGridSize;
    glm::vec3 SpherePosition;
    glm::mat4 ClothWorldMatrix;
    glm::mat4 SphereWorldMatrix;
    std::array<GLuint, 3> ClothBuffers;
    std::unique_ptr<ShaderGL> ObjectShader;
    std::unique_ptr<ShaderGL> ClothShader;
    std::unique_ptr<ObjectGL> ClothObject;
    std::unique_ptr<ObjectGL> SphereObject;
    std::unique_ptr<LightGL> Lights;

    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void cursor(GLFWwindow* window, double xpos, double ypos) override;
    void mouse(GLFWwindow* window, int button, int action, int mods) override;
    void setLights() const;
    void setClothObject();
    void setSphereObject() const;
    void applyForces();
    void drawClothObject() const;
    void drawSphereObject() const;
    void render();
};