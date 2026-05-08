#include "animation.hpp"
#include <cmath>

static constexpr float PI     = 3.14159265f;
static constexpr float CX     =   0.f;
static constexpr float CY     =  50.f;
static constexpr float RADIUS =  15.f;
static constexpr float HALF   =  15.f;

std::vector<float> animStep(AnimState& anim, float dt,
                             const std::vector<float>& origin, float pxPerCm)
{
    anim.t += anim.speed * dt;
    anim.t -= floorf(anim.t);

    float t = anim.t;
    float wx = CX, wy = CY;

    switch (anim.shape) {

    case AnimShape::Heart: {
        float angle = 2.f * PI * t;
        // parametric heart curve, scaled and centered
        wx = CX + RADIUS * 0.6f * sinf(angle) * sinf(angle) * sinf(angle);
        wy = CY + RADIUS * 0.5f * (
            1.3f * cosf(angle)
            - 0.5f * cosf(2.f * angle)
            - 0.2f * cosf(3.f * angle)
            - 0.1f * cosf(4.f * angle));
        break;
    }

    case AnimShape::LineH: {
        float s   = fmodf(t * 2.f, 1.f);
        int   seg = (int)(t * 2.f) % 2;
        switch (seg) {
        case 0: wx = -64.f + 128.f * s; wy = 23.f; break;  // left -> right
        case 1: wx =  64.f - 128.f * s; wy = 23.f; break;  // right -> left
        }
        break;
    }

    case AnimShape::LineV: {
        float s   = fmodf(t * 2.f, 1.f);
        int   seg = (int)(t * 2.f) % 2;
        switch (seg) {
        case 0: wx = 0.f; wy = 23.f + 60.f * s; break;  // bottom -> top
        case 1: wx = 0.f; wy = 83.f - 60.f * s; break;  // top -> bottom
        }
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

    default:
        break;
    }

    float px = origin[0] + wx * pxPerCm;
    float py = origin[1] - wy * pxPerCm;
    return { px, py };
}

std::string shapeName(AnimShape s) {
    switch (s) {
    case AnimShape::Heart: return "Heart";
    case AnimShape::LineH: return "Line H";
    case AnimShape::LineV: return "Line V";
    case AnimShape::Square:   return "Square";
    case AnimShape::Triangle: return "Triangle";
    case AnimShape::Circle:   return "Circle";
    case AnimShape::Figure8:  return "Figure-8";
    default:                  return "None";
    }
}