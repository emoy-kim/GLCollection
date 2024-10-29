#pragma once

#include "../common/include/renderer.h"
#include "../common/include/video_reader.h"
#include "projector_shader.h"

class C02Projector final : public RendererGL
{
public:
    C02Projector();
    ~C02Projector() override;

    C02Projector(const C02Projector&) = delete;
    C02Projector(const C02Projector&&) = delete;
    C02Projector& operator=(const C02Projector&) = delete;
    C02Projector& operator=(const C02Projector&&) = delete;

    void play();

private:
    enum WhichObject { WALL = 0, SCREEN, PROJECTOR };

    int VideoFrameIndex;
    bool IsVideo;
    bool Pause;
    uint8_t* SlideBuffer;
    std::unique_ptr<CameraGL> Projector;
    std::unique_ptr<ShaderGL> ObjectShader;
    std::unique_ptr<ObjectGL> ProjectorPyramidObject;
    std::unique_ptr<ObjectGL> ScreenObject;
    std::unique_ptr<ObjectGL> WallObject;
    std::unique_ptr<LightGL> Lights;
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