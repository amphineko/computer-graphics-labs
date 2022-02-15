#ifndef CAMERAS_COMMON_H_
#define CAMERAS_COMMON_H_

#include "glm/glm.hpp"

class BaseCamera {
public:
    const glm::vec3 DefaultUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float translate_speed = 50.0f, rotate_speed = 1.0f, zoom = 45.0f;

    [[nodiscard]] virtual glm::vec3 GetPosition() const = 0;
    [[nodiscard]] virtual glm::mat4 GetViewMatrix() const = 0;

    virtual void Translate(float forward, float right, float up) = 0;

    virtual void Rotate(float pitch, float yaw) = 0;

    virtual void SetPosition(float x, float y, float z) = 0;

    virtual void SetRotation(float pitch, float yaw) = 0;
};

#endif
