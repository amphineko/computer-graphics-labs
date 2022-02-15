#ifndef CAMERAS_TP_H_
#define CAMERAS_TP_H_

#include "common.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class ThirdPersonCamera : public BaseCamera {
public:
    float distance = 15.0f;

    ThirdPersonCamera(float x, float y, float z, float pitch, float yaw) {
        distance = glm::distance(glm::vec3(x, y, z), glm::vec3(0.0f));

        rotate_speed = BaseCamera::rotate_speed * 50.0f;

        position_ = glm::vec3(x, y, z);
        pitch_ = pitch;
        yaw_ = yaw;

        UpdateDirectionVectors();
    }

    glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    [[nodiscard]] glm::vec3 GetPosition() const final { return this->position_; }

    [[nodiscard]] glm::mat4 GetViewMatrix() const final {
        return glm::lookAt(this->position_, this->position_ + this->real_front_, this->up_);
    }

    void Translate(float forward_offset, float right_offset, float up_offset) final {
        auto velocity = glm::vec3(0.0f);
        velocity += this->real_front_ * forward_offset;
        velocity += this->right_ * right_offset;
        velocity += this->up_ * up_offset;
        this->position_ += velocity * this->translate_speed;
        UpdateDirectionVectors();
    };

    void Rotate(float pitch_offset, float yaw_offset) final {
        auto origin = position_ + real_front_ * distance;

        pitch_ += pitch_offset * rotate_speed;
        yaw_ += yaw_offset * rotate_speed;

        if (this->pitch_ > 89.0f) {
            this->pitch_ = 89.0f;
        }
        if (this->pitch_ < -89.0f) {
            this->pitch_ = -89.0f;
        }
        if (this->yaw_ > 360.0f) {
            this->yaw_ -= 360.0f;
        }
        if (this->yaw_ < 0.0f) {
            this->yaw_ += 360.0f;
        }

        glm::vec3 rev_direction;
        rev_direction.x = -cos(glm::radians(pitch_)) * cos(glm::radians(yaw_));
        rev_direction.y = -sin(glm::radians(pitch_));
        rev_direction.z = -cos(glm::radians(pitch_)) * sin(glm::radians(yaw_));

        position_ = origin + rev_direction * distance;

        UpdateDirectionVectors();
    }

    void SetPosition(float x, float y, float z) final { this->position_ = glm::vec3(x, y, z); }

    void SetRotation(float pitch_offset, float yaw_offset) final {
        this->pitch_ = pitch_offset;
        this->yaw_ = yaw_offset;
        UpdateDirectionVectors();
    }

private:
    float pitch_, yaw_;

    glm::vec3 position_{};
    glm::vec3 real_front_{};
    glm::vec3 right_{};
    glm::vec3 up_{};

    void UpdateDirectionVectors() {
        glm::vec3 new_direction;
        new_direction.x = cos(glm::radians(pitch_)) * cos(glm::radians(yaw_));
        new_direction.y = sin(glm::radians(pitch_));
        new_direction.z = cos(glm::radians(pitch_)) * sin(glm::radians(yaw_));

        real_front_ = glm::normalize(new_direction);
        right_ = glm::normalize(glm::cross(real_front_, WorldUp));
        up_ = glm::normalize(glm::cross(right_, real_front_));
    }
};

#endif
