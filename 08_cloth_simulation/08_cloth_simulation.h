#pragma once

#include "../01_lighting/01_lighting.h"
#include "../common/include/shader.h"

namespace cloth
{
    enum UNIFORM
    {
        SpringRestLength = 0,
        SpringStiffness,
        SpringDamping,
        ShearStiffness,
        ShearDamping,
        FlexionStiffness,
        FlexionDamping,
        DeltaTime,
        Mass,
        ClothPointNumSize,
        ClothWorldMatrix,
        SphereRadius,
        SpherePosition,
        SphereWorldMatrix
    };
}

class C08ClothSimulation final : public RendererGL
{
public:
    C08ClothSimulation();
    ~C08ClothSimulation() override = default;

    C08ClothSimulation(C08ClothSimulation&&) = delete;
    C08ClothSimulation(const C08ClothSimulation&) = delete;
    C08ClothSimulation& operator=(C08ClothSimulation&&) = delete;
    C08ClothSimulation& operator=(const C08ClothSimulation&) = delete;

    void play();

private:
    bool Moving = false;
    float SphereRadius = 20.0f;
    uint ClothTargetIndex = 0;
    glm::ivec2 ClothPointNumSize{ 100 };
    glm::ivec2 ClothGridSize{ 50 };
    glm::vec3 SpherePosition{ 0.0f };
    glm::mat4 ClothWorldMatrix =
        translate( glm::mat4( 1.0f ), glm::vec3( 150.0f, 50.0f, 0.0f ) ) *
        rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) ) *
        scale( glm::mat4( 1.0f ), glm::vec3( 3.0f ) );
    glm::mat4 SphereWorldMatrix = translate( glm::mat4( 1.0f ), glm::vec3( 100.0f, 120.0f, 30.0f ) );
    std::array<GLuint, 3> ClothBuffers{};
    std::unique_ptr<ShaderGL> ObjectShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ShaderGL> ClothShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ObjectGL> ClothObject = std::make_unique<ObjectGL>();
    std::unique_ptr<ObjectGL> SphereObject = std::make_unique<ObjectGL>();
    std::unique_ptr<LightGL> Lights = std::make_unique<LightGL>();

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