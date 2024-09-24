#pragma once

#include "../common/include/light.h"
#include "../common/include/camera.h"
#include "../common/include/object.h"
#include "lighting_shader.h"

class RendererGL final
{
public:
   RendererGL(const RendererGL&) = delete;
   RendererGL(const RendererGL&&) = delete;
   RendererGL& operator=(const RendererGL&) = delete;
   RendererGL& operator=(const RendererGL&&) = delete;


   RendererGL();
   ~RendererGL() = default;

   void play();

private:
   inline static RendererGL* Renderer = nullptr;
   GLFWwindow* Window;
   int FrameWidth;
   int FrameHeight;
   glm::ivec2 ClickedPoint;
   std::unique_ptr<CameraGL> MainCamera;
   std::unique_ptr<ShaderGL> ObjectShader;
   std::unique_ptr<ObjectGL> Object;
   std::unique_ptr<LightGL> Lights;

   bool DrawMovingObject;
   int ObjectRotationAngle;

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
   void mousewheel(GLFWwindow* window, double xoffset, double yoffset) const;
   void reshape(GLFWwindow* window, int width, int height) const;

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

   static void reshapeWrapper(GLFWwindow* window, int width, int height) { Renderer->reshape( window, width, height ); }

   void setLights() const;
   void setObject() const;
   void drawObject(const float& scale_factor = 1.0f) const;
   void render() const;
   void update();
};