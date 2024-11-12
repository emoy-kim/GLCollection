#include "12_animation.h"

C12Animation::C12Animation()
    : StartTiming( 0.0 ),
      ObjectShader( std::make_unique<ShaderGL>() ),
      Animator( std::make_unique<Animator2D>() )
{
    MainCamera = std::make_unique<CameraGL>( FrameWidth, FrameHeight );

    const std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/12_animation/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/shader.vert" ).c_str(),
        std::string( shader_directory_path + "/shader.frag" ).c_str()
    );
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
}

void C12Animation::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    switch (key) {
        case GLFW_KEY_R:
            StartTiming = glfwGetTime() * 1000.0;
            std::cout << "Replay Animation!\n";
            break;
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            cleanup( window );
            break;
        default:
            return;
    }
}

Animator2D::Keyframe C12Animation::getStartKeyframe()
{
    Animator2D::Keyframe keyframe;
    keyframe.FillType = Animator2D::FILL_TYPE::FILL;
    keyframe.TopLeft = { 100, 100 };
    keyframe.ObjectWidth = 200;
    keyframe.ObjectHeight = 200;
    keyframe.Anchor = { 0.5f, 0.5f };
    keyframe.Duration = 1000.0f;

    keyframe.Start.Color = { 0.0f, 1.0f, 0.0f };
    keyframe.Start.Translation = { 0.0f, 0.0f };
    keyframe.Start.Scale = { 1.0f, 1.0f };
    keyframe.Start.RotationAngle = 0.0f;

    keyframe.End.Color = { 0.0f, 1.0f, 0.0f };
    keyframe.End.Translation = { 500.0f, 500.0f };
    keyframe.End.Scale = { 2.0f, 2.0f };
    keyframe.End.RotationAngle = 0.0f;
    return keyframe;
}

Animator2D::Keyframe C12Animation::getEndKeyframe()
{
    Animator2D::Keyframe keyframe;
    keyframe.FillType = Animator2D::FILL_TYPE::FILL;
    keyframe.TopLeft = { 1500, 100 };
    keyframe.ObjectWidth = 200;
    keyframe.ObjectHeight = 500;
    keyframe.Anchor = { 0.5f, 0.5f };
    keyframe.Duration = 2000.0f;

    keyframe.Start.Color = { 0.0f, 0.0f, 1.0f };
    keyframe.Start.Translation = { 0.0f, 0.0f };
    keyframe.Start.Scale = { 1.0f, 1.0f };
    keyframe.Start.RotationAngle = 0.0f;

    keyframe.End.Color = { 0.0f, 1.0f, 1.0f };
    keyframe.End.Translation = { 0.0f, 0.0f };
    keyframe.End.Scale = { 1.0f, 1.0f };
    keyframe.End.RotationAngle = 90.0f;
    return keyframe;
}

void C12Animation::setObjects()
{
    Animator->addKeyframe( getStartKeyframe() );
    Animator->addKeyframe( getEndKeyframe() );

    Objects.clear();
    const int key_frame_num = Animator->getTotalKeyframesNum();
    for (int i = 0; i < key_frame_num; ++i) {
        Objects.emplace_back( std::make_unique<ObjectGL>() );
        Objects[i]->setSquareObject( GL_TRIANGLES );
    }
}

void C12Animation::render() const
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glUseProgram( ObjectShader->getShaderProgram() );
    const auto current_time = static_cast<float>(glfwGetTime() * 1000.0 - StartTiming);
    for (int i = 0; i < Animator->getTotalKeyframesNum(); ++i) {
        const GLenum fill_type = Animator->getFillType( i );
        glPolygonMode( GL_FRONT_AND_BACK, fill_type );

        Animator2D::Animation animation;
        Animator->getAnimationNow( animation, i, current_time );

        const glm::mat4 to_world = Animator->getWorldMatrix( animation, FrameHeight, i );
        ObjectShader->uniformMat4fv(
            ModelViewProjectionMatrix,
            MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
        );
        ObjectShader->uniform3fv( Color, animation.Color );
        glBindVertexArray( Objects[i]->getVAO() );
        glDrawArrays( Objects[i]->getDrawMode(), 0, Objects[i]->getVertexNum() );
    }
}

void C12Animation::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    setObjects();

    StartTiming = glfwGetTime() * 1000.0;
    while (!glfwWindowShouldClose( Window )) {
        render();

        glfwSwapBuffers( Window );
        glfwPollEvents();
    }
    glfwDestroyWindow( Window );
}

int main()
{
    C12Animation renderer{};
    renderer.play();
    return 0;
}