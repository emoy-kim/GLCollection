#pragma once

#include "../common/include/renderer.h"
#include "shadow_shader.h"

class C10ShadowMapping final : public RendererGL
{
public:
    C10ShadowMapping();
    ~C10ShadowMapping() override;

    C10ShadowMapping(const C10ShadowMapping&) = delete;
    C10ShadowMapping(const C10ShadowMapping&&) = delete;
    C10ShadowMapping& operator=(const C10ShadowMapping&) = delete;
    C10ShadowMapping& operator=(const C10ShadowMapping&&) = delete;

    void play();

private:
    float LightTheta;
    GLuint FBO;
    GLuint DepthTextureID;
    std::unique_ptr<CameraGL> LightCamera;
    std::unique_ptr<SimpleShaderGL> ObjectShader;
    std::unique_ptr<ShadowShaderGL> ShadowShader;
    std::unique_ptr<ObjectGL> GroundObject;
    std::unique_ptr<ObjectGL> TigerObject;
    std::unique_ptr<ObjectGL> PandaObject;
    std::unique_ptr<LightGL> Lights;

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