#ifndef IK_HPP
#define IK_HPP

#include <optional>
#include <vector>

constexpr float L1                  = 39.5f;   // cm
constexpr float L2                  = 44.5f;   // cm
constexpr float BASE_SEPARATION     = 22.15f;  // cm
constexpr float MIN_EE_LINE_DIST    = 0.f;     // cm, raise to enforce a min clearance

struct IKResult {
    std::vector<float> jointA;
    std::vector<float> jointB;
    float theta1;
    float phi1;
    float theta2;
    float phi2;
    float eeDist;  // signed distance from end-effector to line between passive joints
};

std::optional<std::vector<float>> solveChain(
    const std::vector<float>& base,
    const std::vector<float>& p,
    float l1, float l2, bool elbowUp);

std::optional<IKResult> fiveBarIK(
    const std::vector<float>& baseA,
    const std::vector<float>& baseB,
    const std::vector<float>& target,
    float l1, float l2);

#endif