#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>

#include "camera.h"
#include "shader.h"

using namespace std;

struct vertex_t {
    GLfloat x, y, z, r, g, b, a;
};

vertex_t vertices[] = {
        {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f,  -1.0f, 1.0f,  1.0f, 0.0f, 0.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f},

        {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f,  -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f,  -1.0f, 1.0f,  1.0f, 0.0f, 0.0f, 1.0f},

        {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f,  1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        {1.0f,  -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f},

        {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f,  1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f,  -1.0f, 1.0f, 1.0f, 0.0f, 1.0f},

        {1.0f,  -1.0f, 1.0f,  1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f,  -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f,  1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
};

int display_width = 1024, display_height = 768;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void process_keyboard(GLFWwindow *window, Camera *camera, double delta_frame);

void process_mouse(GLFWwindow *window, Camera *camera, double &last_x, double &last_y, bool &is_holding);

bool init_objects(GLuint shader_program) {
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    auto pos_loc = glGetAttribLocation(shader_program, "vPos");
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), BUFFER_OFFSET(0));
    glEnableVertexAttribArray(pos_loc);

    auto color_loc = glGetAttribLocation(shader_program, "vColor");
    glVertexAttribPointer(color_loc, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), BUFFER_OFFSET(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(color_loc);

    return true;
}

bool init(ShaderProgram *&shader_program) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    shader_program = new ShaderProgram("./cs7gv5_lab1.vert", "./cs7gv5_lab1.frag");
    if (shader_program->IsReady()) {
        shader_program->Use();
    } else {
        return false;
    }

    return init_objects(shader_program->GetProgram());
}

void draw(GLFWwindow *window) {
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (GLint i = 0; i < sizeof(vertices) / sizeof(vertex_t); ++i) {
        glDrawArrays(GL_TRIANGLES, i * 3, 3);
    }

    glfwSwapBuffers(window);
}

int main(int argc, char **argv) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window;
    window = glfwCreateWindow(display_width, display_height, "Lab 1: Hello World", nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        std::cout << "INFO: Resized window to " << width << "x" << height << std::endl;
        display_width = width;
        display_height = height;
        glViewport(0, 0, width, height);
    });

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwTerminate();
        return -1;
    }

    ShaderProgram *shader_program = nullptr;
    init(shader_program);
    shader_program->Use();

    auto *camera = new Camera(0, 0, 3, 0, 270);

    auto last_frame_time = glfwGetTime();
    auto last_counter_time = glfwGetTime();
    unsigned long frame_count = 0;

    double mouse_last_x, mouse_last_y;
    bool mouse_hold = false;

    while (!glfwWindowShouldClose(window)) {
        auto current_time = glfwGetTime();
        auto frame_time = current_time - last_frame_time;
        last_frame_time = current_time;

        process_keyboard(window, camera, frame_time);
        process_mouse(window, camera, mouse_last_x, mouse_last_y, mouse_hold);

        auto aspect = (float) display_width / (float) display_height;
        auto zoom = glm::radians((float) camera->Zoom);
        auto projection = glm::perspective(zoom, aspect, 0.1f, 100.0f);
        shader_program->SetMat4("projection", projection);

        auto view = camera->GetViewMatrix();
        shader_program->SetMat4("view", view);

        draw(window);

        ++frame_count;
        if (current_time - last_counter_time >= 5.0) {
            auto position = camera->GetPosition();
            std::cout << "DEBUG: fps=" << float(frame_count) / (current_time - last_counter_time) << ",\t";
            std::cout << "pos=" << position.x << "," << position.y << "," << position.z << std::endl;

            last_counter_time = current_time;
            frame_count = 0;
        }

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void process_keyboard(GLFWwindow *window, Camera *camera, double delta_frame) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera->ApplyTranslate(CAMERA_MOVEMENT_FORWARD, delta_frame);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera->ApplyTranslate(CAMERA_MOVEMENT_LEFT, delta_frame);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera->ApplyTranslate(CAMERA_MOVEMENT_BACKWARD, delta_frame);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera->ApplyTranslate(CAMERA_MOVEMENT_RIGHT, delta_frame);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        camera->ApplyTranslate(CAMERA_MOVEMENT_UP, delta_frame);
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        camera->ApplyTranslate(CAMERA_MOVEMENT_DOWN, delta_frame);
    }
}

void process_mouse(GLFWwindow *window, Camera *camera, double &last_x, double &last_y, bool &is_holding) {
    double mouse_x, mouse_y;
    double center_x = double(display_width) / 2, center_y = double(display_height) / 2;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (!is_holding) {
            glfwSetCursorPos(window, center_x, center_y);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            is_holding = true;
        } else {
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            glfwSetCursorPos(window, center_x, center_y);

            mouse_x = (mouse_x - center_x) / display_width;
            mouse_y = (mouse_y - center_y) / display_height;
            camera->ApplyRotate(float(mouse_x), float(-mouse_y));
        }
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        is_holding = false;
    }
}
