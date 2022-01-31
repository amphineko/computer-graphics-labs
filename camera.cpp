#include "camera.h"

Camera::Camera(float x, float y, float z, float pitch, float yaw) {
    this->position = glm::vec3(x, y, z);
    this->pitch = pitch;
    this->yaw = yaw;

    this->UpdateDirectionVectors();
}

void Camera::UpdateDirectionVectors() {
    glm::vec3 new_front;
    new_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    new_front.y = sin(glm::radians(pitch));
    new_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    this->front = glm::normalize(new_front);
    this->right = glm::normalize(glm::cross(this->front, this->WorldUp));
    this->up = glm::normalize(glm::cross(this->right, this->front));
}

void Camera::ApplyTranslate(camera_movement_t direction, double delta_time) {
    auto velocity = (float) (this->TranslateSpeed * delta_time);
    switch (direction) {
        case CAMERA_MOVEMENT_BACKWARD: this->position -= this->front * velocity;
            break;
        case CAMERA_MOVEMENT_FORWARD: this->position += this->front * velocity;
            break;
        case CAMERA_MOVEMENT_LEFT: this->position -= glm::normalize(glm::cross(this->front, this->up)) * velocity;
            break;
        case CAMERA_MOVEMENT_RIGHT: this->position += glm::normalize(glm::cross(this->front, this->up)) * velocity;
            break;
        case CAMERA_MOVEMENT_UP: this->position += this->up * velocity;
            break;
        case CAMERA_MOVEMENT_DOWN: this->position -= this->up * velocity;
            break;
        default:break;
    }
}

void Camera::ApplyRotate(float offset_x, float offset_y) {
    this->yaw += offset_x * this->RotateSpeed;
    this->pitch += offset_y * this->RotateSpeed;

    if (this->pitch > 89.0f) {
        this->pitch = 89.0f;
    }

    if (this->pitch < -89.0f) {
        this->pitch = -89.0f;
    }

    UpdateDirectionVectors();
}

glm::vec3 Camera::GetPosition() const {
    return this->position;
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(this->position, this->position + this->front, this->up);
}

void Camera::SetPosition(float x, float y, float z) {
    this->position = glm::vec3(x, y, z);
}

void Camera::SetRotation(float pitch, float yaw) {
    this->pitch = pitch;
    this->yaw = yaw;

    UpdateDirectionVectors();
}
