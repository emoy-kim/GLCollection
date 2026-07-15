#pragma once

#include "../01_lighting/01_lighting.h"
#include "../common/include/shader.h"

class C07WaveSimulation final : public RendererGL
{
public:
    C07WaveSimulation();
    ~C07WaveSimulation() override = default;

    C07WaveSimulation(C07WaveSimulation&&) = delete;
    C07WaveSimulation(const C07WaveSimulation&) = delete;
    C07WaveSimulation& operator=(C07WaveSimulation&&) = delete;
    C07WaveSimulation& operator=(const C07WaveSimulation&) = delete;

    void play();

private:
    enum UNIFORM { PointNum = 0, Factor };

    int WaveTargetIndex = 0;
    float WaveFactor = 20.0f;
    glm::ivec2 WavePointNum{ 100 };
    glm::ivec2 WaveGridSize{ 25 };
    std::array<GLuint, 3> WaveBuffers{};
    std::unique_ptr<ShaderGL> ObjectShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ShaderGL> WaveShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ShaderGL> WaveNormalShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ObjectGL> WaveObject = std::make_unique<ObjectGL>();
    std::unique_ptr<LightGL> Lights = std::make_unique<LightGL>();

    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void setLights() const;
    void setWaveObject();
    void render();
};