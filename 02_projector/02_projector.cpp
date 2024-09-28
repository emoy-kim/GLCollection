#include "02_projector.h"

C02Projector::C02Projector()
    : VideoFrameIndex( 0 ),
      IsVideo( true ),
      Pause( false ),
      SlideBuffer( nullptr ),
      Projector(
          std::make_unique<CameraGL>(
              glm::vec3{ 40.0f, 30.0f, 20.0f },
              glm::vec3{ 0.0f, 0.0f, 0.0f },
              glm::vec3{ 0.0f, 1.0f, 0.0f },
              30.0f, 10.0f, 60.0f
          )
      ),
      ObjectShader( std::make_unique<ShaderGL>() ),
      ProjectorPyramidObject( std::make_unique<ObjectGL>() ),
      WallObject( std::make_unique<ObjectGL>() ),
      Lights( std::make_unique<LightGL>() )
{
    av_log_set_level( AV_LOG_ERROR );

    MainCamera = std::make_unique<CameraGL>(
        glm::vec3{ 90.0f, 50.0f, 90.0f },
        glm::vec3{ 0.0f, 0.0f, 0.0f },
        glm::vec3{ 0.0f, 1.0f, 0.0f }
    );
    MainCamera->update3DCamera( FrameWidth, FrameHeight );

    const std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/02_projector/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/projector.vert" ).c_str(),
        std::string( shader_directory_path + "/projector.frag" ).c_str()
    );
    glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
}

C02Projector::~C02Projector()
{
    delete [] SlideBuffer;
}

void C02Projector::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    std::ignore = scancode;
    std::ignore = mods;
    if (action != GLFW_PRESS) return;

    switch (key) {
        case GLFW_KEY_UP: MainCamera->moveForward();
            break;
        case GLFW_KEY_DOWN: MainCamera->moveBackward();
            break;
        case GLFW_KEY_LEFT: MainCamera->moveLeft();
            break;
        case GLFW_KEY_RIGHT: MainCamera->moveRight();
            break;
        case GLFW_KEY_W: MainCamera->moveUp();
            break;
        case GLFW_KEY_S: MainCamera->moveDown();
            break;
        case GLFW_KEY_I:
            MainCamera->resetCamera();
            Projector->resetCamera();
            break;
        case GLFW_KEY_R:
            if (IsVideo) {
                prepareSlide();
                std::cout << "Replay Video!\n";
            }
            break;
        case GLFW_KEY_L:
            Lights->toggleLightSwitch();
            std::cout << "Light Turned " << (Lights->isLightOn() ? "On!\n" : "Off!\n");
            break;
        case GLFW_KEY_ENTER:
            IsVideo = !IsVideo;
            prepareSlide();
            break;
        case GLFW_KEY_SPACE:
            if (IsVideo) Pause = !Pause;
            break;
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            cleanup( window );
            break;
        default:
            return;
    }
}

void C02Projector::cursor(GLFWwindow* window, double xpos, double ypos)
{
    if (CameraGL* camera = glfwGetKey( window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS ?
                               MainCamera.get() :
                               Projector.get();
        camera->getMovingState()) {
        const auto x = static_cast<int>(round( xpos ));
        const auto y = static_cast<int>(round( ypos ));
        const int dx = x - ClickedPoint.x;
        const int dy = y - ClickedPoint.y;
        camera->moveForward( -dy );
        camera->rotateAroundWorldY( -dx );

        if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
            camera->pitch( -dy );
        }

        ClickedPoint.x = x;
        ClickedPoint.y = y;
    }
}

void C02Projector::mouse(GLFWwindow* window, int button, int action, int mods)
{
    std::ignore = mods;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        const bool moving_state = action == GLFW_PRESS;
        if (moving_state) {
            double x, y;
            glfwGetCursorPos( window, &x, &y );
            ClickedPoint.x = static_cast<int>(round( x ));
            ClickedPoint.y = static_cast<int>(round( y ));
        }

        if (glfwGetKey( window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS) {
            MainCamera->setMovingState( moving_state );
        }
        else Projector->setMovingState( moving_state );
    }
}

void C02Projector::setLights() const
{
    const glm::vec4 light_position( Projector->getCameraPosition(), 1.0f );
    constexpr glm::vec4 ambient_color( 0.3f, 0.3f, 0.3f, 1.0f );
    constexpr glm::vec4 diffuse_color( 0.7f, 0.7f, 0.7f, 1.0f );
    constexpr glm::vec4 specular_color( 0.9f, 0.9f, 0.9f, 1.0f );
    Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );
}

void C02Projector::setWallObject() const
{
    constexpr float size = 30.0f;
    std::vector<glm::vec3> wall_vertices;
    wall_vertices.emplace_back( size, 0.0f, 0.0f );
    wall_vertices.emplace_back( size, size, 0.0f );
    wall_vertices.emplace_back( 0.0f, size, 0.0f );

    wall_vertices.emplace_back( size, 0.0f, 0.0f );
    wall_vertices.emplace_back( 0.0f, size, 0.0f );
    wall_vertices.emplace_back( 0.0f, 0.0f, 0.0f );

    wall_vertices.emplace_back( size, 0.0f, size );
    wall_vertices.emplace_back( size, 0.0f, 0.0f );
    wall_vertices.emplace_back( 0.0f, 0.0f, 0.0f );

    wall_vertices.emplace_back( size, 0.0f, size );
    wall_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
    wall_vertices.emplace_back( 0.0f, 0.0f, size );

    wall_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
    wall_vertices.emplace_back( 0.0f, size, 0.0f );
    wall_vertices.emplace_back( 0.0f, size, size );

    wall_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
    wall_vertices.emplace_back( 0.0f, size, size );
    wall_vertices.emplace_back( 0.0f, 0.0f, size );

    std::vector<glm::vec3> wall_normals;
    wall_normals.emplace_back( 0.0f, 0.0f, 1.0f );
    wall_normals.emplace_back( 0.0f, 0.0f, 1.0f );
    wall_normals.emplace_back( 0.0f, 0.0f, 1.0f );

    wall_normals.emplace_back( 0.0f, 0.0f, 1.0f );
    wall_normals.emplace_back( 0.0f, 0.0f, 1.0f );
    wall_normals.emplace_back( 0.0f, 0.0f, 1.0f );

    wall_normals.emplace_back( 0.0f, 1.0f, 0.0f );
    wall_normals.emplace_back( 0.0f, 1.0f, 0.0f );
    wall_normals.emplace_back( 0.0f, 1.0f, 0.0f );

    wall_normals.emplace_back( 0.0f, 1.0f, 0.0f );
    wall_normals.emplace_back( 0.0f, 1.0f, 0.0f );
    wall_normals.emplace_back( 0.0f, 1.0f, 0.0f );

    wall_normals.emplace_back( 1.0f, 0.0f, 0.0f );
    wall_normals.emplace_back( 1.0f, 0.0f, 0.0f );
    wall_normals.emplace_back( 1.0f, 0.0f, 0.0f );

    wall_normals.emplace_back( 1.0f, 0.0f, 0.0f );
    wall_normals.emplace_back( 1.0f, 0.0f, 0.0f );
    wall_normals.emplace_back( 1.0f, 0.0f, 0.0f );

    WallObject->setObject( GL_TRIANGLES, wall_vertices, wall_normals );
    WallObject->setDiffuseReflectionColor( { 0.52f, 0.12f, 0.15f, 1.0f } );
}

void C02Projector::prepareSlide()
{
    static const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/02_projector/samples";
    static const std::string image_path = sample_directory_path + "/image.jpg";
    static const std::string video_path = sample_directory_path + "/video.mp4";

    ScreenObject = std::make_unique<ObjectGL>();
    if (!IsVideo) {
        ScreenObject->setSquareObject( GL_TRIANGLES, image_path );
        const glm::ivec2 screen_size = ScreenObject->getTextureSize( ScreenObject->getTextureID( 0 ) );
        Projector->update3DCamera( screen_size.x / 100, screen_size.y / 100 );
    }
    else {
        Pause = false;
        VideoFrameIndex = 0;
        delete [] SlideBuffer;
        Video = std::make_unique<VideoReader>();
        Video->open( video_path );
        const int w = Video->getFrameWidth();
        const int h = Video->getFrameHeight();
        SlideBuffer = new uint8_t[w * h * 4];
        if (!Video->read( SlideBuffer, VideoFrameIndex++ ))
            throw std::runtime_error( "Could not read a video frame!" );
        Projector->update3DCamera( w / 100, h / 100 );
        ScreenObject->setSquareObject( GL_TRIANGLES, SlideBuffer, w, h );
    }
}

void C02Projector::setProjectorPyramidObject() const
{
    const float near_plane = Projector->getNearPlane();
    const float far_plane = Projector->getFarPlane();
    const float half_width = static_cast<float>(Projector->getWidth()) * 0.5f * far_plane / near_plane;
    const float half_height = static_cast<float>(Projector->getHeight()) * 0.5f * far_plane / near_plane;

    std::vector<glm::vec3> pyramid_vertices;
    pyramid_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
    pyramid_vertices.emplace_back( -half_width, half_height, -far_plane );

    pyramid_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
    pyramid_vertices.emplace_back( half_width, half_height, -far_plane );

    pyramid_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
    pyramid_vertices.emplace_back( half_width, -half_height, -far_plane );

    pyramid_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
    pyramid_vertices.emplace_back( -half_width, -half_height, -far_plane );

    pyramid_vertices.emplace_back( -half_width, half_height, -far_plane );
    pyramid_vertices.emplace_back( half_width, half_height, -far_plane );

    pyramid_vertices.emplace_back( half_width, half_height, -far_plane );
    pyramid_vertices.emplace_back( half_width, -half_height, -far_plane );

    pyramid_vertices.emplace_back( half_width, -half_height, -far_plane );
    pyramid_vertices.emplace_back( -half_width, -half_height, -far_plane );

    pyramid_vertices.emplace_back( -half_width, -half_height, -far_plane );
    pyramid_vertices.emplace_back( -half_width, half_height, -far_plane );

    ProjectorPyramidObject->setObject( GL_LINES, pyramid_vertices );
    ProjectorPyramidObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 0.0f, 1.0f } );
}

void C02Projector::drawWallObject() const
{
    using u = ProjectorShader::UNIFORM;
    using l = ShaderGL::LIGHT_UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    constexpr glm::mat4 to_world( 1.0f );
    ObjectShader->uniformMat4fv( u::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniformMat4fv( u::ProjectorViewMatrix, Projector->getViewMatrix() );
    ObjectShader->uniformMat4fv( u::ProjectorProjectionMatrix, Projector->getProjectionMatrix() );
    ObjectShader->uniform1i( u::WhichObject, WALL );

    ObjectShader->uniform4fv( u::Material + m::EmissionColor, WallObject->getEmissionColor() );
    ObjectShader->uniform4fv( u::Material + m::AmbientColor, WallObject->getAmbientReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::DiffuseColor, WallObject->getDiffuseReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::SpecularColor, WallObject->getSpecularReflectionColor() );
    ObjectShader->uniform1f( u::Material + m::SpecularExponent, WallObject->getSpecularReflectionExponent() );
    ObjectShader->uniform1i( u::UseLight, Lights->isLightOn() ? 1 : 0 );
    if (Lights->isLightOn()) {
        ObjectShader->uniform1i( u::LightNum, Lights->getTotalLightNum() );
        ObjectShader->uniform4fv( u::GlobalAmbient, Lights->getGlobalAmbientColor() );
        for (int i = 0; i < Lights->getTotalLightNum(); ++i) {
            const int offset = u::Lights + l::UniformNum * i;
            ObjectShader->uniform1i( offset + l::LightSwitch, Lights->isActivated( i ) ? 1 : 0 );
            ObjectShader->uniform4fv( offset + l::LightPosition, Lights->getPosition( i ) );
            ObjectShader->uniform4fv( offset + l::LightAmbientColor, Lights->getAmbientColors( i ) );
            ObjectShader->uniform4fv( offset + l::LightDiffuseColor, Lights->getDiffuseColors( i ) );
            ObjectShader->uniform4fv( offset + l::LightSpecularColor, Lights->getSpecularColors( i ) );
            ObjectShader->uniform3fv( offset + l::SpotlightDirection, Lights->getSpotlightDirections( i ) );
            ObjectShader->uniform1f( offset + l::SpotlightCutoffAngle, Lights->getSpotlightCutoffAngles( i ) );
            ObjectShader->uniform1f( offset + l::SpotlightFeather, Lights->getSpotlightFeathers( i ) );
            ObjectShader->uniform1f( offset + l::FallOffRadius, Lights->getFallOffRadii( i ) );
        }
    }

    glBindTextureUnit( 0, ScreenObject->getTextureID( 0 ) );
    glBindVertexArray( WallObject->getVAO() );
    glDrawArrays( WallObject->getDrawMode(), 0, WallObject->getVertexNum() );
}

void C02Projector::drawScreenObject() const
{
    using u = ProjectorShader::UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    const glm::mat4 to_world = inverse( Projector->getViewMatrix() );
    ObjectShader->uniformMat4fv( u::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniform1i( u::WhichObject, SCREEN );

    ObjectShader->uniform4fv( u::Material + m::EmissionColor, ScreenObject->getEmissionColor() );
    ObjectShader->uniform4fv( u::Material + m::AmbientColor, ScreenObject->getAmbientReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::DiffuseColor, ScreenObject->getDiffuseReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::SpecularColor, ScreenObject->getSpecularReflectionColor() );
    ObjectShader->uniform1f( u::Material + m::SpecularExponent, ScreenObject->getSpecularReflectionExponent() );

    glBindTextureUnit( 0, ScreenObject->getTextureID( 0 ) );
    glBindVertexArray( ScreenObject->getVAO() );
    glDrawArrays( ScreenObject->getDrawMode(), 0, ScreenObject->getVertexNum() );
}

void C02Projector::drawProjectorObject() const
{
    using u = ProjectorShader::UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    glLineWidth( 3.0f );

    const glm::mat4 to_world = inverse( Projector->getViewMatrix() );
    ObjectShader->uniformMat4fv( u::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniform1i( u::WhichObject, PROJECTOR );

    ObjectShader->uniform4fv( u::Material + m::EmissionColor, ProjectorPyramidObject->getEmissionColor() );
    ObjectShader->uniform4fv( u::Material + m::AmbientColor, ProjectorPyramidObject->getAmbientReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::DiffuseColor, ProjectorPyramidObject->getDiffuseReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::SpecularColor, ProjectorPyramidObject->getSpecularReflectionColor() );
    ObjectShader->uniform1f(
        u::Material + m::SpecularExponent, ProjectorPyramidObject->getSpecularReflectionExponent()
    );

    glBindVertexArray( ProjectorPyramidObject->getVAO() );
    glDrawArrays( ProjectorPyramidObject->getDrawMode(), 0, ProjectorPyramidObject->getVertexNum() );
    glLineWidth( 1.0f );
}

void C02Projector::render() const
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    MainCamera->update3DCamera( FrameWidth, FrameHeight );
    glViewport( 0, 0, FrameWidth, FrameHeight );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glUseProgram( ObjectShader->getShaderProgram() );
    drawWallObject();
    drawScreenObject();
    drawProjectorObject();
}

void C02Projector::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    setLights();
    setWallObject();
    prepareSlide();
    setProjectorPyramidObject();
    while (!glfwWindowShouldClose( Window )) {
        render();

        if (IsVideo && !Pause && Video->read( SlideBuffer, VideoFrameIndex++ )) {
            ScreenObject->updateTexture( SlideBuffer, 0, Video->getFrameWidth(), Video->getFrameHeight() );
        }

        glfwSwapBuffers( Window );
        glfwPollEvents();
    }
    glfwDestroyWindow( Window );
}

int main()
{
    C02Projector renderer{};
    renderer.play();
    return 0;
}