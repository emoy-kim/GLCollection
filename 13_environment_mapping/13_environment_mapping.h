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

    C13EnvironmentMapping(const C13EnvironmentMapping&) = delete;
    C13EnvironmentMapping(const C13EnvironmentMapping&&) = delete;
    C13EnvironmentMapping& operator=(const C13EnvironmentMapping&) = delete;
    C13EnvironmentMapping& operator=(const C13EnvironmentMapping&&) = delete;

    void play();

private:
    struct Rect
    {
        glm::ivec2 TopLeft;
        glm::ivec2 Size;

        Rect() = default;
        Rect(int x, int y, int w, int h) : TopLeft( x, y ), Size( w, h ) {}
    };

    bool DrawMovingObject;
    int ActivatedLightIndex;
    int TigerIndex;
    int TigerRotationAngle;
    int EnvironmentWidth;
    int EnvironmentHeight;
    float EnvironmentRadius;
    float* AdjustedIntensities;
    uint8_t* ImageBuffer;
    uint8_t* LatitudeLongitude;
    std::unique_ptr<ShaderGL> ObjectShader;
    std::unique_ptr<ShaderGL> EnvironmentShader;
    std::unique_ptr<ObjectGL> EnvironmentObject;
    std::unique_ptr<ObjectGL> CowObject;
    std::vector<std::unique_ptr<ObjectGL>> MovingTigerObjects;
    std::unique_ptr<LightGL> Lights;

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