#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <vector>
#include <string>

enum class AnimShape { None, LineH, LineV, Square, Triangle, Circle, Figure8, Heart };

struct AnimState {
    AnimShape shape  = AnimShape::None;
    float     t      = 0.f;   // 0..1 progress through the shape
    float     speed  = 0.5f;  // cycles per second
};

// returns next target position in pixel space
// origin: pixel coords of world (0,0)
// pxPerCm: scale
std::vector<float> animStep(AnimState& anim, float dt,
                             const std::vector<float>& origin, float pxPerCm);

std::string shapeName(AnimShape s);

#endif