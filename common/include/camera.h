#pragma once

#include "base.h"

class CameraGL final
{
public:
    CameraGL();
    CameraGL(
        const glm::vec3& cam_position,
        const glm::vec3& view_reference_position,
        const glm::vec3& view_up_vector,
        float fov = 30.0f,
        float near_plane = 0.1f,
        float far_plane = 10000.0f
    );
    CameraGL(
        int width,
        int height,
        float near_plane = -1.0f,
        float far_plane = 1.0f
    );

    [[nodiscard]] int getWidth() const { return Width; }
    [[nodiscard]] int getHeight() const { return Height; }
    [[nodiscard]] float getNearPlane() const { return NearPlane; }
    [[nodiscard]] float getFarPlane() const { return FarPlane; }
    [[nodiscard]] bool getMovingState() const { return IsMoving; }
    [[nodiscard]] glm::vec3 getCameraPosition() const { return CamPos; }
    [[nodiscard]] const glm::mat4& getViewMatrix() const { return ViewMatrix; }
    [[nodiscard]] const glm::mat4& getProjectionMatrix() const { return ProjectionMatrix; }
    void setInitFOV(float fov) { InitFOV = FOV = fov; }
    void setMovingState(bool is_moving) { IsMoving = is_moving; }
    void setZoomSensitivity(float zoom) { ZoomSensitivity = zoom; }
    void setMoveSensitivity(float move) { MoveSensitivity = move; }
    void setRotationSensitivity(float rotation) { RotationSensitivity = rotation; }
    void updateCamera();
    void updateCameraPosition(
        const glm::vec3& cam_position,
        const glm::vec3& view_reference_position,
        const glm::vec3& view_up_vector
    );
    void pitch(int angle);
    void yaw(int angle);
    void rotateAroundWorldY(int angle);
    void moveForward(int delta = 1);
    void moveBackward(int delta = 1);
    void moveLeft(int delta = 1);
    void moveRight(int delta = 1);
    void moveUp(int delta = 1);
    void moveDown(int delta = 1);
    void zoomIn();
    void zoomOut();
    void resetCamera();
    void update2DCamera(int width, int height);
    void update3DCamera(int width, int height);

private:
    bool IsPerspective;
    bool IsMoving;
    int Width;
    int Height;
    float FOV;
    float InitFOV;
    float NearPlane;
    float FarPlane;
    float AspectRatio;
    float ZoomSensitivity;
    float MoveSensitivity;
    float RotationSensitivity;
    glm::vec3 InitCamPos;
    glm::vec3 InitRefPos;
    glm::vec3 InitUpVec;
    glm::vec3 CamPos;
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
};