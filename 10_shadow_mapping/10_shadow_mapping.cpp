#include "10_shadow_mapping.h"

C10ShadowMapping::C10ShadowMapping()
    : LightTheta( 0.0f ),
      FBO( 0 ),
      DepthTextureID( 0 ),
      LightCamera( std::make_unique<CameraGL>() ),
      ObjectShader( std::make_unique<ShaderGL>() ),
      ShadowShader( std::make_unique<ShaderGL>() ),
      GroundObject( std::make_unique<ObjectGL>() ),
      TigerObject( std::make_unique<ObjectGL>() ),
      PandaObject( std::make_unique<ObjectGL>() ),
      Lights( std::make_unique<LightGL>() )
{
    MainCamera = std::make_unique<CameraGL>(
        glm::vec3( 1024.0f, 500.0f, 1024.0f ),
        glm::vec3( 256.0f, 0.0f, 256.0f ),
        glm::vec3( 0.0f, 1.0f, 0.0f )
    );
    MainCamera->update3DCamera( FrameWidth, FrameHeight );
    MainCamera->setRotationSensitivity( 0.0001f );

    LightCamera = std::make_unique<CameraGL>();
    LightCamera->update3DCamera( FrameWidth, FrameHeight );

    const std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/10_shadow_mapping/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/simple.vert" ).c_str(),
        std::string( shader_directory_path + "/simple.frag" ).c_str()
    );
    ShadowShader->setShader(
        std::string( shader_directory_path + "/shadow.vert" ).c_str(),
        std::string( shader_directory_path + "/shadow.frag" ).c_str()
    );
    glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
}

C10ShadowMapping::~C10ShadowMapping()
{
    if (DepthTextureID != 0)
        glDeleteTextures( 1, &DepthTextureID );
    if (FBO != 0)
        glDeleteFramebuffers( 1, &FBO );
}

void C10ShadowMapping::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    switch (key) {
        case GLFW_KEY_L:
            Lights->toggleLightSwitch();
            std::cout << "Light Turned " << (Lights->isLightOn() ? "On!\n" : "Off!\n");
            break;
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            cleanup( window );
            break;
        default:
            return;
    }
}

void C10ShadowMapping::setLights() const
{
    constexpr glm::vec4 light_position( 256.0f, 500.0f, 512.0f, 1.0f );
    constexpr glm::vec4 ambient_color( 1.0f, 1.0f, 1.0f, 1.0f );
    constexpr glm::vec4 diffuse_color( 0.7f, 0.7f, 0.7f, 1.0f );
    constexpr glm::vec4 specular_color( 0.9f, 0.9f, 0.9f, 1.0f );
    Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );
}

void C10ShadowMapping::setGroundObject() const
{
    GroundObject->setSquareObject(
        GL_TRIANGLES,
        std::string( CMAKE_SOURCE_DIR ) + "/10_shadow_mapping/samples/sand.jpg"
    );
    GroundObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void C10ShadowMapping::setTigerObject() const
{
    std::vector<glm::vec3> tiger_vertices, tiger_normals;
    std::vector<glm::vec2> tiger_textures;
    const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/10_shadow_mapping/samples";
    if (ObjectGL::readTextFile(
        tiger_vertices,
        tiger_normals,
        tiger_textures,
        std::string( sample_directory_path + "/tiger.txt" )
    )) {
        TigerObject->setObject(
            GL_TRIANGLES,
            tiger_vertices,
            tiger_normals,
            tiger_textures,
            std::string( sample_directory_path + "/tiger.jpg" )
        );
    }
    else throw std::runtime_error( "Could not read text file!" );

    TigerObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void C10ShadowMapping::setPandaObject() const
{
    std::vector<glm::vec3> panda_vertices, panda_normals;
    std::vector<glm::vec2> panda_textures;
    const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/10_shadow_mapping/samples";
    if (ObjectGL::readObjectFile(
        panda_vertices,
        panda_normals,
        panda_textures,
        std::string( sample_directory_path + "/panda.obj" )
    )) {
        PandaObject->setObject(
            GL_TRIANGLES,
            panda_vertices,
            panda_normals,
            panda_textures,
            std::string( sample_directory_path + "/panda.png" )
        );
    }
    else throw std::runtime_error( "Could not read object file!" );

    PandaObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void C10ShadowMapping::setDepthFrameBuffer()
{
    glCreateTextures( GL_TEXTURE_2D, 1, &DepthTextureID );
    glTextureStorage2D( DepthTextureID, 1, GL_DEPTH_COMPONENT32F, LightCamera->getWidth(), LightCamera->getHeight() );
    glTextureParameteri( DepthTextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTextureParameteri( DepthTextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTextureParameteri( DepthTextureID, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
    glTextureParameteri( DepthTextureID, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );

    glCreateFramebuffers( 1, &FBO );
    glNamedFramebufferTexture( FBO, GL_DEPTH_ATTACHMENT, DepthTextureID, 0 );
}

void C10ShadowMapping::drawDepthMapFromLightView(int light_index) const
{
    glBindFramebuffer( GL_FRAMEBUFFER, FBO );
    glClear( GL_DEPTH_BUFFER_BIT );
    glClearDepth( 1.0f );
    glUseProgram( ObjectShader->getShaderProgram() );
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

    LightCamera->updateCameraPosition(
        glm::vec3( Lights->getPosition( light_index ) ),
        glm::vec3( 256.0f, 0.0f, 10.0f ),
        glm::vec3( 0.0f, 1.0f, 0.0f )
    );

    glm::mat4 to_world =
        translate( glm::mat4( 1.0f ), glm::vec3( 250.0f, 0.0f, 330.0f ) ) *
        rotate( glm::mat4( 1.0f ), glm::radians( 180.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) ) *
        rotate( glm::mat4( 1.0f ), glm::radians( -90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) *
        scale( glm::mat4( 1.0f ), glm::vec3( 0.3f ) );
    ObjectShader->uniformMat4fv(
        SimpleShader::ModelViewProjectionMatrix,
        LightCamera->getProjectionMatrix() * LightCamera->getViewMatrix() * to_world
    );
    glBindVertexArray( TigerObject->getVAO() );
    glDrawArrays( TigerObject->getDrawMode(), 0, TigerObject->getVertexNum() );

    to_world =
        translate( glm::mat4( 1.0f ), glm::vec3( 250.0f, -5.0f, 180.0f ) ) *
        scale( glm::mat4( 1.0f ), glm::vec3( 20.0f ) );
    ObjectShader->uniformMat4fv(
        SimpleShader::ModelViewProjectionMatrix,
        LightCamera->getProjectionMatrix() * LightCamera->getViewMatrix() * to_world
    );
    glBindVertexArray( PandaObject->getVAO() );
    glDrawArrays( PandaObject->getDrawMode(), 0, PandaObject->getVertexNum() );

    to_world =
        translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, 0.0f, 512.0f ) ) *
        rotate( glm::mat4( 1.0f ), glm::radians( -90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) *
        scale( glm::mat4( 1.0f ), glm::vec3( 512.0f ) );
    ObjectShader->uniformMat4fv(
        SimpleShader::ModelViewProjectionMatrix,
        LightCamera->getProjectionMatrix() * LightCamera->getViewMatrix() * to_world
    );
    glBindVertexArray( GroundObject->getVAO() );
    glDrawArrays( GroundObject->getDrawMode(), 0, GroundObject->getVertexNum() );
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
}

void C10ShadowMapping::drawShadow(int light_index) const
{
    using u = ShadowShader::UNIFORM;
    using l = ShaderGL::LIGHT_UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glUseProgram( ShadowShader->getShaderProgram() );

    ShadowShader->uniformMat4fv(
        u::LightViewProjectionMatrix,
        LightCamera->getProjectionMatrix() * LightCamera->getViewMatrix()
    );
    ShadowShader->uniform1i( u::UseTexture, 1 );
    ShadowShader->uniform1i( u::UseLight, Lights->isLightOn() ? 1 : 0 );
    ShadowShader->uniform1i( u::LightIndex, light_index );
    if (Lights->isLightOn()) {
        const int offset = u::Lights + l::UniformNum * light_index;
        ShadowShader->uniform1i( offset + l::LightSwitch, Lights->isActivated( light_index ) ? 1 : 0 );
        ShadowShader->uniform4fv( offset + l::LightPosition, Lights->getPosition( light_index ) );
        ShadowShader->uniform4fv( offset + l::LightAmbientColor, Lights->getAmbientColors( light_index ) );
        ShadowShader->uniform4fv( offset + l::LightDiffuseColor, Lights->getDiffuseColors( light_index ) );
        ShadowShader->uniform4fv( offset + l::LightSpecularColor, Lights->getSpecularColors( light_index ) );
        ShadowShader->uniform3fv( offset + l::SpotlightDirection, Lights->getSpotlightDirections( light_index ) );
        ShadowShader->uniform1f( offset + l::SpotlightCutoffAngle, Lights->getSpotlightCutoffAngles( light_index ) );
        ShadowShader->uniform1f( offset + l::SpotlightFeather, Lights->getSpotlightFeathers( light_index ) );
        ShadowShader->uniform1f( offset + l::FallOffRadius, Lights->getFallOffRadii( light_index ) );
        ShadowShader->uniform4fv( u::GlobalAmbient, Lights->getGlobalAmbientColor() );
    }

    glBindTextureUnit( 1, DepthTextureID );

    glm::mat4 to_world =
        translate( glm::mat4( 1.0f ), glm::vec3( 250.0f, 0.0f, 330.0f ) ) *
        rotate( glm::mat4( 1.0f ), glm::radians( 180.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) ) *
        rotate( glm::mat4( 1.0f ), glm::radians( -90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) *
        scale( glm::mat4( 1.0f ), glm::vec3( 0.3f ) );
    ShadowShader->uniformMat4fv( u::WorldMatrix, to_world );
    ShadowShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ShadowShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ShadowShader->uniform4fv( u::Material + m::EmissionColor, TigerObject->getEmissionColor() );
    ShadowShader->uniform4fv( u::Material + m::AmbientColor, TigerObject->getAmbientReflectionColor() );
    ShadowShader->uniform4fv( u::Material + m::DiffuseColor, TigerObject->getDiffuseReflectionColor() );
    ShadowShader->uniform4fv( u::Material + m::SpecularColor, TigerObject->getSpecularReflectionColor() );
    ShadowShader->uniform1f( u::Material + m::SpecularExponent, TigerObject->getSpecularReflectionExponent() );
    glBindTextureUnit( 0, TigerObject->getTextureID( 0 ) );
    glBindVertexArray( TigerObject->getVAO() );
    glDrawArrays( TigerObject->getDrawMode(), 0, TigerObject->getVertexNum() );

    to_world =
        translate( glm::mat4( 1.0f ), glm::vec3( 250.0f, -5.0f, 180.0f ) ) *
        scale( glm::mat4( 1.0f ), glm::vec3( 20.0f ) );
    ShadowShader->uniformMat4fv( u::WorldMatrix, to_world );
    ShadowShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ShadowShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ShadowShader->uniform4fv( u::Material + m::EmissionColor, PandaObject->getEmissionColor() );
    ShadowShader->uniform4fv( u::Material + m::AmbientColor, PandaObject->getAmbientReflectionColor() );
    ShadowShader->uniform4fv( u::Material + m::DiffuseColor, PandaObject->getDiffuseReflectionColor() );
    ShadowShader->uniform4fv( u::Material + m::SpecularColor, PandaObject->getSpecularReflectionColor() );
    ShadowShader->uniform1f( u::Material + m::SpecularExponent, PandaObject->getSpecularReflectionExponent() );
    glBindTextureUnit( 0, PandaObject->getTextureID( 0 ) );
    glBindVertexArray( PandaObject->getVAO() );
    glDrawArrays( PandaObject->getDrawMode(), 0, PandaObject->getVertexNum() );

    to_world =
        translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, 0.0f, 512.0f ) ) *
        rotate( glm::mat4( 1.0f ), glm::radians( -90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) *
        scale( glm::mat4( 1.0f ), glm::vec3( 512.0f ) );
    ShadowShader->uniformMat4fv( u::WorldMatrix, to_world );
    ShadowShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ShadowShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ShadowShader->uniform4fv( u::Material + m::EmissionColor, GroundObject->getEmissionColor() );
    ShadowShader->uniform4fv( u::Material + m::AmbientColor, GroundObject->getAmbientReflectionColor() );
    ShadowShader->uniform4fv( u::Material + m::DiffuseColor, GroundObject->getDiffuseReflectionColor() );
    ShadowShader->uniform4fv( u::Material + m::SpecularColor, GroundObject->getSpecularReflectionColor() );
    ShadowShader->uniform1f( u::Material + m::SpecularExponent, GroundObject->getSpecularReflectionExponent() );
    glBindTextureUnit( 0, GroundObject->getTextureID( 0 ) );
    glBindVertexArray( GroundObject->getVAO() );
    glDrawArrays( GroundObject->getDrawMode(), 0, GroundObject->getVertexNum() );
}

void C10ShadowMapping::render() const
{
    const float light_x = 1024.0f * std::cos( LightTheta ) + 256.0f;
    const float light_z = 1024.0f * std::sin( LightTheta ) + 256.0f;
    Lights->setLightPosition( glm::vec4( light_x, 200.0f, light_z, 1.0f ), 0 );

    drawDepthMapFromLightView( 0 );
    drawShadow( 0 );
}

void C10ShadowMapping::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    setLights();
    setGroundObject();
    setTigerObject();
    setPandaObject();
    setDepthFrameBuffer();

    while (!glfwWindowShouldClose( Window )) {
        render();

        LightTheta += 0.01f;
        if (LightTheta >= 360.0f) LightTheta -= 360.0f;

        glfwSwapBuffers( Window );
        glfwPollEvents();
    }
    glfwDestroyWindow( Window );
}

int main()
{
    C10ShadowMapping renderer{};
    renderer.play();
    return 0;
}