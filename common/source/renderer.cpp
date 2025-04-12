#include "renderer.h"

RendererGL::RendererGL()
    : Window( nullptr ),
      FrameWidth( 1920 ),
      FrameHeight( 1080 ),
      ClickedPoint( -1, -1 )
{
    Renderer = this;

    initialize();
    printOpenGLInformation();
}

void RendererGL::printOpenGLInformation()
{
    std::cout << "====================== [ Renderer Information ] ================================================\n";
    std::cout << " - GLFW version supported: " << glfwGetVersionString() << "\n";
    std::cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << "\n";
    std::cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << "\n";
    std::cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << "\n";
    std::cout << "================================================================================================\n";
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
}

void RendererGL::cursor(GLFWwindow* window, double xpos, double ypos)
{
    if (MainCamera->getMovingState()) {
        const auto x = static_cast<int>(std::round( xpos ));
        const auto y = static_cast<int>(std::round( ypos ));
        const int dx = x - ClickedPoint.x;
        const int dy = y - ClickedPoint.y;
        if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
            MainCamera->moveForward( dy );
        }
        else {
            MainCamera->pitch( dy );
            MainCamera->yaw( dx );
        }
        ClickedPoint = { x, y };
    }
}

void RendererGL::mouse(GLFWwindow* window, int button, int action, int mods)
{
    std::ignore = mods;
    if (action != GLFW_PRESS) MainCamera->setMovingState( false );
    else if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double x, y;
        glfwGetCursorPos( window, &x, &y );
        ClickedPoint.x = static_cast<int>(std::round( x ));
        ClickedPoint.y = static_cast<int>(std::round( y ));
        MainCamera->setMovingState( true );
    }
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset) const
{
    std::ignore = window;
    std::ignore = xoffset;
    if (yoffset >= 0.0) MainCamera->zoomIn();
    else MainCamera->zoomOut();
}

void RendererGL::registerCallbacks() const
{
    glfwSetErrorCallback( error );
    glfwSetWindowCloseCallback( Window, cleanup );
    glfwSetKeyCallback( Window, keyboardWrapper );
    glfwSetCursorPosCallback( Window, cursorWrapper );
    glfwSetMouseButtonCallback( Window, mouseWrapper );
    glfwSetScrollCallback( Window, mousewheelWrapper );
    glfwSetFramebufferSizeCallback( Window, reshapeWrapper );
}

void RendererGL::captureTexture() const
{
    const int size = FrameWidth * FrameHeight * 3;
    auto* buffer = new uint8_t[size];
    glReadPixels( 0, 0, FrameWidth, FrameHeight, GL_BGR, GL_UNSIGNED_BYTE, buffer );
    const std::string file_name = std::string( CMAKE_SOURCE_DIR ) + "/frame.png";
    FIBITMAP* image = FreeImage_ConvertFromRawBits(
        buffer, FrameWidth, FrameHeight, FrameWidth * 3, 24,
        FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, false
    );
    FreeImage_Save( FIF_PNG, image, file_name.c_str() );
    FreeImage_Unload( image );
    delete [] buffer;
}

void RendererGL::writeTexture(GLuint texture_id, int width, int height)
{
    const int size = width * height * 4;
    auto* buffer = new uint8_t[size];
    glGetTextureImage( texture_id, 0, GL_BGRA, GL_UNSIGNED_BYTE, size, buffer );
    const std::string file_name = std::string( CMAKE_SOURCE_DIR ) + "/frame.png";
    FIBITMAP* image = FreeImage_ConvertFromRawBits(
        buffer, width, height, width * 4, 32,
        FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, false
    );
    FreeImage_Save( FIF_PNG, image, file_name.c_str() );
    FreeImage_Unload( image );
    delete [] buffer;
}

float RendererGL::linearizeDepthValue(float depth)
{
    // WdC to Eye Coordinates, which requires near/far planes. These are just temporary values for visualization.
    constexpr float n = 1.0f;
    constexpr float f = 1000.0f;
    const float z_ndc = 2.0f * depth - 1.0f;
    const float z = 2.0f * n * f / (f + n - z_ndc * (f - n));
    return std::clamp( (z - n) / (f - n), 0.0f, 1.0f );
}

void RendererGL::writeDepthTexture(GLuint texture_id, int width, int height)
{
    const int size = width * height;
    auto* depth = new GLfloat[size];
    auto* buffer = new uint8_t[size];
    glGetTextureImage(
        texture_id, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
        static_cast<int>(size * sizeof( GLfloat )), depth
    );
    for (int i = 0; i < size; ++i) {
        buffer[i] = static_cast<uint8_t>(linearizeDepthValue( depth[i] ) * 255.0f);
    }
    const std::string file_name = std::string( CMAKE_SOURCE_DIR ) + "/depth.png";
    FIBITMAP* image = FreeImage_ConvertFromRawBits(
        buffer, width, height, width, 8,
        FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, false
    );
    FreeImage_Save( FIF_PNG, image, file_name.c_str() );
    FreeImage_Unload( image );
    delete [] depth;
    delete [] buffer;
}