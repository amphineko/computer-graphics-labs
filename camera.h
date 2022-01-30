#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

typedef int camera_movement_t;

const camera_movement_t CAMERA_MOVEMENT_LEFT = 1;
const camera_movement_t CAMERA_MOVEMENT_RIGHT = 2;
const camera_movement_t CAMERA_MOVEMENT_FORWARD = 3;
const camera_movement_t CAMERA_MOVEMENT_BACKWARD = 4;
const camera_movement_t CAMERA_MOVEMENT_UP = 5;
const camera_movement_t CAMERA_MOVEMENT_DOWN = 6;

class Camera {
public:
    Camera(float x, float y, float z, float pitch, float yaw);

    float Zoom = 45.0f;
    float TranslateSpeed = 1.0f;
    float RotateSpeed = 5.0f;
    glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    [[nodiscard]] glm::vec3 GetPosition() const;
    [[nodiscard]] glm::mat4 GetViewMatrix() const;

    void ApplyTranslate(camera_movement_t direction, double delta_time);

    void ApplyRotate(float offset_x, float offset_y);

    void SetPosition(float x, float y, float z);

    void SetRotation(float pitch, float yaw);

private:
    float pitch, yaw;

    glm::vec3 position{};
    glm::vec3 front{};
    glm::vec3 right{};
    glm::vec3 up{};

    void UpdateDirectionVectors();
};

#endif
