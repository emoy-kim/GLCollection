#include "camera.h"

CameraGL::CameraGL()
    : CameraGL(
        glm::vec3( 0.0f, 0.0f, 10.0f ),
        glm::vec3( 0.0f, 0.0f, 0.0f ),
        glm::vec3( 0.0f, 1.0f, 0.0f )
    ) {}

CameraGL::CameraGL(
    const glm::vec3& cam_position,
    const glm::vec3& view_reference_position,
    const glm::vec3& view_up_vector,
    float fov,
    float near_plane,
    float far_plane
)
    : IsPerspective( true ),
      IsMoving( false ),
      Width( 0 ),
      Height( 0 ),
      FOV( fov ),
      InitFOV( fov ),
      NearPlane( near_plane ),
      FarPlane( far_plane ),
      AspectRatio( 0.0f ),
      ZoomSensitivity( 1.0f ),
      MoveSensitivity( 1.0f ),
      RotationSensitivity( 0.001f ),
      InitCamPos( cam_position ),
      InitRefPos( view_reference_position ),
      InitUpVec( view_up_vector ),
      CamPos( cam_position ),
      ViewMatrix( lookAt( InitCamPos, InitRefPos, InitUpVec ) ),
      ProjectionMatrix( glm::mat4( 1.0f ) ) {}

CameraGL::CameraGL(int width, int height, float near_plane, float far_plane)
    : IsPerspective( false ),
      IsMoving( false ),
      Width( width ),
      Height( height ),
      FOV( 0.0f ),
      InitFOV( 0.0f ),
      NearPlane( near_plane ),
      FarPlane( far_plane ),
      AspectRatio( 0.0f ),
      ZoomSensitivity( 1.0f ),
      MoveSensitivity( 0.05f ),
      RotationSensitivity( 0.005f ),
      InitCamPos(),
      InitRefPos(),
      InitUpVec(),
      CamPos(),
      ViewMatrix( glm::mat4( 1.0f ) ),
      ProjectionMatrix(
          glm::ortho(
              0.0f, static_cast<float>(width),
              0.0f, static_cast<float>(height),
              near_plane, far_plane
          )
      ) {}

void CameraGL::updateCamera()
{
    const glm::mat4 inverse_view = inverse( ViewMatrix );
    CamPos.x = inverse_view[3][0];
    CamPos.y = inverse_view[3][1];
    CamPos.z = inverse_view[3][2];
}

void CameraGL::updateCameraPosition(
    const glm::vec3& cam_position,
    const glm::vec3& view_reference_position,
    const glm::vec3& view_up_vector
)
{
    InitCamPos = cam_position;
    InitRefPos = view_reference_position;
    InitUpVec = view_up_vector;
    ViewMatrix = lookAt( InitCamPos, InitRefPos, InitUpVec );
    updateCamera();
}

void CameraGL::pitch(int delta)
{
    ViewMatrix = rotate(
        glm::mat4( 1.0f ),
        static_cast<float>(delta) * RotationSensitivity,
        glm::vec3( 1.0f, 0.0f, 0.0f )
    ) * ViewMatrix;
    updateCamera();
}

void CameraGL::yaw(int delta)
{
    ViewMatrix = rotate(
        glm::mat4( 1.0f ),
        static_cast<float>(delta) * RotationSensitivity,
        glm::vec3( 0.0f, 1.0f, 0.0f )
    ) * ViewMatrix;
    updateCamera();
}

void CameraGL::roll(int delta)
{
    ViewMatrix = rotate(
        glm::mat4( 1.0f ),
        static_cast<float>(delta) * RotationSensitivity,
        glm::vec3( 0.0f, 0.0f, 1.0f )
    ) * ViewMatrix;
    updateCamera();
}

void CameraGL::moveForward(int delta)
{
    ViewMatrix = translate(
        glm::mat4( 1.0f ),
        glm::vec3( 0.0f, 0.0f, static_cast<float>(delta) * MoveSensitivity )
    ) * ViewMatrix;
    updateCamera();
}

void CameraGL::moveHorizontally(int delta)
{
    ViewMatrix = translate(
        glm::mat4( 1.0f ),
        glm::vec3( static_cast<float>(delta) * MoveSensitivity, 0.0f, 0.0f )
    ) * ViewMatrix;
    updateCamera();
}

void CameraGL::moveVertically(int delta)
{
    ViewMatrix = translate(
        glm::mat4( 1.0f ),
        glm::vec3( 0.0f, static_cast<float>(delta) * MoveSensitivity, 0.0f )
    ) * ViewMatrix;
    updateCamera();
}

void CameraGL::zoomIn()
{
    if (FOV > 0.0f) {
        FOV -= ZoomSensitivity;
        ProjectionMatrix = glm::perspective( glm::radians( FOV ), AspectRatio, NearPlane, FarPlane );
    }
}

void CameraGL::zoomOut()
{
    if (FOV < 90.0f) {
        FOV += ZoomSensitivity;
        ProjectionMatrix = glm::perspective( glm::radians( FOV ), AspectRatio, NearPlane, FarPlane );
    }
}

void CameraGL::resetCamera()
{
    CamPos = InitCamPos;
    ViewMatrix = lookAt( InitCamPos, InitRefPos, InitUpVec );
    ProjectionMatrix = IsPerspective ?
                           glm::perspective( glm::radians( InitFOV ), AspectRatio, NearPlane, FarPlane ) :
                           glm::ortho(
                               0.0f, static_cast<float>(Width),
                               0.0f, static_cast<float>(Height),
                               NearPlane, FarPlane
                           );
}

void CameraGL::update2DCamera(int width, int height)
{
    Width = width;
    Height = height;
    IsPerspective = false;
    glm::ortho(
        0.0f, static_cast<float>(Width),
        0.0f, static_cast<float>(Height),
        NearPlane, FarPlane
    );
}

void CameraGL::update3DCamera(int width, int height)
{
    Width = width;
    Height = height;
    IsPerspective = true;
    AspectRatio = static_cast<float>(width) / static_cast<float>(height);
    ProjectionMatrix = glm::perspective( glm::radians( FOV ), AspectRatio, NearPlane, FarPlane );
}