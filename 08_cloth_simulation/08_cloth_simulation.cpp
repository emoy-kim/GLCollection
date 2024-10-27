#include "08_cloth_simulation.h"

C08ClothSimulation::C08ClothSimulation()
    : SphereRadius( 20.0f ),
      ClothTargetIndex( 0 ),
      ClothPointNumSize( 100, 100 ),
      ClothGridSize( 50, 50 ),
      SpherePosition( 0.0f, 0.0f, 0.0f ),
      ClothWorldMatrix( translate( glm::mat4( 1.0f ), glm::vec3( 50.0f, 100.0f, 0.0f ) ) ),
      SphereWorldMatrix( translate( glm::mat4( 1.0f ), glm::vec3( 100.0f, 30.0f, 20.0f ) ) ),
      ClothBuffers{},
      ObjectShader( std::make_unique<ShaderGL>() ),
      ClothShader( std::make_unique<ShaderGL>() ),
      ClothObject( std::make_unique<ObjectGL>() ),
      SphereObject( std::make_unique<ObjectGL>() ),
      Lights( std::make_unique<LightGL>() )
{
    MainCamera = std::make_unique<CameraGL>(
        glm::vec3( 80.0f, 50.0f, 300.0f ),
        glm::vec3( 80.0f, 50.0f, 0.0f ),
        glm::vec3( 0.0f, 1.0f, 0.0f )
    );
    MainCamera->update3DCamera( FrameWidth, FrameHeight );

    std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/01_lighting/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/scene_shader.vert" ).c_str(),
        std::string( shader_directory_path + "/scene_shader.frag" ).c_str()
    );
    shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/08_cloth_simulation/shaders";
    ClothShader->setComputeShader( std::string( shader_directory_path + "/cloth.comp" ).c_str() );
    glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
}

void C08ClothSimulation::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    switch (key) {
        case GLFW_KEY_I:
            MainCamera->resetCamera();
            break;
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

void C08ClothSimulation::setLights() const
{
    constexpr glm::vec4 light_position( 30.0f, 500.0f, 30.0f, 1.0f );
    constexpr glm::vec4 ambient_color( 1.0f, 1.0f, 1.0f, 1.0f );
    constexpr glm::vec4 diffuse_color( 0.7f, 0.7f, 0.7f, 1.0f );
    constexpr glm::vec4 specular_color( 0.9f, 0.9f, 0.9f, 1.0f );
    Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );
}

void C08ClothSimulation::setClothObject()
{
    const float ds = 1.0f / static_cast<float>(ClothPointNumSize.x - 1);
    const float dt = 1.0f / static_cast<float>(ClothPointNumSize.y - 1);
    const float dx = static_cast<float>(ClothGridSize.x) * ds;
    const float dy = static_cast<float>(ClothGridSize.y) * dt;

    std::vector<glm::vec3> cloth_vertices, cloth_normals;
    std::vector<glm::vec2> cloth_textures;
    for (int j = 0; j < ClothPointNumSize.y; ++j) {
        const auto y = static_cast<float>(j);
        for (int i = 0; i < ClothPointNumSize.x; ++i) {
            const auto x = static_cast<float>(i);
            cloth_vertices.emplace_back( x * dx, 0.0f, y * dy );
            cloth_normals.emplace_back( 0.0f, 1.0f, 0.0f );
            cloth_textures.emplace_back( x * ds, y * dt );
        }
    }

    std::vector<GLuint> cloth_indices;
    for (int j = 0; j < ClothPointNumSize.y - 1; ++j) {
        for (int i = 0; i < ClothPointNumSize.x; ++i) {
            cloth_indices.emplace_back( (j + 1) * ClothPointNumSize.x + i );
            cloth_indices.emplace_back( j * ClothPointNumSize.x + i );
        }
    }

    const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/08_cloth_simulation/samples";
    ClothObject->setObject(
        GL_TRIANGLE_STRIP,
        cloth_vertices,
        cloth_normals,
        cloth_textures,
        cloth_indices,
        std::string( sample_directory_path + "/cloth.jpg" )
    );
    ClothBuffers[0] = ClothObject->getVBO();
    const int buffer_size = ClothObject->getVertexNum() * 8;
    ClothBuffers[1] = ClothObject->addCustomBufferObject<GLfloat>( buffer_size );
    ClothBuffers[2] = ClothObject->addCustomBufferObject<GLfloat>( buffer_size );
    ObjectGL::copy<GLfloat>( ClothBuffers[0], ClothBuffers[1], 0, 0, buffer_size );

    ClothObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void C08ClothSimulation::setSphereObject() const
{
    std::vector<glm::vec3> sphere_vertices, sphere_normals;
    std::vector<glm::vec2> sphere_textures;
    const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/08_cloth_simulation/samples";
    if (ObjectGL::readObjectFile(
        sphere_vertices,
        sphere_normals,
        sphere_textures,
        std::string( sample_directory_path + "/sphere.obj" )
    )) {
        SphereObject->setObject(
            GL_TRIANGLES,
            sphere_vertices,
            sphere_normals,
            sphere_textures,
            std::string( sample_directory_path + "/sphere.jpg" )
        );
    }
    else throw std::runtime_error( "Could not read object file!" );

    SphereObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void C08ClothSimulation::applyForces()
{
    using u = ClothShader::UNIFORM;

    glUseProgram( ClothShader->getShaderProgram() );
    const float rest_length = static_cast<float>(ClothGridSize.x) / static_cast<float>(ClothPointNumSize.x);
    ClothShader->uniform1f( u::SpringRestLength, rest_length );
    ClothShader->uniform1f( u::SpringStiffness, 10.0f );
    ClothShader->uniform1f( u::SpringDamping, -0.5f );
    ClothShader->uniform1f( u::ShearRestLength, 1.5f * rest_length );
    ClothShader->uniform1f( u::ShearStiffness, 10.0f );
    ClothShader->uniform1f( u::ShearDamping, -0.5f );
    ClothShader->uniform1f( u::FlexionRestLength, 2.0f * rest_length );
    ClothShader->uniform1f( u::FlexionStiffness, 5.0f );
    ClothShader->uniform1f( u::FlexionDamping, -0.5f );
    ClothShader->uniform1f( u::GravityConstant, -9.8f );
    ClothShader->uniform1f( u::GravityDamping, -0.3f );
    ClothShader->uniform1f( u::DeltaTime, 0.1f );
    ClothShader->uniform1f( u::Mass, 1.0f );
    ClothShader->uniform2iv( u::ClothPointNumSize, ClothPointNumSize );
    ClothShader->uniformMat4fv( u::ClothWorldMatrix, ClothWorldMatrix );
    ClothShader->uniform1f( u::SphereRadius, SphereRadius );
    ClothShader->uniform3fv( u::SpherePosition, SpherePosition );
    ClothShader->uniformMat4fv( u::SphereWorldMatrix, SphereWorldMatrix );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, ClothBuffers[ClothTargetIndex] );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, ClothBuffers[(ClothTargetIndex + 1) % 3] );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, ClothBuffers[(ClothTargetIndex + 2) % 3] );
    glDispatchCompute( getGroupSize( ClothPointNumSize.x ), getGroupSize( ClothPointNumSize.y ), 1 );
    glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

    ClothTargetIndex = (ClothTargetIndex + 1) % 3;
}

void C08ClothSimulation::drawClothObject() const
{
    using u = LightingShader::UNIFORM;
    using l = ShaderGL::LIGHT_UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    ObjectShader->uniformMat4fv( u::WorldMatrix, ClothWorldMatrix );
    ObjectShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * ClothWorldMatrix
    );
    ObjectShader->uniform1i( u::UseTexture, 1 );
    ObjectShader->uniform4fv( u::Material + m::EmissionColor, ClothObject->getEmissionColor() );
    ObjectShader->uniform4fv( u::Material + m::AmbientColor, ClothObject->getAmbientReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::DiffuseColor, ClothObject->getDiffuseReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::SpecularColor, ClothObject->getSpecularReflectionColor() );
    ObjectShader->uniform1f( u::Material + m::SpecularExponent, ClothObject->getSpecularReflectionExponent() );
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
    glBindTextureUnit( 0, ClothObject->getTextureID( 0 ) );
    glBindVertexArray( ClothObject->getVAO() );
    for (int j = 0; j < ClothPointNumSize.y - 1; ++j) {
        glDrawElements(
            ClothObject->getDrawMode(),
            ClothPointNumSize.x * 2,
            GL_UNSIGNED_INT,
            reinterpret_cast<GLvoid*>(j * ClothPointNumSize.x * 2 * sizeof( GLuint ))
        );
    }
}

void C08ClothSimulation::drawSphereObject() const
{
    using u = LightingShader::UNIFORM;
    using l = ShaderGL::LIGHT_UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    const glm::mat4 to_world = SphereWorldMatrix * translate( glm::mat4( 1.0f ), SpherePosition );
    ObjectShader->uniformMat4fv( u::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniform1i( u::UseTexture, 1 );
    ObjectShader->uniform4fv( u::Material + m::EmissionColor, SphereObject->getEmissionColor() );
    ObjectShader->uniform4fv( u::Material + m::AmbientColor, SphereObject->getAmbientReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::DiffuseColor, SphereObject->getDiffuseReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::SpecularColor, SphereObject->getSpecularReflectionColor() );
    ObjectShader->uniform1f( u::Material + m::SpecularExponent, SphereObject->getSpecularReflectionExponent() );
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
    glBindTextureUnit( 0, SphereObject->getTextureID( 0 ) );
    glBindVertexArray( SphereObject->getVAO() );
    glDrawArrays( SphereObject->getDrawMode(), 0, SphereObject->getVertexNum() );
}

void C08ClothSimulation::render()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    applyForces();

    glViewport( 0, 0, FrameWidth, FrameHeight );
    glUseProgram( ObjectShader->getShaderProgram() );
    drawClothObject();
    drawSphereObject();
}

void C08ClothSimulation::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    setLights();
    setClothObject();
    setSphereObject();

    while (!glfwWindowShouldClose( Window )) {
        render();

        glfwSwapBuffers( Window );
        glfwPollEvents();
    }
    glfwDestroyWindow( Window );
}

int main()
{
    C08ClothSimulation renderer{};
    renderer.play();
    return 0;
}