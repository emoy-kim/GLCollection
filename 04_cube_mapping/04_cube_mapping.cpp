#include "04_cube_mapping.h"

C04CubeMapping::C04CubeMapping()
    : IsVideo( false ),
      VideoFrameIndex( 0 ),
      FrameBuffers{},
      ObjectShader( std::make_unique<ShaderGL>() )
{
    MainCamera = std::make_unique<CameraGL>(
        glm::vec3( 0.0f, 0.0f, 0.0f ),
        glm::vec3( 0.0f, 0.0f, 1.0f ),
        glm::vec3( 0.0f, -1.0f, 0.0f ),
        70.0f
    );
    MainCamera->setMoveSensitivity( 0.005f );

    const std::string shader_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/04_cube_mapping/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/shader.vert" ).c_str(),
        std::string( shader_directory_path + "/shader.frag" ).c_str()
    );
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
}

C04CubeMapping::~C04CubeMapping()
{
    for (const auto& buffer : FrameBuffers) delete [] buffer;
}

void C04CubeMapping::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    std::ignore = scancode;
    std::ignore = mods;
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
        case GLFW_KEY_ENTER:
            IsVideo = !IsVideo;
            setCubeObject( 5.0f );
            break;
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            cleanup( window );
            break;
        default:
            return;
    }
}

void C04CubeMapping::cursor(GLFWwindow* window, double xpos, double ypos)
{
    if (MainCamera->getMovingState()) {
        const auto x = static_cast<int>(round( xpos ));
        const auto y = static_cast<int>(round( ypos ));
        const int dx = x - ClickedPoint.x;
        const int dy = y - ClickedPoint.y;
        MainCamera->moveForward( -dy );
        MainCamera->rotateAroundWorldY( -dx );

        if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
            MainCamera->pitch( -dy );
        }

        ClickedPoint.x = x;
        ClickedPoint.y = y;
    }
}

void C04CubeMapping::mouse(GLFWwindow* window, int button, int action, int mods)
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

void C04CubeMapping::mousewheel(GLFWwindow* window, double xoffset, double yoffset) const
{
    std::ignore = window;
    std::ignore = xoffset;
    if (yoffset >= 0.0) MainCamera->zoomIn();
    else MainCamera->zoomOut();
}

void C04CubeMapping::setCubeObject(float length)
{
    const std::vector<glm::vec3> cube_vertices{
        { -length, length, -length },
        { -length, -length, -length },
        { length, -length, -length },
        { length, -length, -length },
        { length, length, -length },
        { -length, length, -length },

        { -length, -length, length },
        { -length, -length, -length },
        { -length, length, -length },
        { -length, length, -length },
        { -length, length, length },
        { -length, -length, length },

        { length, -length, -length },
        { length, -length, length },
        { length, length, length },
        { length, length, length },
        { length, length, -length },
        { length, -length, -length },

        { -length, -length, length },
        { -length, length, length },
        { length, length, length },
        { length, length, length },
        { length, -length, length },
        { -length, -length, length },

        { -length, length, -length },
        { length, length, -length },
        { length, length, length },
        { length, length, length },
        { -length, length, length },
        { -length, length, -length },

        { -length, -length, -length },
        { -length, -length, length },
        { length, -length, -length },
        { length, -length, -length },
        { -length, -length, length },
        { length, -length, length }
    };
    CubeObject = std::make_unique<ObjectGL>();
    CubeObject->setObject( GL_TRIANGLES, cube_vertices );

    const std::string sample_directory_path = std::string( CMAKE_SOURCE_DIR ) + "/04_cube_mapping/samples";
    if (IsVideo) {
        VideoFrameIndex = 0;
        const std::array<std::string, 6> video_path_set{
            std::string( sample_directory_path + "/dynamic/right.avi" ),
            std::string( sample_directory_path + "/dynamic/left.avi" ),
            std::string( sample_directory_path + "/dynamic/bottom.avi" ),
            std::string( sample_directory_path + "/dynamic/top.avi" ),
            std::string( sample_directory_path + "/dynamic/back.avi" ),
            std::string( sample_directory_path + "/dynamic/front.avi" )
        };
        for (int i = 0; i < 6; ++i) {
            delete [] FrameBuffers[i];
            Videos[i] = std::make_unique<VideoReader>();
            Videos[i]->open( video_path_set[i] );
        }
        const int w = Videos[0]->getFrameWidth();
        const int h = Videos[0]->getFrameHeight();
        for (int i = 0; i < 6; ++i) {
            FrameBuffers[i] = new uint8_t[w * h * 4];
            if (!Videos[i]->read( FrameBuffers[i], VideoFrameIndex ))
                throw std::runtime_error( "Could not read a video frame!" );
        }
        CubeObject->addCubeTextures( FrameBuffers, w, h );
        VideoFrameIndex++;
    }
    else {
        const std::array<std::string, 6> texture_path_set{
            std::string( sample_directory_path + "/static/sample1/right.jpg" ),
            std::string( sample_directory_path + "/static/sample1/left.jpg" ),
            std::string( sample_directory_path + "/static/sample1/bottom.jpg" ),
            std::string( sample_directory_path + "/static/sample1/top.jpg" ),
            std::string( sample_directory_path + "/static/sample1/back.jpg" ),
            std::string( sample_directory_path + "/static/sample1/front.jpg" )
        };
        CubeObject->addCubeTextures( texture_path_set );
    }
    CubeObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 1.0f, 1.0f } );
}

void C04CubeMapping::render() const
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    MainCamera->update3DCamera( FrameWidth, FrameHeight );
    glViewport( 0, 0, FrameWidth, FrameHeight );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glUseProgram( ObjectShader->getShaderProgram() );

    glUseProgram( ObjectShader->getShaderProgram() );
    ObjectShader->uniformMat4fv(
        ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix()
    );
    ObjectShader->uniform4fv( Color, CubeObject->getDiffuseReflectionColor() );

    glBindTextureUnit( 0, CubeObject->getTextureID( 0 ) );
    glBindVertexArray( CubeObject->getVAO() );
    glDrawArrays( CubeObject->getDrawMode(), 0, CubeObject->getVertexNum() );
}

void C04CubeMapping::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    setCubeObject( 5.0f );

    while (!glfwWindowShouldClose( Window )) {
        render();

        if (IsVideo) {
            for (int i = 0; i < 6; ++i) {
                Videos[i]->read( FrameBuffers[i], VideoFrameIndex );
            }
            ObjectGL::updateCubeTextures( FrameBuffers, Videos[0]->getFrameWidth(), Videos[0]->getFrameHeight() );
            VideoFrameIndex++;
        }

        glfwSwapBuffers( Window );
        glfwPollEvents();
    }
    glfwDestroyWindow( Window );
}

int main()
{
    C04CubeMapping renderer{};
    renderer.play();
    return 0;
}