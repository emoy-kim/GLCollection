#include "03_gimbal_lock.h"

C03GimbalLock::C03GimbalLock()
    : CapturedFrameIndex( 0 ),
      EulerAngle(),
      CapturedEulerAngles{ 5 },
      CapturedQuaternions{ 5 },
      Animator( std::make_unique<Animation>() ),
      ObjectShader( std::make_unique<ShaderGL>() ),
      AxisObject( std::make_unique<ObjectGL>() ),
      TeapotObject( std::make_unique<ObjectGL>() ),
      Lights( std::make_unique<LightGL>() )
{
    MainCamera = std::make_unique<CameraGL>(
        glm::vec3( 0.0f, 30.0f, 50.0f ),
        glm::vec3( 0.0f, 0.0f, 0.0f ),
        glm::vec3( 0.0f, 1.0f, 0.0f )
    );
    MainCamera->update3DCamera( FrameWidth, FrameHeight );

    const std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/01_lighting/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/scene_shader.vert" ).c_str(),
        std::string( shader_directory_path + "/scene_shader.frag" ).c_str()
    );
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
}

void C03GimbalLock::captureFrame()
{
    if (CapturedFrameIndex < 5) {
        CapturedEulerAngles[CapturedFrameIndex] = EulerAngle;
        CapturedQuaternions[CapturedFrameIndex] = toQuat( orientate3( EulerAngle ) );
        CapturedFrameIndex++;
    }
}

void C03GimbalLock::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
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

void C03GimbalLock::cursor(GLFWwindow* window, double xpos, double ypos)
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

void C03GimbalLock::setLights() const
{
    constexpr glm::vec4 light_position( 10.0f, 15.0f, 15.0f, 1.0f );
    constexpr glm::vec4 ambient_color( 0.5f, 0.5f, 0.5f, 1.0f );
    constexpr glm::vec4 diffuse_color( 0.9f, 0.9f, 0.9f, 1.0f );
    constexpr glm::vec4 specular_color( 0.9f, 0.9f, 0.9f, 1.0f );
    Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );
}

void C03GimbalLock::setAxisObject() const
{
    const std::vector<glm::vec3> axis_vertices = {
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f }
    };
    AxisObject->setObject( GL_LINES, axis_vertices );
}

void C03GimbalLock::setTeapotObject() const
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

void C03GimbalLock::drawAxisObject(float scale_factor) const
{
    using u = LightingShaderGL::UNIFORM;
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

void C03GimbalLock::drawTeapotObject(const glm::mat4& to_world) const
{
    using u = LightingShaderGL::UNIFORM;
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

void C03GimbalLock::displayEulerAngleMode()
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

void C03GimbalLock::displayQuaternionMode() const
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

void C03GimbalLock::displayCapturedFrames() const
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

void C03GimbalLock::render()
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

void C03GimbalLock::play()
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
    C03GimbalLock renderer{};
    renderer.play();
    return 0;
}