#ifndef PROGRAM_H
#define PROGRAM_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstring>
#include <iostream>

#include "camera.h"
#include "shader.h"

struct simple_vertex_t {
    GLfloat x, y, z, r, g, b, a;
};

class Program {
public:
    Program(const char *vertex_shader_path, const char *fragment_shader_path);

    virtual bool Initialize(std::string window_title);

    void Run();

protected:
    Camera *camera_;
    ShaderProgram *shader_ = nullptr;

    int display_width = 1024, display_height = 768;
    bool mouse_hold = false;

    GLFWwindow *window_ = nullptr;

    simple_vertex_t *vertices_ = {};
    GLint vertices_size_ = 0;

    virtual void Draw();

    virtual void InitializeObjects();

private:
    char *vertex_shader_path_;
    char *fragment_shader_path_;

    static void HandleFramebufferSizeChange(GLFWwindow *window, int width, int height);

    void HandleKeyboardInput(double delta_frame);

    void HandleMouseInput();
};

#endif //PROGRAM_H
