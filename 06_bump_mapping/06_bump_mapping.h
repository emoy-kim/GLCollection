#pragma once

#include "../common/include/renderer.h"
#include "bump_mapping_shader.h"

class C06BumpMapping final : public RendererGL
{
public:
    C06BumpMapping();
    ~C06BumpMapping() override = default;

    C06BumpMapping(const C06BumpMapping&) = delete;
    C06BumpMapping(const C06BumpMapping&&) = delete;
    C06BumpMapping& operator=(const C06BumpMapping&) = delete;
    C06BumpMapping& operator=(const C06BumpMapping&&) = delete;

    void play();

private:
    bool UseBumpMapping;
    int NormalTextureIndex;
    float LightTheta;
    std::unique_ptr<LightGL> Lights;
    std::unique_ptr<ShaderGL> ObjectShader;
    std::unique_ptr<ShaderGL> BoxBlurShader;
    std::unique_ptr<ShaderGL> NormalMapShader;
    std::array<std::unique_ptr<ObjectGL>, 9> WallObjects;

    void cursor(GLFWwindow* window, double xpos, double ypos) override;
    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void mouse(GLFWwindow* window, int button, int action, int mods) override;

    void setLights() const;
    void createNormalMap(ObjectGL* object);
    void setWallObjects();
    void drawWallObject(const ObjectGL* object, const glm::mat4& to_world) const;
    void render() const;
};