#pragma once

#include "../common/include/renderer.h"
#include "distance_transform_shader.h"

class C09DistanceTransform final : public RendererGL
{
public:
    C09DistanceTransform();
    ~C09DistanceTransform() override = default;

    C09DistanceTransform(const C09DistanceTransform&) = delete;
    C09DistanceTransform(const C09DistanceTransform&&) = delete;
    C09DistanceTransform& operator=(const C09DistanceTransform&) = delete;
    C09DistanceTransform& operator=(const C09DistanceTransform&&) = delete;

    void play();

protected:
    enum class DISTANCE_TYPE { EUCLIDEAN = 1, MANHATTAN, CHESSBOARD };

    DISTANCE_TYPE DistanceType;
    GLuint InsideColumnScannerBuffer;
    GLuint OutsideColumnScannerBuffer;
    GLuint InsideDistanceFieldBuffer;
    GLuint OutsideDistanceFieldBuffer;
    std::unique_ptr<MVPShaderGL> ObjectShader;
    std::unique_ptr<MVPShaderGL> FieldShader;
    std::unique_ptr<DistanceTransformShaderGL> TransformShader;
    std::unique_ptr<ObjectGL> ImageObject;
    std::unique_ptr<ObjectGL> DistanceObject;
    std::unique_ptr<CanvasGL> Canvas;

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