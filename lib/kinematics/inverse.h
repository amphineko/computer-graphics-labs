#ifndef LIB_KINEMATICS_CHAIN_H_
#define LIB_KINEMATICS_CHAIN_H_

#include <vector>

#include "../models/node.h"
#include "bone.h"

class BoneChain {
public:
    explicit BoneChain(std::vector<Bone *> &joints) { joints_ = joints; }

    glm::vec3 Solve(glm::vec3 target, float velocity) {
        glm::vec3 end_diff = target - joints_.back()->GetEndPosition();
        if (glm::length(end_diff) > (velocity * 10000.0f)) {
            for (auto joint : joints_) {
                auto delta_x = velocity * glm::dot(joint->Jacobian(target, glm::vec3(1.0, 0.0, 0.0)), end_diff);
                auto delta_y = velocity * glm::dot(joint->Jacobian(target, glm::vec3(0.0, 1.0, 0.0)), end_diff);
                auto delta_z = velocity * glm::dot(joint->Jacobian(target, glm::vec3(0.0, 0.0, 1.0)), end_diff);
                joint->Actuate(glm::vec3(delta_x, delta_y, delta_z));
            }
        }
    }

private:
    std::vector<Bone *> joints_;
};

#endif // LIB_KINEMATICS_CHAIN_H_
