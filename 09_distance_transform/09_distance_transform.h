#pragma once

#include "../common/include/renderer.h"
#include "../common/include/shader.h"

namespace mvp
{
    enum UNIFORM { ModelViewProjectionMatrix = 0 };
}

namespace distance_transform
{
    enum UNIFORM { Phase = 0, DistanceType };
}

class C09DistanceTransform final : public RendererGL
{
public:
    C09DistanceTransform();
    ~C09DistanceTransform() override = default;

    C09DistanceTransform(C09DistanceTransform&&) = delete;
    C09DistanceTransform(const C09DistanceTransform&) = delete;
    C09DistanceTransform& operator=(C09DistanceTransform&&) = delete;
    C09DistanceTransform& operator=(const C09DistanceTransform&) = delete;

    void play();

protected:
    enum class DISTANCE_TYPE { EUCLIDEAN = 1, MANHATTAN, CHESSBOARD };

    DISTANCE_TYPE DistanceType = DISTANCE_TYPE::EUCLIDEAN;
    GLuint InsideColumnScannerBuffer = 0;
    GLuint OutsideColumnScannerBuffer = 0;
    GLuint InsideDistanceFieldBuffer = 0;
    GLuint OutsideDistanceFieldBuffer = 0;
    std::unique_ptr<ShaderGL> ObjectShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ShaderGL> FieldShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ShaderGL> TransformShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ObjectGL> ImageObject = std::make_unique<ObjectGL>();
    std::unique_ptr<ObjectGL> DistanceObject = std::make_unique<ObjectGL>();
    std::unique_ptr<CanvasGL> Canvas = std::make_unique<CanvasGL>();

    void cursor(GLFWwindow* window, double xpos, double ypos) override {}
    void mouse(GLFWwindow* window, int button, int action, int mods) override {}
    void mousewheel(GLFWwindow* window, double xoffset, double yoffset) const override {}
    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;

    void reshape(GLFWwindow* window, int width, int height) const override
    {
        std::ignore = window;
        MainCamera->update2DCamera( width, height );
        glViewport( 0, 0, width, height );
    }

    void setObjects();
    void drawImage() const;
    void drawDistanceField();
    void render();
};