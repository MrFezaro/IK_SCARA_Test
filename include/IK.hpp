#ifndef IK_HPP
#define IK_HPP

#include <optional>
#include <vector>

struct IKResult {
    std::vector<float> jointA;
    std::vector<float> jointB;
    float theta1;
    float theta2;
};

std::optional<std::vector<float>> solveChain(
    const std::vector<float>& base,
    const std::vector<float>& p,
    float l1, float l2, bool elbowUp);

std::optional<IKResult> fiveBarIK(
    const std::vector<float>& baseA,
    const std::vector<float>& baseB,
    const std::vector<float>& target,
    float linkLen);

#endif