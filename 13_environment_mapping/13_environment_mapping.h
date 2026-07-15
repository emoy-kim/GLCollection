#pragma once

#include "../common/include/renderer.h"
#include "../common/include/shader.h"

namespace environment_mappint
{
    enum UNIFORM { WorldMatrix = 0, ModelViewProjectionMatrix, EnvironmentRadius };
}

namespace lighting
{
    enum UNIFORM
    {
        WorldMatrix = 0,
        ViewMatrix,
        ModelViewProjectionMatrix,
        ActivatedLightPosition,
        Lights,
        Material = 292,
        UseLight = 297,
        LightIndex,
        EnvironmentRadius,
        GlobalAmbient
    };
}

class C13EnvironmentMapping final : public RendererGL
{
public:
    C13EnvironmentMapping();
    ~C13EnvironmentMapping() override;

    C13EnvironmentMapping(C13EnvironmentMapping&&) = delete;
    C13EnvironmentMapping(const C13EnvironmentMapping&) = delete;
    C13EnvironmentMapping& operator=(C13EnvironmentMapping&&) = delete;
    C13EnvironmentMapping& operator=(const C13EnvironmentMapping&) = delete;

    void play();

private:
    struct Rect
    {
        glm::ivec2 TopLeft;
        glm::ivec2 Size;

        Rect() = default;
        Rect(int x, int y, int w, int h) : TopLeft( x, y ), Size( w, h ) {}
    };

    bool DrawMovingObject = false;
    int ActivatedLightIndex = 0;
    int TigerIndex = 0;
    int TigerRotationAngle = 180;
    int EnvironmentWidth = 0;
    int EnvironmentHeight = 0;
    float EnvironmentRadius = 50.0f;
    float* AdjustedIntensities = nullptr;
    uint8_t* ImageBuffer = nullptr;
    uint8_t* LatitudeLongitude = nullptr;
    std::unique_ptr<ShaderGL> ObjectShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ShaderGL> EnvironmentShader = std::make_unique<ShaderGL>();
    std::unique_ptr<ObjectGL> EnvironmentObject = std::make_unique<ObjectGL>();
    std::unique_ptr<ObjectGL> CowObject = std::make_unique<ObjectGL>();
    std::vector<std::unique_ptr<ObjectGL>> MovingTigerObjects;
    std::unique_ptr<LightGL> Lights = std::make_unique<LightGL>();

    // https://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2
    static constexpr uint getNextHighestPowerOf2(uint v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }

    void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    [[nodiscard]] static glm::vec4 getPixel(const uint8_t* row_ptr, int x);
    [[nodiscard]] glm::vec4 getBilinearInterpolatedColor(const glm::vec2& point) const;
    void convertFisheye(const std::string& file_path);
    void calculateDeltaXDividingIntensityInHalf(
        int& dx,
        int start,
        int end,
        int block_width,
        float half_intensity
    ) const;
    void calculateDeltaYDividingIntensityInHalf(
        int& dy,
        int start,
        int end,
        int block_height,
        float half_intensity
    ) const;
    void medianCut(std::map<float, glm::ivec2>& light_infos, const Rect& block, int iteration) const;
    [[nodiscard]] std::vector<glm::ivec2> estimateLightPoints();
    void findLightsFromImage();
    void setEnvironmentObject() const;
    void setMovingTigerObjects();
    void setCowObject() const;
    void drawMovingTiger(float scale_factor) const;
    void drawCow(float scale_factor) const;
    void render() const;
    void update();
};