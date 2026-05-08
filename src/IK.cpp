#include "ik.hpp"
#include <cmath>

std::optional<std::vector<float>> solveChain(
    const std::vector<float>& base,
    const std::vector<float>& p,
    float l1, float l2, bool elbowUp)
{
    float dx = p[0] - base[0], dy = p[1] - base[1];
    float d = sqrtf(dx*dx + dy*dy);
    if (d > l1 + l2 || d < fabsf(l1 - l2) || d < 1e-6f) return std::nullopt;
    float a  = (l1*l1 - l2*l2 + d*d) / (2.f * d);
    float h  = sqrtf(fmaxf(0.f, l1*l1 - a*a));
    float mx = base[0] + a * dx / d;
    float my = base[1] + a * dy / d;
    float ox = h * dy / d, oy = h * dx / d;
    return elbowUp
        ? std::vector<float>{mx + ox, my - oy}
        : std::vector<float>{mx - ox, my + oy};
}

// signed distance from point P to line through A->B
// positive = left of A->B, negative = right
static float signedDistToLine(
    const std::vector<float>& a,
    const std::vector<float>& b,
    const std::vector<float>& p)
{
    float abx = b[0] - a[0], aby = b[1] - a[1];
    float len = sqrtf(abx*abx + aby*aby);
    if (len < 1e-6f) return 0.f;
    return -((abx * (p[1] - a[1])) - (aby * (p[0] - a[0]))) / len;
}

std::optional<IKResult> fiveBarIK(
    const std::vector<float>& baseA,
    const std::vector<float>& baseB,
    const std::vector<float>& target,
    float l1, float l2)
{
    auto jA = solveChain(baseA, target, l1, l2, true);
    auto jB = solveChain(baseB, target, l1, l2, false);
    if (!jA || !jB) return std::nullopt;

    // distance from end-effector to line between the two passive joints
    float dist = signedDistToLine(*jA, *jB, target);
    if (dist <= MIN_EE_LINE_DIST) return std::nullopt;

    float t1 = atan2f(-((*jA)[1] - baseA[1]), (*jA)[0] - baseA[0]) * (180.f / 3.14159f);
    float t2 = atan2f(-((*jB)[1] - baseB[1]), (*jB)[0] - baseB[0]) * (180.f / 3.14159f);
    float p1 = atan2f(-(target[1] - (*jA)[1]), target[0] - (*jA)[0]) * (180.f / 3.14159f);
    float p2 = atan2f(-(target[1] - (*jB)[1]), target[0] - (*jB)[0]) * (180.f / 3.14159f);

    return IKResult{ *jA, *jB, t1, p1, t2, p2, dist };
}