#pragma once

#include "light.h"
#include "camera.h"
#include "object.h"

class RendererGL
{
public:
    RendererGL();
    virtual ~RendererGL() = default;

    RendererGL(const RendererGL&) = delete;
    RendererGL(const RendererGL&&) = delete;
    RendererGL& operator=(const RendererGL&) = delete;
    RendererGL& operator=(const RendererGL&&) = delete;

protected:
    inline static RendererGL* Renderer = nullptr;
    GLFWwindow* Window;
    int FrameWidth;
    int FrameHeight;
    glm::ivec2 ClickedPoint;
    std::unique_ptr<CameraGL> MainCamera;

    void registerCallbacks() const;
    void initialize();
    static void printOpenGLInformation();

    static void error(int e, const char* description)
    {
        std::ignore = e;
        puts( description );
    }

    static void cleanup(GLFWwindow* window) { glfwSetWindowShouldClose( window, GLFW_TRUE ); }
    virtual void cursor(GLFWwindow* window, double xpos, double ypos) {}
    virtual void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {}
    virtual void mouse(GLFWwindow* window, int button, int action, int mods) {}
    virtual void mousewheel(GLFWwindow* window, double xoffset, double yoffset) const {}

    virtual void reshape(GLFWwindow* window, int width, int height) const
    {
        std::ignore = window;
        MainCamera->update3DCamera( width, height );
        glViewport( 0, 0, width, height );
    }

    static void keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Renderer->keyboard( window, key, scancode, action, mods );
    }

    static void cursorWrapper(GLFWwindow* window, double xpos, double ypos) { Renderer->cursor( window, xpos, ypos ); }

    static void mouseWrapper(GLFWwindow* window, int button, int action, int mods)
    {
        Renderer->mouse( window, button, action, mods );
    }

    static void mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset)
    {
        Renderer->mousewheel( window, xoffset, yoffset );
    }

    static void reshapeWrapper(GLFWwindow* window, int width, int height)
    {
        Renderer->reshape( window, width, height );
    }
};