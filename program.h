#ifndef PROGRAM_H_
#define PROGRAM_H_

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstring>
#include <iostream>
#include <vector>

#include "cameras/camera_fp.h"
#include "shader.h"

class Program {
public:
    Program();

    virtual bool Initialize(const std::string *window_title);

    void Run();

protected:
    BaseCamera *camera_;

    std::vector<ShaderProgram *> shaders_{};

    int display_width = 1920, display_height = 1080;
    bool mouse_hold = false;

    GLFWwindow *window_ = nullptr;

    virtual void Draw();

private:
    std::string window_title_;

    static void HandleFramebufferSizeChange(GLFWwindow *window, int width, int height);

    void HandleKeyboardInput(double delta_frame);

    void HandleMouseInput();
};

#endif
