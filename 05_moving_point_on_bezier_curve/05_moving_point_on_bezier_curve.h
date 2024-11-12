#pragma once

#include "../common/include/renderer.h"
#include "../common/include/shader.h"

class C05MovingPointOnBezierCurve final : public RendererGL
{
public:
    C05MovingPointOnBezierCurve();
    ~C05MovingPointOnBezierCurve() override = default;

    C05MovingPointOnBezierCurve(const C05MovingPointOnBezierCurve&) = delete;
    C05MovingPointOnBezierCurve(const C05MovingPointOnBezierCurve&&) = delete;
    C05MovingPointOnBezierCurve& operator=(const C05MovingPointOnBezierCurve&) = delete;
    C05MovingPointOnBezierCurve& operator=(const C05MovingPointOnBezierCurve&&) = delete;

    void play();

private:
    enum class MOVE_TYPE { NONE = 0, UNIFORM, VARIABLE };

    enum UNIFORM { ModelViewProjectionMatrix = 0, Color };

    bool PositionMode;
    bool VelocityMode;
    MOVE_TYPE MoveType;
    int FrameIndex;
    int PositionCurveSamplePointNum;
    int TotalPositionCurvePointNum;
    int TotalVelocityCurvePointNum;
    std::vector<glm::vec3> PositionControlPoints;
    std::vector<glm::vec3> VelocityControlPoints;
    std::vector<glm::vec3> PositionCurve;
    std::vector<glm::vec3> VelocityCurve;
    std::vector<glm::vec3> UniformVelocityCurve;
    std::vector<glm::vec3> VariableVelocityCurve;
    std::unique_ptr<ShaderGL> ObjectShader;
    std::unique_ptr<ObjectGL> AxisObject;
    std::unique_ptr<ObjectGL> PositionObject;
    std::unique_ptr<ObjectGL> VelocityObject;
    std::unique_ptr<ObjectGL> PositionCurveObject;
    std::unique_ptr<ObjectGL> VelocityCurveObject;
    std::unique_ptr<ObjectGL> MovingObject;

    void mousewheel(GLFWwindow* window, double xoffset, double yoffset) const override {}
    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void cursor(GLFWwindow* window, double xpos, double ypos) override;
    void mouse(GLFWwindow* window, int button, int action, int mods) override;
    [[nodiscard]] glm::vec3 getPointOnPositionBezierCurve(float t) const;
    [[nodiscard]] glm::vec3 getPointOnVelocityBezierCurve(float t) const;
    [[nodiscard]] float getDeltaLength(float t) const;
    [[nodiscard]] float getCurveLengthFromZeroTo(float t) const;
    [[nodiscard]] float getInverseCurveLength(float length) const;
    [[nodiscard]] float getAlphaSatisfyingXValue(float x) const;
    void createPositionCurve();
    void createVelocityCurve();
    void clearCurve();
    void setAxisObject() const;
    void setCurveObjects() const;
    void drawAxisObject() const;
    void drawControlPoints(const ObjectGL* control_points) const;
    void drawCurve(const ObjectGL* curve) const;
    void drawMovingPoint();
    void drawMainCurve();
    void drawPositionCurve() const;
    void drawVelocityCurve() const;
};