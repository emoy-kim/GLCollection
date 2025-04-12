#include "13_environment_mapping.h"

C13EnvironmentMapping::C13EnvironmentMapping()
    : DrawMovingObject( false ),
      ActivatedLightIndex( 0 ),
      TigerIndex( 0 ),
      TigerRotationAngle( 180 ),
      EnvironmentWidth( 0 ),
      EnvironmentHeight( 0 ),
      EnvironmentRadius( 50.0f ),
      AdjustedIntensities( nullptr ),
      ImageBuffer( nullptr ),
      LatitudeLongitude( nullptr ),
      ObjectShader( std::make_unique<ShaderGL>() ),
      EnvironmentShader( std::make_unique<ShaderGL>() ),
      EnvironmentObject( std::make_unique<ObjectGL>() ),
      CowObject( std::make_unique<ObjectGL>() ),
      Lights( std::make_unique<LightGL>() )
{
    MainCamera = std::make_unique<CameraGL>(
        glm::vec3( -20.0f, 5.0f, 20.0f ),
        glm::vec3( 5.0f, 25.0f, -15.0f ),
        glm::vec3( 0.0f, 1.0f, 0.0f )
    );
    MainCamera->setMoveSensitivity( 0.01f );
    MainCamera->update3DCamera( FrameWidth, FrameHeight );

    const std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/13_environment_mapping/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/lighting.vert" ).c_str(),
        std::string( shader_directory_path + "/lighting.frag" ).c_str()
    );
    EnvironmentShader->setShader(
        std::string( shader_directory_path + "/environment_map.vert" ).c_str(),
        std::string( shader_directory_path + "/environment_map.frag" ).c_str()
    );
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
}

C13EnvironmentMapping::~C13EnvironmentMapping()
{
    delete [] AdjustedIntensities;
    delete [] ImageBuffer;
    delete [] LatitudeLongitude;
}

void C13EnvironmentMapping::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    switch (key) {
        case GLFW_KEY_UP:
            MainCamera->moveForward( 100 );
            break;
        case GLFW_KEY_DOWN:
            MainCamera->moveForward( -100 );
            break;
        case GLFW_KEY_LEFT:
            MainCamera->moveHorizontally( 100 );
            break;
        case GLFW_KEY_RIGHT:
            MainCamera->moveHorizontally( -100 );
            break;
        case GLFW_KEY_W:
            MainCamera->moveVertically( -100 );
            break;
        case GLFW_KEY_S:
            MainCamera->moveVertically( 100 );
            break;
        case GLFW_KEY_I:
            MainCamera->resetCamera();
            break;
        case GLFW_KEY_L:
            Lights->toggleLightSwitch();
            ActivatedLightIndex = 0;
            std::cout << "Light Turned " << (Lights->isLightOn() ? "On!\n" : "Off!\n");
            break;
        case GLFW_KEY_ENTER:
            if (Lights->isLightOn()) {
                ActivatedLightIndex++;
                if (ActivatedLightIndex == Lights->getTotalLightNum()) ActivatedLightIndex = 0;
                std::cout << "Activate Light " << ActivatedLightIndex << "\n";
            }
            break;
        case GLFW_KEY_SPACE:
            DrawMovingObject = !DrawMovingObject;
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

glm::vec4 C13EnvironmentMapping::getPixel(const uint8_t* row_ptr, int x)
{
    return {
        row_ptr[x * 4], row_ptr[x * 4 + 1], row_ptr[x * 4 + 2], row_ptr[x * 4 + 3]
    };
}

glm::vec4 C13EnvironmentMapping::getBilinearInterpolatedColor(const glm::vec2& point) const
{
    const int x0 = static_cast<int>(std::floor( point.x ));
    const int y0 = static_cast<int>(std::floor( point.y ));
    const int x1 = std::min( x0 + 1, EnvironmentWidth - 1 );
    const int y1 = std::min( y0 + 1, EnvironmentHeight - 1 );
    const auto tx = point.x - static_cast<float>(x0);
    const auto ty = point.y - static_cast<float>(y0);

    const auto* curr_row = ImageBuffer + y0 * EnvironmentWidth * 4;
    const auto* next_row = ImageBuffer + y1 * EnvironmentWidth * 4;
    return {
        getPixel( curr_row, x0 ) * (1.0f - tx) * (1.0f - ty) +
        getPixel( curr_row, x1 ) * tx * (1.0f - ty) +
        getPixel( next_row, x0 ) * (1.0f - tx) * ty +
        getPixel( next_row, x1 ) * tx * ty
    };
}

void C13EnvironmentMapping::convertFisheye(const std::string& file_path)
{
    std::cout << ">> Convert Fisheye Image to Longitude-Latitude Image...\r";

    const FREE_IMAGE_FORMAT format = FreeImage_GetFileType( file_path.c_str(), 0 );
    FIBITMAP* texture = FreeImage_Load( format, file_path.c_str() );
    const uint n_bits_per_pixel = FreeImage_GetBPP( texture );
    FIBITMAP* texture_converted = n_bits_per_pixel == 32 ? texture : FreeImage_ConvertTo32Bits( texture );
    const uint8_t* image = FreeImage_GetBits( texture_converted );
    EnvironmentWidth = static_cast<int>(FreeImage_GetWidth( texture_converted ));
    EnvironmentHeight = static_cast<int>(FreeImage_GetHeight( texture_converted ));
    LatitudeLongitude = new uint8_t[EnvironmentWidth * EnvironmentHeight * 4];
    ImageBuffer = new uint8_t[EnvironmentWidth * EnvironmentHeight * 4];
    std::memcpy( ImageBuffer, image, EnvironmentWidth * EnvironmentHeight * 4 );
    FreeImage_Unload( texture_converted );
    if (n_bits_per_pixel != 32) FreeImage_Unload( texture );

    const auto w = static_cast<float>(EnvironmentWidth);
    const auto h = static_cast<float>(EnvironmentHeight);
    for (int j = 0; j < EnvironmentHeight; ++j) {
        uint8_t* converted_ptr = LatitudeLongitude + j * EnvironmentWidth * 4;
        for (int i = 0; i < EnvironmentWidth; ++i) {
            const glm::vec2 texture_point(
                static_cast<float>(i) / (w - 1.0f),
                static_cast<float>(j) / (h - 1.0f)
            );

            const float phi = texture_point.x * glm::two_pi<float>();
            const float theta = texture_point.y * glm::pi<float>() - glm::half_pi<float>();
            const float sin_phi = std::sin( phi );
            const float cos_phi = std::cos( phi );
            const float sin_theta = std::sin( theta );
            const float cos_theta = std::cos( theta );
            const glm::vec3 on_sphere( -sin_theta * cos_phi, cos_theta, -sin_theta * sin_phi );

            const glm::vec2 fisheye_point( on_sphere.x, on_sphere.z );
            if (fisheye_point.x * fisheye_point.x + fisheye_point.y * fisheye_point.y > 1.0f) {
                converted_ptr[4 * i] = converted_ptr[4 * i + 1] = converted_ptr[4 * i + 2] = 0;
                converted_ptr[4 * i + 3] = 255;
                continue;
            }

            const glm::vec2 fisheye_image_point(
                (fisheye_point.x + 1.0f) * 0.5f * (w - 1.0f),
                (fisheye_point.y + 1.0f) * 0.5f * (h - 1.0f)
            );
            if (fisheye_image_point.x < 0.0f || fisheye_image_point.x >= w ||
                fisheye_image_point.y < 0.0f || fisheye_image_point.y >= h) {
                converted_ptr[4 * i] = converted_ptr[4 * i + 1] = converted_ptr[4 * i + 2] = 0;
                converted_ptr[4 * i + 3] = 255;
                continue;
            }

            const glm::vec4 color = getBilinearInterpolatedColor( fisheye_image_point );
            converted_ptr[4 * i] = static_cast<uint8_t>(color.x);
            converted_ptr[4 * i + 1] = static_cast<uint8_t>(color.y);
            converted_ptr[4 * i + 2] = static_cast<uint8_t>(color.z);
            converted_ptr[4 * i + 3] = static_cast<uint8_t>(color.w);
        }
    }
    std::cout << ">> Convert Fisheye Image to Longitude-Latitude Image...(Done)\n";
}

void C13EnvironmentMapping::calculateDeltaXDividingIntensityInHalf(
    int& dx,
    int start,
    int end,
    int block_width,
    float half_intensity
) const
{
    float half_sum_from_x = 0.0f;
    while (half_sum_from_x < half_intensity && dx < block_width) {
        dx++;
        for (int j = start; j < end; ++j) {
            const float* adjusted_ptr = AdjustedIntensities + j * EnvironmentWidth;
            half_sum_from_x += adjusted_ptr[dx];
        }
    }
}

void C13EnvironmentMapping::calculateDeltaYDividingIntensityInHalf(
    int& dy,
    int start,
    int end,
    int block_height,
    float half_intensity
) const
{
    float half_sum_from_y = 0.0f;
    while (half_sum_from_y < half_intensity && dy < block_height) {
        dy++;
        const float* adjusted_ptr = AdjustedIntensities + dy * EnvironmentWidth;
        for (int i = start; i < end; ++i) {
            half_sum_from_y += adjusted_ptr[i];
        }
    }
}

void C13EnvironmentMapping::medianCut(std::map<float, glm::ivec2>& light_infos, const Rect& block, int iteration) const
{
    if (block.TopLeft.x >= EnvironmentWidth || block.TopLeft.y >= EnvironmentHeight ||
        block.Size.x == 0 || block.Size.y == 0)
        return;

    float half_intensity = 0.0f;
    for (int j = block.TopLeft.y; j < block.TopLeft.y + block.Size.y; ++j) {
        const float* adjusted_ptr = AdjustedIntensities + j * EnvironmentWidth;
        for (int i = block.TopLeft.x; i < block.TopLeft.x + block.Size.x; ++i) {
            half_intensity += adjusted_ptr[i];
        }
    }
    half_intensity *= 0.5f;

    int dx = -1, dy = -1;
    if (iteration == 0) {
        calculateDeltaXDividingIntensityInHalf(
            dx, 0, block.Size.y, block.Size.x, half_intensity
        );
        calculateDeltaYDividingIntensityInHalf(
            dy, 0, block.Size.x, block.Size.y, half_intensity
        );

        const glm::ivec2 light_position( block.TopLeft.x + dx, block.TopLeft.y + dy );
        const float intensity = AdjustedIntensities[light_position.y * EnvironmentWidth + light_position.x];
        light_infos.emplace( intensity, light_position );
    }
    else if (block.Size.x > block.Size.y) {
        calculateDeltaXDividingIntensityInHalf(
            dx, 0, block.Size.y, block.Size.x, half_intensity
        );

        const Rect left_block( block.TopLeft.x, block.TopLeft.y, dx, block.Size.y );
        const Rect right_block( block.TopLeft.x + dx, block.TopLeft.y, block.Size.x - dx, block.Size.y );
        medianCut( light_infos, left_block, iteration - 1 );
        medianCut( light_infos, right_block, iteration - 1 );
    }
    else {
        calculateDeltaYDividingIntensityInHalf(
            dy, 0, block.Size.x, block.Size.y, half_intensity
        );

        const Rect top_block( block.TopLeft.x, block.TopLeft.y, block.Size.x, dy );
        const Rect bottom_block( block.TopLeft.x, block.TopLeft.y + dy, block.Size.x, block.Size.y - dy );
        medianCut( light_infos, top_block, iteration - 1 );
        medianCut( light_infos, bottom_block, iteration - 1 );
    }
}

std::vector<glm::ivec2> C13EnvironmentMapping::estimateLightPoints()
{
    std::cout << ">> Find Light Points...\r";

    AdjustedIntensities = new float[EnvironmentWidth * EnvironmentHeight];
    const float scale = 1.0f / static_cast<float>(EnvironmentHeight - 1);
    for (int j = 0; j < EnvironmentHeight; ++j) {
        const float adjuster = std::sin( static_cast<float>(j) * scale * glm::pi<float>() );
        const u_int8_t* ptr = LatitudeLongitude + j * EnvironmentWidth * 4;
        float* adjusted_ptr = AdjustedIntensities + j * EnvironmentWidth;
        for (int i = 0; i < EnvironmentWidth; ++i) {
            const float gray = static_cast<float>(ptr[4 * i] + ptr[4 * i + 1] + ptr[4 * i + 2]) / 3.0f;
            adjusted_ptr[i] = adjuster * gray;
        }
    }

    constexpr int light_num_to_find = 5;
    constexpr int light_num = getNextHighestPowerOf2( light_num_to_find );
    constexpr int iteration = light_num == 0 ? 0 : static_cast<int>(std::log2( light_num ));

    std::map<float, glm::ivec2> light_infos;
    medianCut( light_infos, { 0, 0, EnvironmentWidth, EnvironmentHeight }, iteration );

    std::vector<glm::ivec2> light_points;
    const auto end = std::next( light_infos.begin(), light_num_to_find );
    for (auto it = light_infos.begin(); it != end; ++it) {
        light_points.emplace_back( it->second );
    }

    std::cout << ">> Find Light Points...(Done)\n";
    return light_points;
}

void C13EnvironmentMapping::findLightsFromImage()
{
    convertFisheye( std::string( CMAKE_SOURCE_DIR ) + "/13_environment_mapping/samples/fisheye/sky.jpg" );

    std::vector<glm::ivec2> light_points = estimateLightPoints();

    constexpr float color_scale = 1.0f / 255.0f;
    const float width_scale = glm::two_pi<float>() / static_cast<float>(EnvironmentWidth - 1);
    const float height_scale = glm::pi<float>() / static_cast<float>(EnvironmentHeight - 1);
    constexpr glm::vec4 ambient_color( 1.0f, 1.0f, 1.0f, 1.0f );
    constexpr glm::vec4 specular_color( 0.9f, 0.9f, 0.9f, 1.0f );
    for (const auto& light : light_points) {
        const u_int8_t* ptr = ImageBuffer + light.y * EnvironmentWidth * 4;
        const glm::vec4 diffuse_color(
            static_cast<float>(ptr[4 * light.x]) * color_scale,
            static_cast<float>(ptr[4 * light.x + 1]) * color_scale,
            static_cast<float>(ptr[4 * light.x + 2]) * color_scale,
            static_cast<float>(ptr[4 * light.x + 3]) * color_scale
        );
        const auto x = static_cast<float>(light.x) * width_scale;
        const auto y = static_cast<float>(light.y) * height_scale - glm::half_pi<float>();
        const glm::vec4 light_position(
            -std::sin( y ) * std::cos( x ),
            std::cos( y ),
            -std::sin( y ) * std::sin( x ),
            1.0f
        );
        Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );
    }
}

void C13EnvironmentMapping::setEnvironmentObject() const
{
    std::vector<glm::vec3> hemisphere_vertices;
    const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/13_environment_mapping/samples";
    if (ObjectGL::readObjectFile(
        hemisphere_vertices,
        std::string( sample_directory_path + "/objects/hemisphere.obj" )
    )) {
        EnvironmentObject->setObject( GL_TRIANGLES, hemisphere_vertices );
        EnvironmentObject->addTexture( ImageBuffer, EnvironmentWidth, EnvironmentHeight );
        EnvironmentObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
    }
    else throw std::runtime_error( "Could not read text file!" );
}

void C13EnvironmentMapping::setMovingTigerObjects()
{
    MovingTigerObjects.clear();
    const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/13_environment_mapping/samples";
    for (int t = 0; t < 12; ++t) {
        std::vector<glm::vec3> tiger_vertices, tiger_normals;
        std::vector<glm::vec2> tiger_textures;
        if (ObjectGL::readTextFile(
            tiger_vertices,
            tiger_normals,
            tiger_textures,
            sample_directory_path + "/objects/tiger" + std::to_string( t ) + ".txt"
        )) {
            MovingTigerObjects.emplace_back( std::make_unique<ObjectGL>() );
            MovingTigerObjects[t]->setObject( GL_TRIANGLES, tiger_vertices, tiger_normals );
            MovingTigerObjects[t]->addTexture( ImageBuffer, EnvironmentWidth, EnvironmentHeight );
            MovingTigerObjects[t]->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
        }
        else throw std::runtime_error( "Could not read text file!" );
    }
}

void C13EnvironmentMapping::setCowObject() const
{
    std::vector<glm::vec3> cow_vertices, cow_normals;
    const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/13_environment_mapping/samples";
    if (ObjectGL::readTextFile(
        cow_vertices,
        cow_normals,
        sample_directory_path + "/objects/cow.txt"
    )) {
        CowObject->setObject( GL_TRIANGLES, cow_vertices, cow_normals );
        CowObject->addTexture( ImageBuffer, EnvironmentWidth, EnvironmentHeight );
        CowObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
    }
    else throw std::runtime_error( "Could not read text file!" );
}

void C13EnvironmentMapping::drawMovingTiger(float scale_factor) const
{
    using l = ShaderGL::LIGHT_UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    glUseProgram( ObjectShader->getShaderProgram() );
    const auto theta = static_cast<float>(TigerRotationAngle);
    const glm::mat4 to_world =
        translate( glm::mat4( 1.0f ), glm::vec3( 20.0f, 20.0f, -20.0f ) ) *
        rotate( glm::mat4( 1.0f ), glm::radians( theta ), glm::vec3( 0.0f, 1.0f, 0.0f ) ) *
        rotate( glm::mat4( 1.0f ), glm::radians( -90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) *
        scale( glm::mat4( 1.0f ), glm::vec3( scale_factor, scale_factor, scale_factor ) );
    ObjectShader->uniformMat4fv( lighting::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv( lighting::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        lighting::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniform3fv( lighting::ActivatedLightPosition, Lights->getPosition( ActivatedLightIndex ) );
    ObjectShader->uniform1i( lighting::UseLight, Lights->isLightOn() ? 1 : 0 );
    ObjectShader->uniform1i( lighting::LightIndex, ActivatedLightIndex );
    if (Lights->isLightOn()) {
        const int offset = lighting::Lights + l::UniformNum * ActivatedLightIndex;
        ObjectShader->uniform1i( offset + l::LightSwitch, Lights->isActivated( ActivatedLightIndex ) ? 1 : 0 );
        ObjectShader->uniform4fv( offset + l::LightPosition, Lights->getPosition( ActivatedLightIndex ) );
        ObjectShader->uniform4fv( offset + l::LightAmbientColor, Lights->getAmbientColors( ActivatedLightIndex ) );
        ObjectShader->uniform4fv( offset + l::LightDiffuseColor, Lights->getDiffuseColors( ActivatedLightIndex ) );
        ObjectShader->uniform4fv( offset + l::LightSpecularColor, Lights->getSpecularColors( ActivatedLightIndex ) );
        ObjectShader->uniform3fv(
            offset + l::SpotlightDirection,
            Lights->getSpotlightDirections( ActivatedLightIndex )
        );
        ObjectShader->uniform1f(
            offset + l::SpotlightCutoffAngle,
            Lights->getSpotlightCutoffAngles( ActivatedLightIndex )
        );
        ObjectShader->uniform1f( offset + l::SpotlightFeather, Lights->getSpotlightFeathers( ActivatedLightIndex ) );
        ObjectShader->uniform1f( offset + l::FallOffRadius, Lights->getFallOffRadii( ActivatedLightIndex ) );
        ObjectShader->uniform4fv( lighting::GlobalAmbient, Lights->getGlobalAmbientColor() );
    }
    ObjectShader->uniform4fv(
        lighting::Material + m::EmissionColor,
        MovingTigerObjects[TigerIndex]->getEmissionColor()
    );
    ObjectShader->uniform4fv(
        lighting::Material + m::AmbientColor,
        MovingTigerObjects[TigerIndex]->getAmbientReflectionColor()
    );
    ObjectShader->uniform4fv(
        lighting::Material + m::DiffuseColor,
        MovingTigerObjects[TigerIndex]->getDiffuseReflectionColor()
    );
    ObjectShader->uniform4fv(
        lighting::Material + m::SpecularColor,
        MovingTigerObjects[TigerIndex]->getSpecularReflectionColor()
    );
    ObjectShader->uniform1f(
        lighting::Material + m::SpecularExponent,
        MovingTigerObjects[TigerIndex]->getSpecularReflectionExponent()
    );
    ObjectShader->uniform1f( lighting::EnvironmentRadius, EnvironmentRadius );
    glBindTextureUnit( 0, MovingTigerObjects[TigerIndex]->getTextureID( 0 ) );
    glBindVertexArray( MovingTigerObjects[TigerIndex]->getVAO() );
    glDrawArrays( MovingTigerObjects[TigerIndex]->getDrawMode(), 0, MovingTigerObjects[TigerIndex]->getVertexNum() );
}

void C13EnvironmentMapping::drawCow(float scale_factor) const
{
    using l = ShaderGL::LIGHT_UNIFORM;
    using m = ShaderGL::MATERIAL_UNIFORM;

    glUseProgram( ObjectShader->getShaderProgram() );
    const glm::mat4 to_world =
        translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, 15.0f, 0.0f ) ) *
        rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) ) *
        scale( glm::mat4( 1.0f ), glm::vec3( scale_factor, scale_factor, scale_factor ) );
    ObjectShader->uniformMat4fv( lighting::WorldMatrix, to_world );
    ObjectShader->uniformMat4fv( lighting::ViewMatrix, MainCamera->getViewMatrix() );
    ObjectShader->uniformMat4fv(
        lighting::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniform3fv( lighting::ActivatedLightPosition, Lights->getPosition( ActivatedLightIndex ) );
    ObjectShader->uniform1i( lighting::UseLight, Lights->isLightOn() ? 1 : 0 );
    ObjectShader->uniform1i( lighting::LightIndex, ActivatedLightIndex );
    if (Lights->isLightOn()) {
        const int offset = lighting::Lights + l::UniformNum * ActivatedLightIndex;
        ObjectShader->uniform1i( offset + l::LightSwitch, Lights->isActivated( ActivatedLightIndex ) ? 1 : 0 );
        ObjectShader->uniform4fv( offset + l::LightPosition, Lights->getPosition( ActivatedLightIndex ) );
        ObjectShader->uniform4fv( offset + l::LightAmbientColor, Lights->getAmbientColors( ActivatedLightIndex ) );
        ObjectShader->uniform4fv( offset + l::LightDiffuseColor, Lights->getDiffuseColors( ActivatedLightIndex ) );
        ObjectShader->uniform4fv( offset + l::LightSpecularColor, Lights->getSpecularColors( ActivatedLightIndex ) );
        ObjectShader->uniform3fv(
            offset + l::SpotlightDirection,
            Lights->getSpotlightDirections( ActivatedLightIndex )
        );
        ObjectShader->uniform1f(
            offset + l::SpotlightCutoffAngle,
            Lights->getSpotlightCutoffAngles( ActivatedLightIndex )
        );
        ObjectShader->uniform1f( offset + l::SpotlightFeather, Lights->getSpotlightFeathers( ActivatedLightIndex ) );
        ObjectShader->uniform1f( offset + l::FallOffRadius, Lights->getFallOffRadii( ActivatedLightIndex ) );
        ObjectShader->uniform4fv( lighting::GlobalAmbient, Lights->getGlobalAmbientColor() );
    }
    ObjectShader->uniform4fv( lighting::Material + m::EmissionColor, CowObject->getEmissionColor() );
    ObjectShader->uniform4fv( lighting::Material + m::AmbientColor, CowObject->getAmbientReflectionColor() );
    ObjectShader->uniform4fv( lighting::Material + m::DiffuseColor, CowObject->getDiffuseReflectionColor() );
    ObjectShader->uniform4fv( lighting::Material + m::SpecularColor, CowObject->getSpecularReflectionColor() );
    ObjectShader->uniform1f( lighting::Material + m::SpecularExponent, CowObject->getSpecularReflectionExponent() );
    ObjectShader->uniform1f( lighting::EnvironmentRadius, EnvironmentRadius );
    glBindTextureUnit( 0, CowObject->getTextureID( 0 ) );
    glBindVertexArray( CowObject->getVAO() );
    glDrawArrays( CowObject->getDrawMode(), 0, CowObject->getVertexNum() );
}

void C13EnvironmentMapping::render() const
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( EnvironmentShader->getShaderProgram() );
    const glm::mat4 to_world =
        rotate( glm::mat4( 1.0f ), glm::radians( -90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) *
        scale( glm::mat4( 1.0f ), glm::vec3( EnvironmentRadius ) );
    EnvironmentShader->uniformMat4fv( environment_mappint::WorldMatrix, to_world );
    EnvironmentShader->uniformMat4fv(
        environment_mappint::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    EnvironmentShader->uniform1f( environment_mappint::EnvironmentRadius, EnvironmentRadius );
    glBindTextureUnit( 0, EnvironmentObject->getTextureID( 0 ) );
    glBindVertexArray( EnvironmentObject->getVAO() );
    glDrawArrays( EnvironmentObject->getDrawMode(), 0, EnvironmentObject->getVertexNum() );

    if (DrawMovingObject) drawMovingTiger( 0.05f );
    else drawCow( 7.0f );
}

void C13EnvironmentMapping::update()
{
    if (DrawMovingObject) {
        TigerIndex++;
        if (TigerIndex == 12) TigerIndex = 0;
        TigerRotationAngle += 3;
        if (TigerRotationAngle == 360) TigerRotationAngle = 0;
    }
}

void C13EnvironmentMapping::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    findLightsFromImage();
    setEnvironmentObject();
    setMovingTigerObjects();
    setCowObject();

    constexpr double update_time = 0.2;
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

        glfwSwapBuffers( Window );
        glfwPollEvents();
    }
    glfwDestroyWindow( Window );
}

int main()
{
    C13EnvironmentMapping renderer{};
    renderer.play();
    return 0;
}