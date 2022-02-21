#ifndef LIB_KINEMATICS_JOINT_H_
#define LIB_KINEMATICS_JOINT_H_

#include "../models/node.h"

class Bone {
public:
    /**
     * @param origin The offset of the joint origin from the parent node.
     * @param length The offset of the joint end from the joint origin.
     * @param rotation_limit_x The maximum rotation around the x axis.
     * @param rotation_limit_y The maximum rotation around the y axis.
     * @param rotation_limit_z The maximum rotation around the z axis.
     * @param rotation_speed The maximum rotation speed around the x, y, and z axis.
     * @param node The node that this joint is attached to.
     */
    Bone(glm::vec3 origin,
         glm::vec3 length,
         glm::vec2 rotation_limit_x,
         glm::vec2 rotation_limit_y,
         glm::vec2 rotation_limit_z,
         float rotation_speed,
         Node *node)
        : origin_(origin), length_(length), rotation_limit_x_(rotation_limit_x), rotation_limit_y_(rotation_limit_y),
          rotation_limit_z_(rotation_limit_z), rotation_speed_(rotation_speed), node_(node) {}

    void Actuate(glm::vec3 delta) {
        delta.x = glm::clamp(delta.x, -rotation_speed_, rotation_speed_);
        delta.y = glm::clamp(delta.y, -rotation_speed_, rotation_speed_);
        delta.z = glm::clamp(delta.z, -rotation_speed_, rotation_speed_);

        auto rotation = node_->GetRotation() + delta;
        rotation.x = glm::clamp(rotation.x, rotation_limit_x_.x, rotation_limit_x_.y);
        rotation.y = glm::clamp(rotation.y, rotation_limit_y_.x, rotation_limit_y_.y);
        rotation.z = glm::clamp(rotation.z, rotation_limit_z_.x, rotation_limit_z_.y);

        node_->SetRotation(rotation);
    }

    [[nodiscard]] glm::vec3 GetEndPosition() const {
        return node_->GetWorldTransform() * glm::vec4(origin_ + length_, 1.0f);
    }

    [[nodiscard]] glm::mat4 GetWorldTransform() const { return node_->GetWorldTransform(); }

    glm::vec3 Jacobian(glm::vec3 end_position, glm::vec3 axis) {
        axis = glm::mat3(node_->GetWorldTransform()) * axis;
        return glm::cross(axis, end_position - node_->GetWorldPosition());
    }

private:
    glm::vec3 origin_, length_;

    glm::vec2 rotation_limit_x_, rotation_limit_y_, rotation_limit_z_;

    float rotation_speed_;

    Node *node_;
};

#endif // LIB_KINEMATICS_JOINT_H_
