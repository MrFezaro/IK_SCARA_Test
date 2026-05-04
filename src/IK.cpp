#include "IK.hpp"
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

std::optional<IKResult> fiveBarIK(
    const std::vector<float>& baseA,
    const std::vector<float>& baseB,
    const std::vector<float>& target,
    float linkLen)
{
    auto jA = solveChain(baseA, target, linkLen, linkLen, true);
    auto jB = solveChain(baseB, target, linkLen, linkLen, false);
    if (!jA || !jB) return std::nullopt;

    float t1 = atan2f((*jA)[1] - baseA[1], (*jA)[0] - baseA[0]) * (180.f / 3.14159f);
    float t2 = atan2f((*jB)[1] - baseB[1], (*jB)[0] - baseB[0]) * (180.f / 3.14159f);

    return IKResult{ *jA, *jB, t1, t2 };
}