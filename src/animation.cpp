#include "animation.hpp"
#include <cmath>

static constexpr float PI     = 3.14159265f;
static constexpr float CX     =   0.f;
static constexpr float CY     =  50.f;
static constexpr float RADIUS =  15.f;
static constexpr float HALF   =  15.f;

void startAnim(AnimState& anim, AnimShape newShape,
               const std::vector<float>& currentTarget)
{
    anim.fromPos = currentTarget;
    anim.blendT  = 0.f;
    anim.shape   = newShape;
}

static std::vector<float> calcAnimTarget(AnimState& anim,
                                          const std::vector<float>& origin,
                                          float pxPerCm)
{
    float t  = anim.t;
    float wx = CX, wy = CY;

    switch (anim.shape) {

    case AnimShape::LineH: {
        float s   = fmodf(t * 2.f, 1.f);
        int   seg = (int)(t * 2.f) % 2;
        wx = (seg == 0) ? -45.f + 90.f * s : 45.f - 90.f * s;
        wy = 45.f;
        break;
    }

    case AnimShape::LineV: {
        float s   = fmodf(t * 2.f, 1.f);
        int   seg = (int)(t * 2.f) % 2;
        wx = 0.f;
        wy = (seg == 0) ? 23.f + 60.f * s : 83.f - 60.f * s;
        break;
    }

    case AnimShape::Square: {
        float s    = fmodf(t * 4.f, 1.f);
        int   side = (int)(t * 4.f) % 4;
        switch (side) {
        case 0: wx = CX - HALF + 2*HALF*s; wy = CY - HALF; break;
        case 1: wx = CX + HALF;             wy = CY - HALF + 2*HALF*s; break;
        case 2: wx = CX + HALF - 2*HALF*s; wy = CY + HALF; break;
        case 3: wx = CX - HALF;             wy = CY + HALF - 2*HALF*s; break;
        }
        break;
    }

    case AnimShape::Triangle: {
        float pts[3][2] = {
            { CX,        CY + HALF * 1.2f },
            { CX + HALF, CY - HALF * 0.6f },
            { CX - HALF, CY - HALF * 0.6f }
        };
        float s   = fmodf(t * 3.f, 1.f);
        int   seg = (int)(t * 3.f) % 3;
        int   nxt = (seg + 1) % 3;
        wx = pts[seg][0] + s * (pts[nxt][0] - pts[seg][0]);
        wy = pts[seg][1] + s * (pts[nxt][1] - pts[seg][1]);
        break;
    }

    case AnimShape::Circle:
        wx = CX + RADIUS * cosf(2.f * PI * t);
        wy = CY + RADIUS * sinf(2.f * PI * t);
        break;

    case AnimShape::Figure8:
        wx = CX + RADIUS * sinf(2.f * PI * t);
        wy = CY + RADIUS * sinf(4.f * PI * t) * 0.5f;
        break;

    case AnimShape::Heart: {
        float angle = 2.f * PI * t;
        wx = CX + RADIUS * 0.6f * sinf(angle) * sinf(angle) * sinf(angle);
        wy = CY + RADIUS * 0.5f * (
            1.3f * cosf(angle)
            - 0.5f * cosf(2.f * angle)
            - 0.2f * cosf(3.f * angle)
            - 0.1f * cosf(4.f * angle));
        break;
    }

    default: break;
    }

    return { origin[0] + wx * pxPerCm,
             origin[1] - wy * pxPerCm };
}

// smooth step (ease in/out) - also exposed via animation.hpp for gui.cpp
float smoothStep(float x) {
    x = fmaxf(0.f, fminf(1.f, x));
    return x * x * (3.f - 2.f * x);
}

std::vector<float> animStep(AnimState& anim, float dt,
                             const std::vector<float>& origin, float pxPerCm)
{
    anim.t += anim.speed * dt;
    anim.t -= floorf(anim.t);

    // advance blend
    anim.blendT = fminf(1.f, anim.blendT + anim.blendSpeed * dt);
    float blend = smoothStep(anim.blendT);

    auto animPos = calcAnimTarget(anim, origin, pxPerCm);

    // lerp from previous position to animation
    return {
        anim.fromPos[0] + blend * (animPos[0] - anim.fromPos[0]),
        anim.fromPos[1] + blend * (animPos[1] - anim.fromPos[1])
    };
}

std::string shapeName(AnimShape s) {
    switch (s) {
    case AnimShape::LineH:    return "Line H";
    case AnimShape::LineV:    return "Line V";
    case AnimShape::Square:   return "Square";
    case AnimShape::Triangle: return "Triangle";
    case AnimShape::Circle:   return "Circle";
    case AnimShape::Figure8:  return "Figure-8";
    case AnimShape::Heart:    return "Heart";
    default:                  return "None";
    }
}