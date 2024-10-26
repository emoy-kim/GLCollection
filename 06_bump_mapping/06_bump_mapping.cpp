#include "06_bump_mapping.h"

#include <unistd.h>

C06BumpMapping::C06BumpMapping()
    : UseBumpMapping( true ),
      NormalTextureIndex( -1 ),
      LightTheta( 0.0f ),
      Lights( std::make_unique<LightGL>() ),
      ObjectShader( std::make_unique<ShaderGL>() ),
      BoxBlurShader( std::make_unique<ShaderGL>() ),
      NormalMapShader( std::make_unique<ShaderGL>() )
{
    MainCamera = std::make_unique<CameraGL>(
        glm::vec3( 1.5f, 1.5f, 7.0f ),
        glm::vec3( 1.5f, 1.5f, 0.0f ),
        glm::vec3( 0.0f, 1.0f, 0.0f )
    );
    MainCamera->update3DCamera( FrameWidth, FrameHeight );

    const std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/06_bump_mapping/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/bump_mapping.vert" ).c_str(),
        std::string( shader_directory_path + "/bump_mapping.frag" ).c_str()
    );
    BoxBlurShader->setComputeShader( std::string( shader_directory_path + "/box_blur.comp" ).c_str() );
    NormalMapShader->setComputeShader( std::string( shader_directory_path + "/normal_map.comp" ).c_str() );
    glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
}

void C06BumpMapping::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
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
            break;
        case GLFW_KEY_B:
            UseBumpMapping = !UseBumpMapping;
            std::cout << "Bump Mapping Turned " << (UseBumpMapping ? "On!\n" : "Off!\n");
            break;
        case GLFW_KEY_P: {
            const glm::vec3 pos = MainCamera->getCameraPosition();
            std::cout << "Camera Position: " << pos.x << ", " << pos.y << ", " << pos.z << "\n";
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

void C06BumpMapping::setLights() const
{
    glm::vec4 light_position( 0.5f, 0.5f, 0.2f, 1.0f );
    glm::vec4 ambient_color( 0.9f, 0.9f, 0.9f, 1.0f );
    glm::vec4 diffuse_color( 0.9f, 0.9f, 0.9f, 1.0f );
    glm::vec4 specular_color( 0.9f, 0.9f, 0.9f, 1.0f );
    Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );

    light_position = glm::vec4( 1.5f, 1.5f, 15.0f, 1.0f );
    ambient_color = glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f );
    diffuse_color = glm::vec4( 0.5f, 0.5f, 0.5f, 1.0f );
    specular_color = glm::vec4( 0.8f, 0.8f, 0.8f, 1.0f );
    constexpr glm::vec3 spotlight_direction( 0.0f, 0.0f, -1.0f );
    constexpr float spotlight_exponent = 128;
    constexpr float spotlight_cutoff_angle_in_degree = 10.0f;
    Lights->addLight(
        light_position,
        ambient_color,
        diffuse_color,
        specular_color,
        spotlight_direction,
        spotlight_exponent,
        spotlight_cutoff_angle_in_degree
    );
}

void C06BumpMapping::createNormalMap(ObjectGL* object)
{
    GLuint texture_id = object->getTextureID( 0 );
    const glm::ivec2 size = object->getTextureSize( texture_id );
    object->addTexture( size.x, size.y );
    object->addTexture( size.x, size.y );

    int t = 0;
    const std::array<GLuint, 2> textures{ object->getTextureID( 1 ), object->getTextureID( 2 ) };
    glUseProgram( BoxBlurShader->getShaderProgram() );
    BoxBlurShader->uniform1f( BoxBlurShader::BlurRadius, 3.0f );
    for (int i = 0; i < 3; ++i) {
        BoxBlurShader->uniform1i( BoxBlurShader::IsHorizontal, 1 );
        glBindImageTexture( 0, texture_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8 );
        glBindImageTexture( 1, textures[t], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8 );
        glDispatchCompute( getGroupSize( size.y ), 1, 1 );
        glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
        texture_id = textures[t];
        t ^= 1;

        BoxBlurShader->uniform1i( BoxBlurShader::IsHorizontal, 0 );
        glBindImageTexture( 0, texture_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8 );
        glBindImageTexture( 1, textures[t], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8 );
        glDispatchCompute( getGroupSize( size.x ), 1, 1 );
        glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
        texture_id = textures[t];
        t ^= 1;
    }

    glUseProgram( NormalMapShader->getShaderProgram() );
    glBindImageTexture( 0, texture_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8 );
    glBindImageTexture( 1, textures[t], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8 );
    glDispatchCompute( getGroupSize( size.x ), getGroupSize( size.y ), 1 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
    NormalTextureIndex = t + 1;
}

void C06BumpMapping::setWallObjects()
{
    const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/06_bump_mapping/samples/";
    for (size_t i = 0; i < WallObjects.size(); ++i) {
        WallObjects[i] = std::make_unique<ObjectGL>();
        const std::string texture_path = sample_directory_path + std::to_string( i ) + ".jpg";
        WallObjects[i]->setSquareObject( GL_TRIANGLES, texture_path );
        WallObjects[i]->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
        createNormalMap( WallObjects[i].get() );
    }
}

void C06BumpMapping::drawWallObject(const ObjectGL* object, const glm::mat4& to_world) const
{
    using u = BumpMappingShader::UNIFORM;
    using l = ShaderGL::LIGHT_UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    glUseProgram( ObjectShader->getShaderProgram() );
    ObjectShader->uniformMat4fv( u::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniform1i( u::UseBumpMapping, UseBumpMapping ? 1 : 0 );
    ObjectShader->uniform4fv( u::Material + m::EmissionColor, object->getEmissionColor() );
    ObjectShader->uniform4fv( u::Material + m::AmbientColor, object->getAmbientReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::DiffuseColor, object->getDiffuseReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::SpecularColor, object->getSpecularReflectionColor() );
    ObjectShader->uniform1f( u::Material + m::SpecularExponent, object->getSpecularReflectionExponent() );
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
    glBindTextureUnit( 0, object->getTextureID( 0 ) );
    glBindTextureUnit( 1, object->getTextureID( NormalTextureIndex ) );
    glBindVertexArray( object->getVAO() );
    glDrawArrays( object->getDrawMode(), 0, object->getVertexNum() );
}

void C06BumpMapping::render() const
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    const float light_x = 1.25f * std::cos( LightTheta ) + 1.5f;
    const float light_y = 1.25f * std::sin( LightTheta ) + 1.5f;
    Lights->setLightPosition( glm::vec4( light_x, light_y, 0.2f, 1.0f ), 0 );

    const glm::mat4 to_up = translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    glm::mat4 to_world_3rd_row( 1.0f );
    glm::mat4 to_world_2nd_row = to_up;
    glm::mat4 to_world_1st_row = to_world_2nd_row * to_up;

    drawWallObject( WallObjects[0].get(), to_world_3rd_row );
    drawWallObject( WallObjects[1].get(), to_world_2nd_row );
    drawWallObject( WallObjects[2].get(), to_world_1st_row );

    const glm::mat4 to_right = translate( glm::mat4( 1.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) );
    to_world_3rd_row *= to_right;
    to_world_2nd_row *= to_right;
    to_world_1st_row *= to_right;

    drawWallObject( WallObjects[3].get(), to_world_3rd_row );
    drawWallObject( WallObjects[4].get(), to_world_2nd_row );
    drawWallObject( WallObjects[5].get(), to_world_1st_row );

    to_world_3rd_row *= to_right;
    to_world_2nd_row *= to_right;
    to_world_1st_row *= to_right;

    drawWallObject( WallObjects[6].get(), to_world_3rd_row );
    drawWallObject( WallObjects[7].get(), to_world_2nd_row );
    drawWallObject( WallObjects[8].get(), to_world_1st_row );
}

void C06BumpMapping::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    setLights();
    setWallObjects();

    while (!glfwWindowShouldClose( Window )) {
        render();

        LightTheta += 0.05f;
        if (LightTheta >= 360.0f) LightTheta -= 360.0f;
        glfwSwapBuffers( Window );
        glfwPollEvents();
    }
    glfwDestroyWindow( Window );
}

int main()
{
    C06BumpMapping renderer{};
    renderer.play();
    return 0;
}