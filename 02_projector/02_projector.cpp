#include "02_projector.h"

RendererGL::RendererGL()
    : Window( nullptr ),
      FrameWidth( 1920 ),
      FrameHeight( 1080 ),
      IsVideo( false ),
      SlideBuffer( nullptr ),
      ClickedPoint( -1, -1 ),
      MainCamera(
          std::make_unique<CameraGL>(
              glm::vec3( 90.0f, 50.0f, 90.0f ),
              glm::vec3( 0.0f, 0.0f, 0.0f ),
              glm::vec3( 0.0f, 1.0f, 0.0f )
          )
      ),
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
      ScreenObject( std::make_unique<ObjectGL>() ),
      WallObject( std::make_unique<ObjectGL>() ),
      Lights( std::make_unique<LightGL>() ),
      Video( std::make_unique<VideoReader>() )
{
    Renderer = this;

    initialize();
    printOpenGLInformation();
}

RendererGL::~RendererGL()
{
    delete [] SlideBuffer;
}

void RendererGL::printOpenGLInformation()
{
    std::cout << "****************************************************************\n";
    std::cout << " - GLFW version supported: " << glfwGetVersionString() << "\n";
    std::cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << "\n";
    std::cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << "\n";
    std::cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << "\n";
    std::cout << "****************************************************************\n\n";
}

void RendererGL::initialize()
{
    if (!glfwInit()) {
        std::cout << "Cannot Initialize OpenGL...\n";
        return;
    }
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
    glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    Window = glfwCreateWindow( FrameWidth, FrameHeight, "Main Camera", nullptr, nullptr );
    glfwMakeContextCurrent( Window );

    if (!gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress )) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    registerCallbacks();

    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );

    MainCamera->updateWindowSize( FrameWidth, FrameHeight );

    const std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/02_projector/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/projector.vert" ).c_str(),
        std::string( shader_directory_path + "/projector.frag" ).c_str()
    );
}

void RendererGL::error(int error, const char* description) const
{
    puts( description );
}

void RendererGL::errorWrapper(int error, const char* description)
{
    Renderer->error( error, description );
}

void RendererGL::cleanup(GLFWwindow* window)
{
    glfwSetWindowShouldClose( window, GLFW_TRUE );
}

void RendererGL::cleanupWrapper(GLFWwindow* window)
{
    Renderer->cleanup( window );
}

void RendererGL::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    switch (key) {
        case GLFW_KEY_UP:
            MainCamera->moveForward();
            break;
        case GLFW_KEY_DOWN:
            MainCamera->moveBackward();
            break;
        case GLFW_KEY_LEFT:
            MainCamera->moveLeft();
            break;
        case GLFW_KEY_RIGHT:
            MainCamera->moveRight();
            break;
        case GLFW_KEY_W:
            MainCamera->moveUp();
            break;
        case GLFW_KEY_S:
            MainCamera->moveDown();
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
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            cleanupWrapper( window );
            break;
        default:
            return;
    }
}

void RendererGL::keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Renderer->keyboard( window, key, scancode, action, mods );
}

void RendererGL::cursor(GLFWwindow* window, double xpos, double ypos)
{
    CameraGL* camera = glfwGetKey( window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS ? MainCamera.get() : Projector.get();
    if (camera->getMovingState()) {
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

void RendererGL::cursorWrapper(GLFWwindow* window, double xpos, double ypos)
{
    Renderer->cursor( window, xpos, ypos );
}

void RendererGL::mouse(GLFWwindow* window, int button, int action, int mods)
{
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

void RendererGL::mouseWrapper(GLFWwindow* window, int button, int action, int mods)
{
    Renderer->mouse( window, button, action, mods );
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset) const
{
    if (yoffset >= 0.0) MainCamera->zoomIn();
    else MainCamera->zoomOut();
}

void RendererGL::mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset)
{
    Renderer->mousewheel( window, xoffset, yoffset );
}

void RendererGL::reshape(GLFWwindow* window, int width, int height) const
{
    MainCamera->updateWindowSize( width, height );
    glViewport( 0, 0, width, height );
}

void RendererGL::reshapeWrapper(GLFWwindow* window, int width, int height)
{
    Renderer->reshape( window, width, height );
}

void RendererGL::registerCallbacks() const
{
    glfwSetErrorCallback( errorWrapper );
    glfwSetWindowCloseCallback( Window, cleanupWrapper );
    glfwSetKeyCallback( Window, keyboardWrapper );
    glfwSetCursorPosCallback( Window, cursorWrapper );
    glfwSetMouseButtonCallback( Window, mouseWrapper );
    glfwSetScrollCallback( Window, mousewheelWrapper );
    glfwSetFramebufferSizeCallback( Window, reshapeWrapper );
}

void RendererGL::setLights() const
{
    const glm::vec4 light_position( Projector->getCameraPosition(), 1.0f );
    constexpr glm::vec4 ambient_color( 0.3f, 0.3f, 0.3f, 1.0f );
    constexpr glm::vec4 diffuse_color( 0.7f, 0.7f, 0.7f, 1.0f );
    constexpr glm::vec4 specular_color( 0.9f, 0.9f, 0.9f, 1.0f );
    Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );
}

void RendererGL::setWallObject() const
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

void RendererGL::prepareSlide()
{
    static const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/02_projector/samples";
    static const std::string image_path = sample_directory_path + "/image.jpg";
    static const std::string video_path = sample_directory_path + "/video.mp4";

    if (!IsVideo) {
        ScreenObject->setSquareObject( GL_TRIANGLES, image_path );
        const glm::ivec2 screen_size = ScreenObject->getTextureSize( ScreenObject->getTextureID( 0 ) );
        Projector->updateWindowSize( screen_size.x / 100, screen_size.y / 100 );
    }
    else {
        Video.reset();
        delete [] SlideBuffer;

        Video->open( video_path );
        const int w = Video->getFrameWidth();
        const int h = Video->getFrameHeight();
        SlideBuffer = new uint8_t[w * h * 4];
        if (!Video->read( SlideBuffer, 0 ))
            throw std::runtime_error( "Could not read a video frame!" );
        Projector->updateWindowSize( w / 100, h / 100 );
        ScreenObject->setSquareObject( GL_TRIANGLES, SlideBuffer, w, h );
    }
}

void RendererGL::setScreenObject()
{
    const float near_plane = Projector->getNearPlane();
    const float half_width = static_cast<float>(Projector->getWidth()) * 0.5f;
    const float half_height = static_cast<float>(Projector->getHeight()) * 0.5f;

    std::vector<glm::vec3> screen_vertices;
    screen_vertices.emplace_back( half_width, -half_height, -near_plane );
    screen_vertices.emplace_back( half_width, half_height, -near_plane );
    screen_vertices.emplace_back( -half_width, half_height, -near_plane );

    screen_vertices.emplace_back( half_width, -half_height, -near_plane );
    screen_vertices.emplace_back( -half_width, half_height, -near_plane );
    screen_vertices.emplace_back( -half_width, -half_height, -near_plane );

    const std::vector<glm::vec3> screen_normals( 6, { 0.0f, 0.0f, 1.0f } );

    std::vector<glm::vec2> screen_textures;
    screen_textures.emplace_back( 1.0f, 0.0f );
    screen_textures.emplace_back( 1.0f, 1.0f );
    screen_textures.emplace_back( 0.0f, 1.0f );

    screen_textures.emplace_back( 1.0f, 0.0f );
    screen_textures.emplace_back( 0.0f, 1.0f );
    screen_textures.emplace_back( 0.0f, 0.0f );

    ScreenObject->setObject( GL_TRIANGLES, screen_vertices, screen_normals, screen_textures );
    prepareSlide();
}

void RendererGL::setProjectorPyramidObject() const
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

void RendererGL::drawWallObject() const
{
    using u = ProjectorShader::UNIFORM;
    using l = ShaderGL::LIGHT_UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    constexpr glm::mat4 to_world( 1.0f );
    ObjectShader->uniformMat4fv( u::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix, MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
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

void RendererGL::drawScreenObject() const
{
    using u = ProjectorShader::UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    const glm::mat4 to_world = inverse( Projector->getViewMatrix() );
    ObjectShader->uniformMat4fv( u::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix, MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
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

void RendererGL::drawProjectorObject() const
{
    using u = ProjectorShader::UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    glLineWidth( 3.0f );

    const glm::mat4 to_world = inverse( Projector->getViewMatrix() );
    ObjectShader->uniformMat4fv( u::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix, MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
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

void RendererGL::render() const
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    MainCamera->updateWindowSize( FrameWidth, FrameHeight );
    glViewport( 0, 0, FrameWidth, FrameHeight );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glUseProgram( ObjectShader->getShaderProgram() );
    drawWallObject();
    drawScreenObject();
    drawProjectorObject();
}

void RendererGL::setNextSlide()
{
    /*if (IsVideo) {
        Video >> Slide;
        if (Slide.empty()) return;

        ScreenObject->updateTexture( Slide, 0 );
    }*/
}

void RendererGL::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    setLights();
    setWallObject();
    setScreenObject();
    setProjectorPyramidObject();
    while (!glfwWindowShouldClose( Window )) {
        render();
        setNextSlide();

        glfwSwapBuffers( Window );
        glfwPollEvents();
    }
    glfwDestroyWindow( Window );
}

int main()
{
    RendererGL renderer{};
    renderer.play();
    return 0;
}