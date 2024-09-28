#pragma once

#include "../common/include/renderer.h"
#include "../common/include/shader.h"
#include "../common/include/video_reader.h"

class C04CubeMapping final : public RendererGL
{
public:
    C04CubeMapping();
    ~C04CubeMapping() override;

    C04CubeMapping(const C04CubeMapping&) = delete;
    C04CubeMapping(const C04CubeMapping&&) = delete;
    C04CubeMapping& operator=(const C04CubeMapping&) = delete;
    C04CubeMapping& operator=(const C04CubeMapping&&) = delete;

    void play();

private:
    enum UNIFORM { ModelViewProjectionMatrix = 0, Color };

    bool IsVideo;
    int VideoFrameIndex;
    std::array<uint8_t*, 6> FrameBuffers;
    std::unique_ptr<ShaderGL> ObjectShader;
    std::unique_ptr<ObjectGL> CubeObject;
    std::array<std::unique_ptr<VideoReader>, 6> Videos;

    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void setCubeObject(float length);
    void render() const;
};