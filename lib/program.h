#ifndef PROGRAM_H_
#define PROGRAM_H_

#include "cameras/camera_fp.h"
#include "models/scene.h"
#include "shaders/shader.h"
#include <GLFW/glfw3.h>

#define MAX_N_LIGHTS 16

class Program {
public:
    Program() { camera_ = new FirstPersonCamera(0, 0, 3, 0, 270); }

    virtual bool Initialize(const std::string *window_title) {
        window_title_ = *window_title;

        if (!glfwInit()) {
            std::cerr << "FATAL: Failed to initialize GLFW" << std::endl;
            return false;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        glfwWindowHint(GLFW_SAMPLES, 4);

        window_ = glfwCreateWindow(display_width, display_height, window_title->c_str(), nullptr, nullptr);
        if (window_ == nullptr) {
            std::cerr << "FATAL: Failed to open GLFW window." << std::endl;
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window_);

        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, &HandleFramebufferSizeChange);

        std::cout << "INFO: OpenGL session created, version " << glGetString(GL_VERSION) << std::endl;

        if (glewInit() != GLEW_OK) {
            std::cerr << "FATAL: Failed to initialize GLEW" << std::endl;
            glfwTerminate();
            return false;
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_MULTISAMPLE);

        return true;
    }

    void Run() {
        auto last_frame_time = glfwGetTime(), last_fps_time = last_frame_time;
        auto fps = 0;

        while (!glfwWindowShouldClose(window_)) {
            auto current_time = glfwGetTime();
            auto frame_time = current_time - last_frame_time;
            last_frame_time = current_time;

            ++fps;
            if (current_time - last_fps_time >= 1.0) {
                glfwSetWindowTitle(window_, (window_title_ + " [FPS: " + std::to_string(fps) + "]").c_str());
                fps = 0;
                last_fps_time = current_time;
            }

            HandleKeyboardInput(frame_time);
            HandleMouseInput();

            auto aspect = (float)display_width / (float)display_height;
            auto zoom = glm::radians((float)camera_->zoom);
            auto projection = glm::perspective(zoom, aspect, 0.1f, 500.0f);

            auto view = camera_->GetViewMatrix();

            for (auto &shader : shaders_) {
                shader->SetMat4("projectionMatrix", projection);
                shader->SetMat4("viewMatrix", view);

                shader->SetVec3("cameraPosition", camera_->GetPosition());

                shader->SetInt("nLights", n_lights_);
                shader->SetVec3Array("lightDiffuses", MAX_N_LIGHTS, light_diffuses_);
                shader->SetVec3Array("lightDirections", MAX_N_LIGHTS, light_directions_);
                shader->SetVec3Array("lightPositions", MAX_N_LIGHTS, light_positions_);
            }

            Draw();

            glfwSwapBuffers(window_);
            glfwPollEvents();
        }
    }

protected:
    BaseCamera *camera_;

    std::vector<ShaderProgram *> shaders_{};

    TextureManager texture_manager_;

    int display_width = 1024, display_height = 768;
    bool mouse_hold = false;

    GLFWwindow *window_ = nullptr;

    virtual void Draw() {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void SetLight(glm::vec3 position, glm::vec3 direction) { SetLight(0, position, direction); }

    void SetLight(size_t index, glm::vec3 position, glm::vec3 direction) {
        SetLight(index, position, direction, glm::vec3(1.0f));
    }

    void SetLight(size_t index, glm::vec3 position, glm::vec3 direction, glm::vec3 diffuse) {
        if (index >= MAX_N_LIGHTS) {
            std::cerr << "WARNING: Light index " << index << " is out of range" << std::endl;
            return;
        }

        light_diffuses_[index] = diffuse;
        light_directions_[index] = direction;
        light_positions_[index] = position;
    }

    void SetLightCount(size_t n_lights) { n_lights_ = n_lights; }

private:
    std::string window_title_;

    GLint n_lights_ = 1;
    glm::vec3 light_diffuses_[MAX_N_LIGHTS] = {glm::vec3(1.0f, 1.0f, 1.0f)};
    glm::vec3 light_directions_[MAX_N_LIGHTS] = {glm::vec3(0, 0, 0)};
    glm::vec3 light_positions_[MAX_N_LIGHTS] = {glm::vec3(0, 0, 0)};

    static void HandleFramebufferSizeChange(GLFWwindow *window, int width, int height) {
        auto that = (Program *)glfwGetWindowUserPointer(window);
        std::cout << "INFO: Resized window to " << width << "x" << height << std::endl;
        that->display_width = width;
        that->display_height = height;
        glViewport(0, 0, width, height);
    }

    void HandleKeyboardInput(double delta_frame) {
        // escape to exit
        if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window_, true);
        }

        double forward = 0, right = 0, up = 0;

        // w s to translate forward and backward
        if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
            forward += delta_frame;
        }
        if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
            forward -= delta_frame;
        }

        // a d to translate left and right_
        if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
            right -= delta_frame;
        }
        if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
            right += delta_frame;
        }

        // r f to translate up_ and down
        if (glfwGetKey(window_, GLFW_KEY_R) == GLFW_PRESS) {
            up += delta_frame;
        }
        if (glfwGetKey(window_, GLFW_KEY_F) == GLFW_PRESS) {
            up -= delta_frame;
        }

        if (abs(forward) > 0.01f || abs(right) > 0.01f || abs(up) > 0.01f) {
            camera_->Translate((float)forward, (float)right, (float)up);
        }

        // TODO: reset camera to initial position_
    }

    void HandleMouseInput() {
        double mouse_x, mouse_y;
        double center_x = double(display_width) / 2, center_y = double(display_height) / 2;
        if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (!mouse_hold) {
                glfwSetCursorPos(window_, center_x, center_y);
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                mouse_hold = true;
            } else {
                glfwGetCursorPos(window_, &mouse_x, &mouse_y);
                glfwSetCursorPos(window_, center_x, center_y);

                mouse_x = (mouse_x - center_x) / display_width;
                mouse_y = (mouse_y - center_y) / display_height;
                camera_->Rotate(float(-mouse_y), float(mouse_x));
            }
        } else {
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            mouse_hold = false;
        }
    }
};

#endif
