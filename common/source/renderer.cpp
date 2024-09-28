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