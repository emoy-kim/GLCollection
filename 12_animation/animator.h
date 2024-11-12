#pragma once

#include "base.h"

class Animator2D
{
public:
    enum class FILL_TYPE { FILL = 0, WIRED };

    struct Animation
    {
        glm::vec3 Color{};
        glm::vec2 Translation{};
        glm::vec2 Scale{};
        float RotationAngle;

        Animation() : RotationAngle( 0.0f ) {}
    };

    // The object in a key frame is defined in the coordinates whose origin is at the TOP-LEFT.
    struct Keyframe
    {
        FILL_TYPE FillType;
        int ObjectWidth;
        int ObjectHeight;
        float Duration;
        glm::ivec2 TopLeft;
        glm::vec2 Anchor;
        Animation Start;
        Animation End;

        Keyframe()
            : FillType( FILL_TYPE::FILL ),
              ObjectWidth( 0 ),
              ObjectHeight( 0 ),
              Duration( 0.0f ),
              TopLeft(),
              Anchor() {}
    };

    Animator2D() : TotalKeyframesNum( 0 ) {}

    [[nodiscard]] int getTotalKeyframesNum() const { return TotalKeyframesNum; }

    void addKeyframe(const Keyframe& key_frame)
    {
        Keyframes.emplace_back( key_frame );
        TotalKeyframesNum = static_cast<int>(Keyframes.size());
    }

    [[nodiscard]] glm::ivec4 getOriginalPosition(int keyframe_index) const
    {
        if (keyframe_index >= static_cast<int>(Keyframes.size())) return {};

        const Keyframe& keyframe = Keyframes[keyframe_index];
        return { keyframe.TopLeft.x, keyframe.TopLeft.y, keyframe.ObjectWidth, keyframe.ObjectHeight };
    }

    [[nodiscard]] glm::vec2 getAnchor(int keyframe_index) const
    {
        if (keyframe_index >= static_cast<int>(Keyframes.size())) return {};

        return Keyframes[keyframe_index].Anchor;
    }

    [[nodiscard]] GLenum getFillType(int keyframe_index) const
    {
        if (keyframe_index >= static_cast<int>(Keyframes.size())) return 0;

        return Keyframes[keyframe_index].FillType == FILL_TYPE::FILL ? GL_FILL : GL_LINE;
    }

    void getAnimationNow(Animation& animation, int keyframe_index, float current_time) const
    {
        if (keyframe_index >= static_cast<int>(Keyframes.size())) return;

        const Keyframe& keyframe = Keyframes[keyframe_index];
        const float t = std::clamp( current_time / keyframe.Duration, 0.0f, 1.0f );
        animation.Color = keyframe.Start.Color * (1.0f - t) + keyframe.End.Color * t;
        animation.Scale.x = keyframe.Start.Scale.x * (1.0f - t) + keyframe.End.Scale.x * t;
        animation.Scale.y = keyframe.Start.Scale.y * (1.0f - t) + keyframe.End.Scale.y * t;
        animation.Translation.x = keyframe.Start.Translation.x * (1.0f - t) + keyframe.End.Translation.x * t;
        animation.Translation.y = keyframe.Start.Translation.y * (1.0f - t) + keyframe.End.Translation.y * t;
        animation.RotationAngle = keyframe.Start.RotationAngle * (1.0f - t) + keyframe.End.RotationAngle * t;
    }

    [[nodiscard]] glm::mat4 getWorldMatrix(const Animation& animation, int screen_height, int keyframe_index) const
    {
        const glm::vec2 anchor = getAnchor( keyframe_index );
        const glm::ivec4 box = getOriginalPosition( keyframe_index );
        const float scale_x = static_cast<float>(box.z) * animation.Scale.x;
        const float scale_y = static_cast<float>(box.w) * animation.Scale.y;
        const glm::mat4 scale_matrix = scale( glm::mat4( 1.0f ), glm::vec3( scale_x, scale_y, 1.0f ) );

        const float to_origin_x = scale_x * anchor.x;
        const float to_origin_y = scale_y * anchor.y;
        const glm::mat4 to_origin =
            translate( glm::mat4( 1.0f ), glm::vec3( -to_origin_x, -to_origin_y, 0.0f ) );
        const glm::mat4 rotation = rotate(
            glm::mat4( 1.0f ),
            glm::radians( animation.RotationAngle ),
            glm::vec3( 0.0f, 0.0f, 1.0f )
        );

        const float translate_x = static_cast<float>(box.x) + animation.Translation.x;
        const float translate_y = static_cast<float>(screen_height - box.y) - scale_y - animation.Translation.y;
        const glm::mat4 translation =
            translate( glm::mat4( 1.0f ), glm::vec3( translate_x, translate_y, 0.0f ) );

        return translation * inverse( to_origin ) * rotation * to_origin * scale_matrix;
    }

private:
    int TotalKeyframesNum;
    std::vector<Keyframe> Keyframes;
};