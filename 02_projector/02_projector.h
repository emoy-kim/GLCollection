#pragma once

#include "../common/include/renderer.h"
#include "../common/include/video_reader.h"
#include "../common/include/shader.h"

namespace projector
{
    enum UNIFORM
    {
        WorldMatrix = 0,
        ViewMatrix,
        ModelViewProjectionMatrix,
        ProjectorViewMatrix,
        ProjectorProjectionMatrix,
        Lights,
        Material = 293,
        WhichObject = 298,
        UseLight,
        LightNum,
        GlobalAmbient
    };
}

class C02Projector final : public RendererGL
{
public:
    C02Projector();
    ~C02Projector() override;

    C02Projector(C02Projector&&) = delete;
    C02Projector(const C02Projector&) = delete;
    C02Projector& operator=(C02Projector&&) = delete;
    C02Projector& operator=(const C02Projector&) = delete;

    void play();

private:
    enum WhichObject { WALL = 0, SCREEN, PROJECTOR };

    int VideoFrameIndex = 0;
    bool IsVideo = true;
    bool Pause = false;
    uint8_t* SlideBuffer = nullptr;
    std::unique_ptr<CameraGL> Projector = std::make_unique<CameraGL>(
        glm::vec3{ 40.0f, 30.0f, 20.0f },
        glm::vec3{ 0.0f, 0.0f, 0.0f },
        glm::vec3{ 0.0f, 1.0f, 0.0f },
        0.0f, 10.0f, 60.0f
    );
    std::unique_ptr<ShaderGL> ObjectShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ObjectGL> ProjectorPyramidObject;
    std::unique_ptr<ObjectGL> ScreenObject;
    std::unique_ptr<ObjectGL> WallObject = std::make_unique<ObjectGL>();
    std::unique_ptr<LightGL> Lights = std::make_unique<LightGL>();
    std::unique_ptr<VideoReader> Video;

    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void cursor(GLFWwindow* window, double xpos, double ypos) override;
    void mouse(GLFWwindow* window, int button, int action, int mods) override;
    void setProjectorPyramidObject();
    void prepareSlide();
    void setLights() const;
    void setWallObject() const;
    void drawWallObject() const;
    void drawScreenObject() const;
    void drawProjectorObject() const;
    void render() const;
};