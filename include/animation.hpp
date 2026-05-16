#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <vector>
#include <string>

enum class AnimShape { None, LineH, LineV, Square, Triangle, Circle, Figure8, Heart };

struct AnimState {
    AnimShape shape       = AnimShape::None;
    float     t           = 0.f;
    float     speed       = 0.5f;

    float     blendT      = 1.f;   // 0 = fully at fromPos, 1 = fully on animation
    float     blendSpeed  = 0.5f;   // how fast to blend in seconds
    std::vector<float> fromPos = {0.f, 0.f};  // position we're blending from
};

std::vector<float> animStep(AnimState& anim, float dt,
                             const std::vector<float>& origin, float pxPerCm);

void startAnim(AnimState& anim, AnimShape newShape,
               const std::vector<float>& currentTarget);

std::string shapeName(AnimShape s);

float smoothStep(float x);

#endif