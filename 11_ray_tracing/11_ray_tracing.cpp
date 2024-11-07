#include "11_ray_tracing.h"

C11RayTracing::C11RayTracing()
    : FrameIndex( 0 ),
      RayTracingShader( std::make_unique<RayTracingShaderGL>() ),
      ScreenShader( std::make_unique<SceneShaderGL>() ),
      ScreenObject( std::make_unique<ObjectGL>() ),
      FinalCanvas( std::make_unique<CanvasGL>() )
{
    MainCamera = std::make_unique<CameraGL>( FrameWidth, FrameHeight );

    const std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/11_ray_tracing/shaders";
    RayTracingShader->setComputeShader( std::string( shader_directory_path + "/raytracer.comp" ).c_str() );
    ScreenShader->setShader(
        std::string( shader_directory_path + "/screen.vert" ).c_str(),
        std::string( shader_directory_path + "/screen.frag" ).c_str()
    );

    FinalCanvas->setCanvas( FrameWidth, FrameHeight, GL_RGBA8 );
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
}

void C11RayTracing::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    std::ignore = scancode;
    std::ignore = mods;
    if (action != GLFW_PRESS) return;

    switch (key) {
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            cleanup( window );
            break;
        default:
            return;
    }
}

void C11RayTracing::render() const
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( RayTracingShader->getShaderProgram() );
    const auto sphere_size = static_cast<int>(Spheres.size());
    RayTracingShader->uniform1i( RayTracingShaderGL::FrameIndex, FrameIndex );
    RayTracingShader->uniform1i( RayTracingShaderGL::SphereNum, sphere_size );
    for (int i = 0; i < sphere_size; ++i) {
        const int offset = RayTracingShaderGL::Sphere + RayTracingShaderGL::UniformNum * i;
        RayTracingShader->uniform1i( offset + RayTracingShaderGL::Type, static_cast<int>(Spheres[i].Type) );
        RayTracingShader->uniform1f( offset + RayTracingShaderGL::Radius, Spheres[i].Radius );
        RayTracingShader->uniform3fv( offset + RayTracingShaderGL::Albedo, Spheres[i].Albedo );
        RayTracingShader->uniform3fv( offset + RayTracingShaderGL::Center, Spheres[i].Center );
    }

    glBindImageTexture( 0, FinalCanvas->getColor0TextureID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8 );
    glDispatchCompute( getGroupSize( FrameWidth ), getGroupSize( FrameHeight ), 1 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glUseProgram( ScreenShader->getShaderProgram() );

    const glm::mat4 to_world = scale( glm::mat4( 1.0f ), glm::vec3( FrameWidth, FrameHeight, 1.0f ) );
    ScreenShader->uniformMat4fv(
        SceneShaderGL::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    glBindTextureUnit( 0, FinalCanvas->getColor0TextureID() );
    glBindVertexArray( ScreenObject->getVAO() );
    glDrawArrays( ScreenObject->getDrawMode(), 0, ScreenObject->getVertexNum() );
}

void C11RayTracing::update()
{
    // if animation is needed, update here ...
}

void C11RayTracing::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    Spheres = {
        { Sphere::TYPE::LAMBERTIAN, 0.5f, glm::vec3( 0.0f, 0.0f, -1.0f ), glm::vec3( 0.8f, 0.3f, 0.3f ) },
        { Sphere::TYPE::LAMBERTIAN, 100.0f, glm::vec3( 0.0f, -100.5f, -1.0f ), glm::vec3( 0.8f, 0.8f, 0.0f ) },
        { Sphere::TYPE::METAL, 0.5f, glm::vec3( 1.0f, 0.0f, -1.0f ), glm::vec3( 0.8f, 0.6f, 0.2f ) },
        { Sphere::TYPE::METAL, 0.5f, glm::vec3( -1.0f, 0.0f, -1.0f ), glm::vec3( 0.8f, 0.8f, 0.8f ) }
    };
    ScreenObject->setSquareObject( GL_TRIANGLES, true );

    constexpr double update_time = 0.1;
    double last = glfwGetTime(), time_delta = 0.0;
    while (!glfwWindowShouldClose( Window )) {
        const double now = glfwGetTime();
        time_delta += now - last;
        last = now;
        if (time_delta >= update_time) {
            update();
            time_delta -= update_time;
        }

        render();
        FrameIndex++;

        glfwSwapBuffers( Window );
        glfwPollEvents();
    }
    glfwDestroyWindow( Window );
}

int main()
{
    C11RayTracing renderer{};
    renderer.play();
    return 0;
}