#include "05_moving_point_on_bezier_curve.h"

C05MovingPointOnBezierCurve::C05MovingPointOnBezierCurve()
    : PositionMode( false ),
      VelocityMode( false ),
      MoveType( MOVE_TYPE::NONE ),
      FrameIndex( 0 ),
      PositionCurveSamplePointNum( 101 ),
      TotalPositionCurvePointNum( 201 ),
      TotalVelocityCurvePointNum( 201 ),
      ObjectShader( std::make_unique<ShaderGL>() ),
      AxisObject( std::make_unique<ObjectGL>() ),
      PositionObject( std::make_unique<ObjectGL>() ),
      VelocityObject( std::make_unique<ObjectGL>() ),
      PositionCurveObject( std::make_unique<ObjectGL>() ),
      VelocityCurveObject( std::make_unique<ObjectGL>() ),
      MovingObject( std::make_unique<ObjectGL>() )
{
    MainCamera = std::make_unique<CameraGL>( FrameWidth, FrameHeight );

    const std::string shader_directory_path =
        std::string( CMAKE_SOURCE_DIR ) + "/05_moving_point_on_bezier_curve/shaders";
    ObjectShader->setShader(
        std::string( shader_directory_path + "/shader.vert" ).c_str(),
        std::string( shader_directory_path + "/shader.frag" ).c_str()
    );
}

void C05MovingPointOnBezierCurve::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    switch (key) {
        case GLFW_KEY_P:
            PositionMode = true;
            VelocityMode = false;
            std::cout << "Select 4 points for the position curve.\n";
            break;
        case GLFW_KEY_V:
            if (static_cast<int>(PositionCurve.size()) != PositionCurveSamplePointNum) {
                std::cout << "Select Position Curve Points First!\n";
                return;
            }
            PositionMode = false;
            VelocityMode = true;
            std::cout << "Select 2 points for the velocity curve.\n";
            break;
        case GLFW_KEY_C:
            clearCurve();
            break;
        case GLFW_KEY_1:
            if (static_cast<int>(PositionCurve.size()) == PositionCurveSamplePointNum &&
                static_cast<int>(UniformVelocityCurve.size()) == TotalPositionCurvePointNum) {
                std::cout << "The point is moving at an uniform speed.\n";
                MoveType = MOVE_TYPE::UNIFORM;
                FrameIndex = 0;
            }
            break;
        case GLFW_KEY_2:
            if (static_cast<int>(VelocityCurve.size()) == TotalVelocityCurvePointNum &&
                static_cast<int>(VariableVelocityCurve.size()) == TotalVelocityCurvePointNum) {
                std::cout << "The point is moving at an variable speed.\n";
                MoveType = MOVE_TYPE::VARIABLE;
                FrameIndex = 0;
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

void C05MovingPointOnBezierCurve::cursor(GLFWwindow* window, double xpos, double ypos)
{
    if (1280.0f <= static_cast<float>(xpos)) {
        glfwSetCursor( window, glfwCreateStandardCursor( GLFW_CROSSHAIR_CURSOR ) );
    }
    else glfwSetCursor( window, nullptr );
}

glm::vec3 C05MovingPointOnBezierCurve::getPointOnPositionBezierCurve(float t) const
{
    const float t2 = t * t;
    const float t3 = t2 * t;
    const float one_minus_t = 1.0f - t;
    const float b0 = one_minus_t * one_minus_t * one_minus_t / 6.0f;
    const float b1 = (3.0f * t3 - 6.0f * t2 + 4.0f) / 6.0f;
    const float b2 = (-3.0f * t3 + 3.0f * t2 + 3.0f * t + 1.0f) / 6.0f;
    const float b3 = t3 / 6.0f;
    return
        b0 * PositionControlPoints[0] + b1 * PositionControlPoints[1] +
        b2 * PositionControlPoints[2] + b3 * PositionControlPoints[3];
}

glm::vec3 C05MovingPointOnBezierCurve::getPointOnVelocityBezierCurve(float t) const
{
    const float one_minus_t = 1.0f - t;
    const glm::vec3 b0 = one_minus_t * VelocityControlPoints[0] + t * VelocityControlPoints[1];
    const glm::vec3 b1 = one_minus_t * VelocityControlPoints[1] + t * VelocityControlPoints[2];
    const glm::vec3 b2 = one_minus_t * VelocityControlPoints[2] + t * VelocityControlPoints[3];
    const glm::vec3 b3 = one_minus_t * b0 + t * b1;
    const glm::vec3 b4 = one_minus_t * b1 + t * b2;
    return one_minus_t * b3 + t * b4;
}

float C05MovingPointOnBezierCurve::getDeltaLength(float t) const
{
    const float t2 = t * t;
    const float one_minus_t = 1.0f - t;
    const float b0 = -1.0f * one_minus_t * one_minus_t / 2.0f;
    const float b1 = (3.0f * t2 - 4.0f * t) / 2.0f;
    const float b2 = (-3.0f * t2 + 2.0f * t + 1.0f) / 2.0f;
    const float b3 = t2 / 2.0f;
    const glm::vec3 derivative =
        b0 * PositionControlPoints[0] + b1 * PositionControlPoints[1] +
        b2 * PositionControlPoints[2] + b3 * PositionControlPoints[3];
    return length( derivative );
}

float C05MovingPointOnBezierCurve::getCurveLengthFromZeroTo(float t) const
{
    constexpr int n = 6;
    const float h = t / static_cast<float>(n);
    float sum1 = 0.0f, sum2 = 0.0f;
    for (int i = 1; i < n; i += 2) {
        sum1 += getDeltaLength( static_cast<float>(i) * h );
    }
    for (int i = 2; i < n; i += 2) {
        sum2 += getDeltaLength( static_cast<float>(i) * h );
    }
    return h / 3.0f * (getDeltaLength( 0.0f ) + getDeltaLength( t ) + 4.0f * sum1 + 2.0f * sum2);
}

float C05MovingPointOnBezierCurve::getInverseCurveLength(float length) const
{
    int iteration = 12;
    float a = 0.0f, b = 1.0f;
    float mid, f_mid;
    do {
        mid = (a + b) / 2;
        f_mid = getCurveLengthFromZeroTo( mid ) - length;
        if ((getCurveLengthFromZeroTo( a ) - length) * f_mid < 0.0f) b = mid;
        else a = mid;
    } while (std::abs( f_mid ) > 1e-4f && iteration-- > 0);
    return mid;
}

float C05MovingPointOnBezierCurve::getAlphaSatisfyingXValue(float x) const
{
    if (x <= 0.0f) return 0.0f;

    int iteration = 12;
    float a = 0.0f, b = 1.0f;
    float mid, f_mid;
    do {
        mid = (a + b) * 0.5f;
        f_mid = getPointOnVelocityBezierCurve( mid ).x - x;
        if ((getPointOnVelocityBezierCurve( a ).x - x) * f_mid <= 0.0f) b = mid;
        else a = mid;
    } while (std::abs( f_mid ) > 1e-4f && iteration-- > 0);
    return mid;
}

void C05MovingPointOnBezierCurve::createPositionCurve()
{
    float t = 0.0f;
    const float dt = 1.0f / static_cast<float>(PositionCurveSamplePointNum - 1);
    for (int i = 0; i < PositionCurveSamplePointNum; ++i) {
        PositionCurve.emplace_back( getPointOnPositionBezierCurve( t ) );
        t += dt;
    }
    PositionCurveObject->updateDataBuffer( PositionCurve );

    float l = 0.0f;
    const float dl = getCurveLengthFromZeroTo( 1.0f ) / static_cast<float>(TotalPositionCurvePointNum - 1);
    for (int i = 0; i < TotalPositionCurvePointNum; ++i) {
        UniformVelocityCurve.emplace_back( getPointOnPositionBezierCurve( getInverseCurveLength( l ) ) );
        l += dl;
    }
}

void C05MovingPointOnBezierCurve::createVelocityCurve()
{
    float t = 0.0f;
    float dt = 1.0f / static_cast<float>(TotalVelocityCurvePointNum - 1);
    for (int i = 0; i < TotalVelocityCurvePointNum; ++i) {
        VelocityCurve.emplace_back( getPointOnVelocityBezierCurve( t ) );
        t += dt;
    }
    VelocityCurveObject->updateDataBuffer( VelocityCurve );

    t = VelocityControlPoints[0].x;
    dt = (VelocityControlPoints[3].x - VelocityControlPoints[0].x) / static_cast<float>(TotalVelocityCurvePointNum - 1);
    for (int i = 0; i < TotalVelocityCurvePointNum; ++i) {
        const float velocity_alpha = getAlphaSatisfyingXValue( t );
        const float length = getPointOnVelocityBezierCurve( velocity_alpha ).y;
        VariableVelocityCurve.emplace_back( getPointOnPositionBezierCurve( getInverseCurveLength( length ) ) );
        t += dt;
    }
}

void C05MovingPointOnBezierCurve::clearCurve()
{
    MoveType = MOVE_TYPE::NONE;
    PositionMode = false;
    PositionControlPoints.clear();
    PositionCurve.clear();
    UniformVelocityCurve.clear();
    VelocityMode = false;
    VelocityControlPoints.clear();
    VelocityCurve.clear();
    VariableVelocityCurve.clear();
}

void C05MovingPointOnBezierCurve::mouse(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double x_pos, y_pos;
        glfwGetCursorPos( window, &x_pos, &y_pos );
        const auto x = static_cast<float>(x_pos);
        const auto y = static_cast<float>(y_pos);

        if (PositionMode && PositionControlPoints.size() <= 3 && 1280.0f <= x && y <= 540.0f) {
            PositionControlPoints.emplace_back( (x - 1280.0f) * 3.0f, (540.0f - y) * 2.0f, 0.0f );
            if (PositionControlPoints.size() == 4) {
                PositionMode = false;
                createPositionCurve();
            }
        }
        else if (VelocityMode && VelocityControlPoints.size() <= 3 && 1280.0f <= x && 540.0f < y) {
            if (VelocityControlPoints.empty()) {
                VelocityControlPoints.emplace_back( 150.0f, 100.0f, 0.0f );
                VelocityControlPoints.emplace_back( (x - 1280.0f) * 3.0f, (1080.0f - y) * 2.0f, 0.0f );
            }
            else {
                VelocityControlPoints.emplace_back( (x - 1280.0f) * 3.0f, (1080.0f - y) * 2.0f, 0.0f );
                VelocityControlPoints.emplace_back( 1750.0f, 900.0f, 0.0f );

                VelocityMode = false;
                createVelocityCurve();
            }
        }
    }
}

void C05MovingPointOnBezierCurve::setAxisObject() const
{
    const std::vector<glm::vec3> axis_vertices = {
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f }
    };
    AxisObject->setObject( GL_LINES, axis_vertices );
    AxisObject->setDiffuseReflectionColor( { 0.93f, 0.92f, 0.91f, 1.0f } );
}

void C05MovingPointOnBezierCurve::setCurveObjects() const
{
    PositionObject->setObject( GL_LINE_STRIP, 4 );
    PositionObject->setDiffuseReflectionColor( { 0.9f, 0.8f, 0.1f, 1.0f } );

    VelocityObject->setObject( GL_LINE_STRIP, 4 );
    VelocityObject->setDiffuseReflectionColor( { 0.9f, 0.8f, 0.1f, 1.0f } );

    PositionCurveObject->setObject( GL_LINE_STRIP, PositionCurveSamplePointNum );
    PositionCurveObject->setDiffuseReflectionColor( { 0.9f, 0.1f, 0.1f, 1.0f } );

    VelocityCurveObject->setObject( GL_LINE_STRIP, TotalVelocityCurvePointNum );
    VelocityCurveObject->setDiffuseReflectionColor( { 0.9f, 0.1f, 0.1f, 1.0f } );

    MovingObject->setObject( GL_POINTS, 1 );
    MovingObject->setDiffuseReflectionColor( { 1.0f, 0.0f, 0.0f, 1.0f } );
}

void C05MovingPointOnBezierCurve::drawAxisObject() const
{
    glLineWidth( 5.0f );
    glUseProgram( ObjectShader->getShaderProgram() );
    const glm::mat4 scale_matrix = scale( glm::mat4( 1.0f ), glm::vec3( 1600.0f, 800.0f, 1.0f ) );
    const glm::mat4 translation = translate( glm::mat4( 1.0f ), glm::vec3( 150.0f, 100.0f, 0.0f ) );
    glm::mat4 to_world = translation * scale_matrix;
    ObjectShader->uniformMat4fv(
        ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniform4fv( Color, AxisObject->getDiffuseReflectionColor() );
    glBindVertexArray( AxisObject->getVAO() );
    glDrawArrays( AxisObject->getDrawMode(), 0, AxisObject->getVertexNum() );

    const glm::mat4 rotation = rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    to_world = to_world * rotation;
    ObjectShader->uniformMat4fv(
        ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix() * to_world
    );
    ObjectShader->uniform4fv( Color, AxisObject->getDiffuseReflectionColor() );
    glDrawArrays( AxisObject->getDrawMode(), 0, AxisObject->getVertexNum() );
    glLineWidth( 1.0f );
}

void C05MovingPointOnBezierCurve::drawControlPoints(const ObjectGL* control_points) const
{
    glLineWidth( 3.0f );
    glPointSize( 10.0f );
    glUseProgram( ObjectShader->getShaderProgram() );
    ObjectShader->uniformMat4fv(
        ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix()
    );
    ObjectShader->uniform4fv( Color, control_points->getDiffuseReflectionColor() );
    glBindVertexArray( control_points->getVAO() );
    glDrawArrays( control_points->getDrawMode(), 0, control_points->getVertexNum() );
    glDrawArrays( GL_POINTS, 0, control_points->getVertexNum() );
    glLineWidth( 1.0f );
    glPointSize( 1.0f );
}

void C05MovingPointOnBezierCurve::drawCurve(const ObjectGL* curve) const
{
    glLineWidth( 3.0f );
    glUseProgram( ObjectShader->getShaderProgram() );
    ObjectShader->uniformMat4fv(
        ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix()
    );
    ObjectShader->uniform4fv( Color, curve->getDiffuseReflectionColor() );
    glBindVertexArray( curve->getVAO() );
    glDrawArrays( curve->getDrawMode(), 0, curve->getVertexNum() );
    glLineWidth( 1.0f );
}

void C05MovingPointOnBezierCurve::drawMovingPoint()
{
    switch (MoveType) {
        case MOVE_TYPE::UNIFORM:
            if (FrameIndex >= TotalPositionCurvePointNum) FrameIndex = TotalPositionCurvePointNum - 1;
            MovingObject->updateDataBuffer( { UniformVelocityCurve[FrameIndex] } );
            break;
        case MOVE_TYPE::VARIABLE:
            if (FrameIndex >= TotalVelocityCurvePointNum) FrameIndex = TotalVelocityCurvePointNum - 1;
            MovingObject->updateDataBuffer( { VariableVelocityCurve[FrameIndex] } );
            break;
        case MOVE_TYPE::NONE:
        default:
            return;
    }

    glPointSize( 20.0f );
    glUseProgram( ObjectShader->getShaderProgram() );
    ObjectShader->uniformMat4fv(
        ModelViewProjectionMatrix,
        MainCamera->getProjectionMatrix() * MainCamera->getViewMatrix()
    );
    ObjectShader->uniform4fv( Color, MovingObject->getDiffuseReflectionColor() );
    glBindVertexArray( MovingObject->getVAO() );
    glDrawArrays( MovingObject->getDrawMode(), 0, MovingObject->getVertexNum() );
    glPointSize( 1.0f );

    FrameIndex++;
}

void C05MovingPointOnBezierCurve::drawMainCurve()
{
    glViewport( 0, 0, 1280, 1080 );
    glClearColor( 0.72f, 0.72f, 0.77f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    drawAxisObject();

    if (!PositionMode && !PositionCurve.empty()) {
        drawCurve( PositionCurveObject.get() );
        drawMovingPoint();
    }
}

void C05MovingPointOnBezierCurve::drawPositionCurve() const
{
    glEnable( GL_SCISSOR_TEST );
    glViewport( 1280, 540, 640, 540 );
    glScissor( 1280, 540, 640, 540 );
    glClearColor( 0.63f, 0.53f, 0.49f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    drawAxisObject();

    if (PositionControlPoints.size() <= 4) {
        PositionObject->updateDataBuffer( PositionControlPoints );
    }
    drawControlPoints( PositionObject.get() );

    if (!PositionMode && !PositionCurve.empty()) {
        drawCurve( PositionCurveObject.get() );
    }
    glDisable( GL_SCISSOR_TEST );
}

void C05MovingPointOnBezierCurve::drawVelocityCurve() const
{
    glEnable( GL_SCISSOR_TEST );
    glViewport( 1280, 0, 640, 540 );
    glScissor( 1280, 0, 640, 540 );
    glClearColor( 0.55f, 0.43f, 0.38f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    drawAxisObject();

    if (VelocityControlPoints.size() <= 4) {
        VelocityObject->updateDataBuffer( VelocityControlPoints );
    }
    drawControlPoints( VelocityObject.get() );

    if (!VelocityMode && !VelocityCurve.empty()) {
        drawCurve( VelocityCurveObject.get() );
    }
    glDisable( GL_SCISSOR_TEST );
}

void C05MovingPointOnBezierCurve::play()
{
    if (glfwWindowShouldClose( Window )) initialize();

    setAxisObject();
    setCurveObjects();

    while (!glfwWindowShouldClose( Window )) {
        drawMainCurve();
        drawPositionCurve();
        drawVelocityCurve();

        glfwPollEvents();
        glfwSwapBuffers( Window );
    }
    glfwDestroyWindow( Window );
}

int main()
{
    C05MovingPointOnBezierCurve renderer{};
    renderer.play();
    return 0;
}