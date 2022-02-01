#ifndef CAMERAS_FP_H_
#define CAMERAS_FP_H_

#include "common.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class FirstPersonCamera : public BaseCamera {
public:
    FirstPersonCamera(float x, float y, float z, float pitch, float yaw) {
        rotate_speed = BaseCamera::rotate_speed * 50.0f;

        position = glm::vec3(x, y, z);
        pitch_ = pitch;
        yaw_ = yaw;

        UpdateDirectionVectors();
    }

    [[nodiscard]] glm::vec3 GetPosition() const final { return this->position; }

    [[nodiscard]] glm::mat4 GetViewMatrix() const final {
        return glm::lookAt(this->position, this->position + this->front_, this->up_);
    }

    void Translate(float forward_offset, float right_offset, float up_offset) final {
        auto velocity = glm::vec3(0.0f);
        velocity += this->front_ * forward_offset;
        velocity += this->right_ * right_offset;
        velocity += this->up_ * up_offset;
        this->position += velocity * this->translate_speed;
        UpdateDirectionVectors();
    };

    void Rotate(float pitch_offset, float yaw_offset) final {
        this->pitch_ += pitch_offset * this->rotate_speed;
        this->yaw_ += yaw_offset * this->rotate_speed;

        if (this->pitch_ > 90.0f) {
            this->pitch_ = 90.0f;
        }
        if (this->pitch_ < -90.0f) {
            this->pitch_ = -90.0f;
        }
        if (this->yaw_ > 360.0f) {
            this->yaw_ -= 360.0f;
        }
        if (this->yaw_ < 0.0f) {
            this->yaw_ += 360.0f;
        }

        UpdateDirectionVectors();
    }

    void SetPosition(float x, float y, float z) final { this->position = glm::vec3(x, y, z); }

    void SetRotation(float pitch_offset, float yaw_offset) final {
        this->pitch_ = pitch_offset;
        this->yaw_ = yaw_offset;
        UpdateDirectionVectors();
    }

private:
    float pitch_, yaw_;

    glm::vec3 position{};
    glm::vec3 front_{};
    glm::vec3 right_{};
    glm::vec3 up_{};

    void UpdateDirectionVectors() {
        glm::vec3 new_front;
        new_front.x = cos(glm::radians(pitch_)) * cos(glm::radians(yaw_));
        new_front.y = sin(glm::radians(pitch_));
        new_front.z = cos(glm::radians(pitch_)) * sin(glm::radians(yaw_));

        front_ = glm::normalize(new_front);
        right_ = glm::normalize(glm::cross(front_, DefaultUp));
        up_ = glm::normalize(glm::cross(right_, front_));
    }
};

#endif
