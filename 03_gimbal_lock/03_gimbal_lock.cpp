#include "03_gimbal_lock.h"

RendererGL::RendererGL()
    : Window( nullptr ),
      FrameWidth( 1920 ),
      FrameHeight( 1080 ),
      CapturedFrameIndex( 0 ),
      ClickedPoint( -1, -1 ),
      EulerAngle(),
      CapturedEulerAngles{ 5 },
      CapturedQuaternions{ 5 },
      Animator( std::make_unique<Animation>() ),
      MainCamera(
          std::make_unique<CameraGL>(
              glm::vec3( 0.0f, 30.0f, 50.0f ),
              glm::vec3( 0.0f, 0.0f, 0.0f ),
              glm::vec3( 0.0f, 1.0f, 0.0f )
          )
      ),
      ObjectShader( std::make_unique<ShaderGL>() ),
      AxisObject( std::make_unique<ObjectGL>() ),
      TeapotObject( std::make_unique<ObjectGL>() ),
      Lights( std::make_unique<LightGL>() )

{
    Renderer = this;

    initialize();
    printOpenGLInformation();
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
    glfwSetWindowUserPointer( Window, this );
    glfwMakeContextCurrent( Window );

    if (!gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress )) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    registerCallbacks();

    glEnable( GL_DEPTH_TEST );
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

    MainCamera->updateWindowSize( FrameWidth, FrameHeight );

    const std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/01_lighting/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/scene_shader.vert" ).c_str(),
        std::string( shader_directory_path + "/scene_shader.frag" ).c_str()
    );
}

void RendererGL::captureFrame()
{
    if (CapturedFrameIndex < 5) {
        CapturedEulerAngles[CapturedFrameIndex] = EulerAngle;
        CapturedQuaternions[CapturedFrameIndex] = toQuat( orientate3( EulerAngle ) );
        CapturedFrameIndex++;
    }
}

void RendererGL::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    std::ignore = scancode;
    std::ignore = mods;
    if (action != GLFW_PRESS) return;

    switch (key) {
        case GLFW_KEY_C:
            captureFrame();
            break;
        case GLFW_KEY_L:
            Lights->toggleLightSwitch();
            std::cout << "Light Turned " << (Lights->isLightOn() ? "On!\n" : "Off!\n");
            break;
        case GLFW_KEY_R:
            CapturedFrameIndex = 0;
            Animator->AnimationMode = false;
            CapturedEulerAngles.clear();
            CapturedQuaternions.clear();
            CapturedEulerAngles.resize( 5 );
            CapturedQuaternions.resize( 5 );
            break;
        case GLFW_KEY_SPACE:
            if (!Animator->AnimationMode && CapturedFrameIndex == static_cast<int>(CapturedEulerAngles.size())) {
                Animator->StartTiming = glfwGetTime() * 1000.0;
                Animator->AnimationMode = true;
            }
            break;
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            cleanup( window );
            break;
        default:
            return;
    }
}

void RendererGL::cursor(GLFWwindow* window, double xpos, double ypos)
{
    if (MainCamera->getMovingState()) {
        const auto x = static_cast<int>(round( xpos ));
        const auto y = static_cast<int>(round( ypos ));
        const int dx = x - ClickedPoint.x;
        const int dy = y - ClickedPoint.y;

        if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS) {
            EulerAngle.y += static_cast<float>(dx) * 0.01f;
            if (EulerAngle.y >= 360.0f) EulerAngle.y -= 360.0f;
        }

        if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
            EulerAngle.x += static_cast<float>(dy) * 0.01f;
            if (EulerAngle.x >= 360.0f) EulerAngle.x -= 360.0f;
        }

        ClickedPoint.x = x;
        ClickedPoint.y = y;
    }
}

void RendererGL::mouse(GLFWwindow* window, int button, int action, int mods)
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
        MainCamera->setMovingState( moving_state );
    }
}

void RendererGL::registerCallbacks() const
{
    glfwSetWindowCloseCallback( Window, cleanup );
    glfwSetKeyCallback( Window, keyboardWrapper );
    glfwSetCursorPosCallback( Window, cursorWrapper );
    glfwSetMouseButtonCallback( Window, mouseWrapper );
    glfwSetFramebufferSizeCallback( Window, reshapeWrapper );
}

void RendererGL::setLights() const
{
    constexpr glm::vec4 light_position( 10.0f, 15.0f, 15.0f, 1.0f );
    constexpr glm::vec4 ambient_color( 0.5f, 0.5f, 0.5f, 1.0f );
    constexpr glm::vec4 diffuse_color( 0.9f, 0.9f, 0.9f, 1.0f );
    constexpr glm::vec4 specular_color( 0.9f, 0.9f, 0.9f, 1.0f );
    Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );
}

void RendererGL::setAxisObject() const
{
    const std::vector<glm::vec3> axis_vertices = {
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f }
    };
    AxisObject->setObject( GL_LINES, axis_vertices );
}

void RendererGL::setTeapotObject() const
{
    std::vector<glm::vec3> teapot_vertices, teapot_normals;
    std::vector<glm::vec2> teapot_textures;
    const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/03_gimbal_lock";
    if (ObjectGL::readObjectFile(
        teapot_vertices,
        teapot_normals,
        teapot_textures,
        std::string( sample_directory_path + "/teapot.obj" )
    )) {
        TeapotObject->setObject( GL_TRIANGLES, teapot_vertices, teapot_normals );
    }
    else throw std::runtime_error( "Could not read object file!" );
}

void RendererGL::drawAxisObject(float scale_factor) const
{
    using u = LightingShader::UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    glUseProgram( ObjectShader->getShaderProgram() );
    glLineWidth( 5.0f );

    ObjectShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniform1i( u::UseTexture, 0 );
    ObjectShader->uniform1i( u::UseLight, 0 );

    const glm::mat4 scale_matrix = scale( glm::mat4( 1.0f ), glm::vec3( scale_factor ) );
    glm::mat4 to_world = scale_matrix;
    ObjectShader->uniformMat4fv( u::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniform4fv( u::Material + m::DiffuseColor, { 1.0f, 0.0f, 0.0f, 1.0f } );
    glBindVertexArray( AxisObject->getVAO() );
    glDrawArrays( AxisObject->getDrawMode(), 0, AxisObject->getVertexNum() );

    to_world = scale_matrix * rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ObjectShader->uniformMat4fv( u::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniform4fv( u::Material + m::DiffuseColor, { 0.0f, 1.0f, 0.0f, 1.0f } );
    glDrawArrays( AxisObject->getDrawMode(), 0, AxisObject->getVertexNum() );

    to_world = scale_matrix * rotate( glm::mat4( 1.0f ), glm::radians( -90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    ObjectShader->uniformMat4fv( u::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniform4fv( u::Material + m::DiffuseColor, { 0.0f, 0.0f, 1.0f, 1.0f } );
    glDrawArrays( AxisObject->getDrawMode(), 0, AxisObject->getVertexNum() );

    glLineWidth( 1.0f );
}

void RendererGL::drawTeapotObject(const glm::mat4& to_world) const
{
    using u = LightingShader::UNIFORM;
    using l = ShaderGL::LIGHT_UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    glUseProgram( ObjectShader->getShaderProgram() );
    ObjectShader->uniformMat4fv( u::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniform1i( u::UseTexture, 0 );
    ObjectShader->uniform4fv( u::Material + m::EmissionColor, TeapotObject->getEmissionColor() );
    ObjectShader->uniform4fv( u::Material + m::AmbientColor, TeapotObject->getAmbientReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::DiffuseColor, TeapotObject->getDiffuseReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::SpecularColor, TeapotObject->getSpecularReflectionColor() );
    ObjectShader->uniform1f( u::Material + m::SpecularExponent, TeapotObject->getSpecularReflectionExponent() );
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
    glBindVertexArray( TeapotObject->getVAO() );
    glDrawArrays( TeapotObject->getDrawMode(), 0, TeapotObject->getVertexNum() );
}

void RendererGL::displayEulerAngleMode()
{
    glViewport( 0, 216, 980, 864 );
    drawAxisObject( 15.0f );

    if (Animator->AnimationMode) {
        const uint curr = Animator->CurrentFrameIndex;
        const uint next = (Animator->CurrentFrameIndex + 1) % CapturedEulerAngles.size();
        const auto t = static_cast<float>(Animator->ElapsedTime / Animator->TimePerSection - curr);
        EulerAngle = (1 - t) * CapturedEulerAngles[curr] + t * CapturedEulerAngles[next];
    }
    TeapotObject->setDiffuseReflectionColor( { 0.0f, 0.47f, 0.75f, 1.0f } );

    const glm::mat4 to_world = orientate4( EulerAngle );
    drawTeapotObject( to_world );
}

void RendererGL::displayQuaternionMode() const
{
    glViewport( 980, 216, 980, 864 );
    drawAxisObject( 15.0f );

    glm::mat4 to_world;
    if (Animator->AnimationMode) {
        const uint curr = Animator->CurrentFrameIndex;
        const uint next = (Animator->CurrentFrameIndex + 1) % CapturedQuaternions.size();
        const auto t = static_cast<float>(Animator->ElapsedTime / Animator->TimePerSection - curr);
        to_world = toMat4( slerp( CapturedQuaternions[curr], CapturedQuaternions[next], t ) );
    }
    else to_world = orientate4( EulerAngle );

    TeapotObject->setDiffuseReflectionColor( { 1.0f, 0.37f, 0.37f, 1.0f } );
    drawTeapotObject( to_world );
}

void RendererGL::displayCapturedFrames() const
{
    for (int i = 0; i < 5; ++i) {
        glViewport( 384 * i, 0, 384, 216 );
        drawAxisObject( 15.0f );

        if (i < CapturedFrameIndex) {
            if (Animator->AnimationMode && i == Animator->CurrentFrameIndex) {
                TeapotObject->setDiffuseReflectionColor( { 1.0f, 0.7f, 0.0f, 1.0f } );
            }
            else TeapotObject->setDiffuseReflectionColor( { 0.7f, 0.7f, 1.0f, 1.0f } );

            glm::mat4 to_world = toMat4( CapturedQuaternions[i] );
            drawTeapotObject( to_world );
        }
    }
}

void RendererGL::render()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if (Animator->AnimationMode) {
        const double now = glfwGetTime() * 1000.0;
        Animator->ElapsedTime = now - Animator->StartTiming;
        if (Animator->ElapsedTime >= Animator->AnimationDuration) {
            Animator->StartTiming = now;
            Animator->ElapsedTime = 0.0;
        }
        Animator->CurrentFrameIndex = static_cast<int>(std::floor( Animator->ElapsedTime / Animator->TimePerSection ));
    }

    displayEulerAngleMode();
    displayQuaternionMode();
    displayCapturedFrames();
}

void RendererGL::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    setLights();
    setAxisObject();
    setTeapotObject();

    Animator->TimePerSection = Animator->AnimationDuration / static_cast<double>(CapturedEulerAngles.size());
    while (!glfwWindowShouldClose( Window )) {
        render();
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