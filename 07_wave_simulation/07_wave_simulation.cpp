#include "07_wave_simulation.h"

C07WaveSimulation::C07WaveSimulation()
    : WaveTargetIndex( 0 ),
      WaveFactor( 20.0f ),
      WavePointNum( 100 ),
      WaveGridSize( 25, 25 ),
      WaveBuffers{},
      ObjectShader( std::make_unique<ShaderGL>() ),
      WaveShader( std::make_unique<ShaderGL>() ),
      WaveNormalShader( std::make_unique<ShaderGL>() ),
      WaveObject( std::make_unique<ObjectGL>() ),
      Lights( std::make_unique<LightGL>() )
{
    MainCamera = std::make_unique<CameraGL>(
        glm::vec3( 50.0f, 20.0f, 50.0f ),
        glm::vec3( 0.0f, 0.0f, 0.0f ),
        glm::vec3( 0.0f, 1.0f, 0.0f )
    );
    MainCamera->update3DCamera( FrameWidth, FrameHeight );

    std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/01_lighting/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/scene_shader.vert" ).c_str(),
        std::string( shader_directory_path + "/scene_shader.frag" ).c_str()
    );
    shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/07_wave_simulation/shaders";
    WaveShader->setComputeShader( std::string( shader_directory_path + "/wave.comp" ).c_str() );
    WaveNormalShader->setComputeShader( std::string( shader_directory_path + "/wave_normal.comp" ).c_str() );
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
}

void C07WaveSimulation::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
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

void C07WaveSimulation::setLights() const
{
    constexpr glm::vec4 light_position( 10.0f, 30.0f, 10.0f, 1.0f );
    constexpr glm::vec4 ambient_color( 0.9f, 0.9f, 0.9f, 1.0f );
    constexpr glm::vec4 diffuse_color( 0.7f, 0.7f, 0.9f, 1.0f );
    constexpr glm::vec4 specular_color( 0.9f, 0.9f, 0.9f, 1.0f );
    Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );
}

void C07WaveSimulation::setWaveObject()
{
    const float ds = 1.0f / static_cast<float>(WavePointNum.x - 1);
    const float dt = 1.0f / static_cast<float>(WavePointNum.y - 1);
    const float dx = static_cast<float>(WaveGridSize.x) * ds;
    const float dy = static_cast<float>(WaveGridSize.y) * dt;
    const auto mid_x = static_cast<float>(WavePointNum.x >> 1);
    const auto mid_y = static_cast<float>(WavePointNum.y >> 1);

    std::vector<glm::vec3> wave_vertices, wave_normals;
    std::vector<glm::vec2> wave_textures;
    for (int j = 0; j < WavePointNum.y; ++j) {
        const auto y = static_cast<float>(j);
        for (int i = 0; i < WavePointNum.x; ++i) {
            const auto x = static_cast<float>(i);
            wave_vertices.emplace_back( x * dx, 0.0f, y * dy );

            constexpr float initial_radius_squared = 64.0f;
            constexpr float initial_wave_factor = glm::pi<float>() / initial_radius_squared;
            const float distance_squared = (x - mid_x) * (x - mid_x) + (y - mid_y) * (y - mid_y);
            if (distance_squared <= initial_radius_squared) {
                constexpr float initial_wave_height = 4.0f;
                const float theta = std::sqrt( initial_wave_factor * distance_squared );
                wave_vertices.back().y = initial_wave_height * (std::cos( theta ) + 1.0f);
            }

            wave_normals.emplace_back( 0.0f, 0.0f, 0.0f );
            wave_textures.emplace_back( x * ds, y * dt );
        }
    }

    std::vector<GLuint> wave_indices;
    for (int j = 0; j < WavePointNum.y - 1; ++j) {
        for (int i = 0; i < WavePointNum.x; ++i) {
            wave_indices.emplace_back( (j + 1) * WavePointNum.x + i );
            wave_indices.emplace_back( j * WavePointNum.x + i );
        }
    }

    const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/07_wave_simulation";
    WaveObject->setObject(
        GL_TRIANGLE_STRIP,
        wave_vertices,
        wave_normals,
        wave_textures,
        wave_indices,
        std::string( sample_directory_path + "/water.png" )
    );
    WaveBuffers[0] = WaveObject->getVBO();
    const int buffer_size = WaveObject->getVertexNum() * 8;
    WaveBuffers[1] = WaveObject->addCustomBufferObject<GLfloat>( buffer_size );
    WaveBuffers[2] = WaveObject->addCustomBufferObject<GLfloat>( buffer_size );
    ObjectGL::copy<GLfloat>( WaveBuffers[0], WaveBuffers[1], 0, 0, buffer_size );

    WaveObject->setDiffuseReflectionColor( { 0.0f, 0.47f, 0.75f, 1.0f } );

    constexpr float delta_time = 0.001f;
    WaveFactor = WaveFactor * WaveFactor * delta_time * delta_time / dx;
}

void C07WaveSimulation::render()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( WaveShader->getShaderProgram() );
    WaveShader->uniform2iv( PointNum, WavePointNum );
    WaveShader->uniform1f( Factor, WaveFactor );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, WaveBuffers[WaveTargetIndex] );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, WaveBuffers[(WaveTargetIndex + 1) % 3] );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, WaveBuffers[(WaveTargetIndex + 2) % 3] );
    glDispatchCompute( getGroupSize( WavePointNum.x ), getGroupSize( WavePointNum.y ), 1 );
    glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

    glUseProgram( WaveNormalShader->getShaderProgram() );
    WaveNormalShader->uniform2iv( PointNum, WavePointNum );
    glDispatchCompute( getGroupSize( WavePointNum.x ), getGroupSize( WavePointNum.y ), 1 );
    glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

    WaveTargetIndex = (WaveTargetIndex + 1) % 3;

    using u = LightingShader::UNIFORM;
    using l = ShaderGL::LIGHT_UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    glUseProgram( ObjectShader->getShaderProgram() );
    ObjectShader->uniformMat4fv( u::WorldMatrix, glm::mat4( 1.0f ) );
    ObjectShader->uniformMat4fv( u::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        u::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix()
    );
    ObjectShader->uniform1i( u::UseTexture, 1 );
    ObjectShader->uniform4fv( u::Material + m::EmissionColor, WaveObject->getEmissionColor() );
    ObjectShader->uniform4fv( u::Material + m::AmbientColor, WaveObject->getAmbientReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::DiffuseColor, WaveObject->getDiffuseReflectionColor() );
    ObjectShader->uniform4fv( u::Material + m::SpecularColor, WaveObject->getSpecularReflectionColor() );
    ObjectShader->uniform1f( u::Material + m::SpecularExponent, WaveObject->getSpecularReflectionExponent() );
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
    glBindTextureUnit( 0, WaveObject->getTextureID( 0 ) );
    glBindVertexArray( WaveObject->getVAO() );
    for (int j = 0; j < WavePointNum.y - 1; ++j) {
        glDrawElements(
            WaveObject->getDrawMode(),
            WavePointNum.x * 2,
            GL_UNSIGNED_INT,
            reinterpret_cast<GLvoid*>(j * WavePointNum.x * 2 * sizeof( GLuint ))
        );
    }
}

void C07WaveSimulation::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    setLights();
    setWaveObject();

    while (!glfwWindowShouldClose( Window )) {
        render();

        glfwPollEvents();
        glfwSwapBuffers( Window );
    }
    glfwDestroyWindow( Window );
}

int main()
{
    C07WaveSimulation renderer{};
    renderer.play();
    return 0;
}