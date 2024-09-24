#pragma once

#include "../common/include/light.h"
#include "../common/include/camera.h"
#include "../common/include/object.h"
#include "../01_lighting/lighting_shader.h"

class RendererGL final
{
public:
    RendererGL();
    ~RendererGL() = default;

    RendererGL(const RendererGL&) = delete;
    RendererGL(const RendererGL&&) = delete;
    RendererGL& operator=(const RendererGL&) = delete;
    RendererGL& operator=(const RendererGL&&) = delete;

    void play();

private:
    struct Animation
    {
        bool AnimationMode;
        double AnimationDuration;
        double TimePerSection;
        double StartTiming;
        double ElapsedTime;
        uint CurrentFrameIndex;

        Animation()
            : AnimationMode( false ),
              AnimationDuration( 10000.0 ),
              TimePerSection( 0.0 ),
              StartTiming( 0.0 ),
              ElapsedTime( 0.0 ),
              CurrentFrameIndex( 0 ) {}
    };

    inline static RendererGL* Renderer = nullptr;
    GLFWwindow* Window;
    int FrameWidth;
    int FrameHeight;
    int CapturedFrameIndex;
    glm::ivec2 ClickedPoint;
    glm::vec3 EulerAngle;
    std::vector<glm::vec3> CapturedEulerAngles;
    std::vector<glm::quat> CapturedQuaternions;
    std::unique_ptr<Animation> Animator;
    std::unique_ptr<CameraGL> MainCamera;
    std::unique_ptr<ShaderGL> ObjectShader;
    std::unique_ptr<ObjectGL> AxisObject;
    std::unique_ptr<ObjectGL> TeapotObject;
    std::unique_ptr<LightGL> Lights;

    void registerCallbacks() const;
    void initialize();

    static void printOpenGLInformation();

    static void error(int e, const char* description)
    {
        std::ignore = e;
        puts( description );
    }

    static void cleanup(GLFWwindow* window) { glfwSetWindowShouldClose( window, GLFW_TRUE ); }
    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
    void cursor(GLFWwindow* window, double xpos, double ypos);
    void mouse(GLFWwindow* window, int button, int action, int mods);

    void reshape(GLFWwindow* window, int width, int height) const
    {
        std::ignore = window;
        MainCamera->updateWindowSize( width, height );
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

    static void reshapeWrapper(GLFWwindow* window, int width, int height)
    {
        Renderer->reshape( window, width, height );
    }

    void captureFrame();
    void setLights() const;
    void setAxisObject() const;
    void setTeapotObject() const;
    void drawAxisObject(float scale_factor = 1.0f) const;
    void drawTeapotObject(const glm::mat4& to_world) const;
    void displayEulerAngleMode();
    void displayQuaternionMode() const;
    void displayCapturedFrames() const;
    void render();
};