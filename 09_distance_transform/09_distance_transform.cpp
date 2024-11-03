#include "09_distance_transform.h"

C09DistanceTransform::C09DistanceTransform()
    : DistanceType( DISTANCE_TYPE::EUCLIDEAN ),
      InsideColumnScannerBuffer( 0 ),
      OutsideColumnScannerBuffer( 0 ),
      InsideDistanceFieldBuffer( 0 ),
      OutsideDistanceFieldBuffer( 0 ),
      ObjectShader( std::make_unique<MVPShaderGL>() ),
      FieldShader( std::make_unique<MVPShaderGL>() ),
      TransformShader( std::make_unique<DistanceTransformShaderGL>() ),
      ImageObject( std::make_unique<ObjectGL>() ),
      DistanceObject( std::make_unique<ObjectGL>() ),
      Canvas( std::make_unique<CanvasGL>() )
{
    MainCamera = std::make_unique<CameraGL>( FrameWidth, FrameHeight );

    const std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/09_distance_transform/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/read_texture.vert" ).c_str(),
        std::string( shader_directory_path + "/read_texture.frag" ).c_str()
    );
    FieldShader->setShader(
        std::string( shader_directory_path + "/field.vert" ).c_str(),
        std::string( shader_directory_path + "/field.frag" ).c_str()
    );
    TransformShader->setComputeShader(
        std::string( shader_directory_path + "/distance_transform.comp" ).c_str()
    );
    glClearColor( 0.97f, 0.93f, 0.89f, 1.0f );
    glDisable( GL_DEPTH_TEST );
}

void C09DistanceTransform::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    switch (key) {
        case GLFW_KEY_1:
            DistanceType = DISTANCE_TYPE::EUCLIDEAN;
            break;
        case GLFW_KEY_2:
            DistanceType = DISTANCE_TYPE::MANHATTAN;
            break;
        case GLFW_KEY_3:
            DistanceType = DISTANCE_TYPE::CHESSBOARD;
            break;
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            cleanup( window );
            break;
        default:
            return;
    }
}

void C09DistanceTransform::setObjects()
{
    const int data_size = FrameWidth * FrameHeight;
    InsideColumnScannerBuffer = DistanceObject->addCustomBufferObject<int>( data_size );
    OutsideColumnScannerBuffer = DistanceObject->addCustomBufferObject<int>( data_size );
    InsideDistanceFieldBuffer = DistanceObject->addCustomBufferObject<int>( data_size );
    OutsideDistanceFieldBuffer = DistanceObject->addCustomBufferObject<int>( data_size );

    Canvas->setCanvas( FrameWidth, FrameHeight, GL_RGBA8 );

    ImageObject->setSquareObject(
        GL_TRIANGLES,
        std::string( CMAKE_SOURCE_DIR ) + "/09_distance_transform/horse.png"
    );
}

void C09DistanceTransform::drawImage() const
{
    Canvas->clearColor();

    glUseProgram( ObjectShader->getShaderProgram() );
    const GLuint texture_id = ImageObject->getTextureID( 0 );
    const glm::ivec2 texture_size = ImageObject->getTextureSize( texture_id );
    const glm::mat4 to_world =
        translate(
            glm::mat4( 1.0f ),
            glm::vec3( (FrameWidth - texture_size.x) / 2, (FrameHeight - texture_size.y) / 2, 0.0f )
        ) *
        scale( glm::mat4( 1.0f ), glm::vec3( texture_size, 1.0f ) );
    ObjectShader->uniformMat4fv(
        MVPShaderGL::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );

    glBindFramebuffer( GL_FRAMEBUFFER, Canvas->getCanvasID() );
    glBindTextureUnit( 0, texture_id );
    glBindVertexArray( ImageObject->getVAO() );
    glDrawArrays( ImageObject->getDrawMode(), 0, ImageObject->getVertexNum() );
}

void C09DistanceTransform::drawDistanceField()
{
    glUseProgram( TransformShader->getShaderProgram() );
    TransformShader->uniform1i( DistanceTransformShaderGL::Phase, 1 );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, InsideColumnScannerBuffer );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, OutsideColumnScannerBuffer );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, InsideDistanceFieldBuffer );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3, OutsideDistanceFieldBuffer );
    glBindImageTexture( 0, Canvas->getColor0TextureID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8 );
    glDispatchCompute( getGroupSize( FrameHeight ), 1, 1 );
    glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );

    TransformShader->uniform1i( DistanceTransformShaderGL::Phase, 2 );
    TransformShader->uniform1i( DistanceTransformShaderGL::DistanceType, static_cast<int>(DistanceType) );
    glDispatchCompute( getGroupSize( FrameWidth ), 1, 1 );
    glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );

    glUseProgram( FieldShader->getShaderProgram() );
    const glm::mat4 to_world = scale( glm::mat4( 1.0f ), glm::vec3( FrameWidth, FrameHeight, 1.0f ) );
    FieldShader->uniformMat4fv(
        MVPShaderGL::ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindTextureUnit( 0, Canvas->getColor0TextureID() );
    glBindVertexArray( ImageObject->getVAO() );
    glDrawArrays( ImageObject->getDrawMode(), 0, ImageObject->getVertexNum() );
}

void C09DistanceTransform::render()
{
    glClear( GL_COLOR_BUFFER_BIT );

    drawImage();
    drawDistanceField();
}

void C09DistanceTransform::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    setObjects();

    while (!glfwWindowShouldClose( Window )) {
        render();

        glfwSwapBuffers( Window );
        glfwPollEvents();
    }
    glfwDestroyWindow( Window );
}

int main()
{
    C09DistanceTransform renderer{};
    renderer.play();
    return 0;
}