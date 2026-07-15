#pragma once

#include "../common/include/renderer.h"
#include "../common/include/shader.h"

namespace shadow
{
    enum UNIFORM
    {
        WorldMatrix = 0,
        ViewMatrix,
        ModelViewProjectionMatrix,
        LightViewProjectionMatrix,
        Lights,
        Material = 292,
        UseTexture = 297,
        UseLight,
        LightIndex,
        GlobalAmbient
    };
}

namespace simple
{
    enum UNIFORM { ModelViewProjectionMatrix = 0 };
}

class C10ShadowMapping final : public RendererGL
{
public:
    C10ShadowMapping();
    ~C10ShadowMapping() override;

    C10ShadowMapping(C10ShadowMapping&&) = delete;
    C10ShadowMapping(const C10ShadowMapping&) = delete;
    C10ShadowMapping& operator=(C10ShadowMapping&&) = delete;
    C10ShadowMapping& operator=(const C10ShadowMapping&) = delete;

    void play();

private:
    float LightTheta = 0.0f;
    GLuint FBO = 0;
    GLuint DepthTextureID = 0;
    std::unique_ptr<CameraGL> LightCamera = std::make_unique<CameraGL>();
    std::unique_ptr<ShaderGL> ObjectShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ShaderGL> ShadowShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ObjectGL> GroundObject = std::make_unique<ObjectGL>();
    std::unique_ptr<ObjectGL> TigerObject = std::make_unique<ObjectGL>();
    std::unique_ptr<ObjectGL> PandaObject = std::make_unique<ObjectGL>();
    std::unique_ptr<LightGL> Lights = std::make_unique<LightGL>();

    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void setLights() const;
    void setGroundObject() const;
    void setTigerObject() const;
    void setPandaObject() const;
    void setDepthFrameBuffer();
    void drawDepthMapFromLightView(int light_index) const;
    void drawShadow(int light_index) const;
    void render() const;
};