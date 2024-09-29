#pragma once

#include "../common/include/renderer.h"
#include "../01_lighting/lighting_shader.h"

class C07WaveSimulation final : public RendererGL
{
public:
    C07WaveSimulation();
    ~C07WaveSimulation() override = default;

    C07WaveSimulation(const C07WaveSimulation&) = delete;
    C07WaveSimulation(const C07WaveSimulation&&) = delete;
    C07WaveSimulation& operator=(const C07WaveSimulation&) = delete;
    C07WaveSimulation& operator=(const C07WaveSimulation&&) = delete;

    void play();

private:
    enum UNIFORM { PointNum = 0, Factor };

    int WaveTargetIndex;
    float WaveFactor;
    glm::ivec2 WavePointNum;
    glm::ivec2 WaveGridSize;
    std::array<GLuint, 3> WaveBuffers;
    std::unique_ptr<ShaderGL> ObjectShader;
    std::unique_ptr<ShaderGL> WaveShader;
    std::unique_ptr<ShaderGL> WaveNormalShader;
    std::unique_ptr<ObjectGL> WaveObject;
    std::unique_ptr<LightGL> Lights;

    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void setLights() const;
    void setWaveObject();
    void render();
};