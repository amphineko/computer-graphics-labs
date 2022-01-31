#include "program.h"
#include "model.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

Program::Program(const char *vertex_shader_path, const char *fragment_shader_path) {
    camera_ = new Camera(0, 0, 3, 0, 270);

    vertex_shader_path_ = new char[strlen(vertex_shader_path) + 1];
    strcpy(vertex_shader_path_, vertex_shader_path);
    fragment_shader_path_ = new char[strlen(fragment_shader_path) + 1];
    strcpy(fragment_shader_path_, fragment_shader_path);
}

bool Program::Initialize(std::string window_title) {
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

    window_ = glfwCreateWindow(display_width, display_height, window_title.c_str(), nullptr, nullptr);
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

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    shader_ = new ShaderProgram(vertex_shader_path_, fragment_shader_path_);
    if (!shader_->IsReady()) {
        std::cerr << "FATAL: Failed to initialize shader" << std::endl;
        glfwTerminate();
        return false;
    }
    shader_->Use();
    std::cout << "INFO: Shader program is ready" << std::endl;

    InitializeObjects();
    std::cout << "INFO: Object buffers are ready" << std::endl;

    return true;
}

void Program::Run() {
    auto last_frame_time = glfwGetTime();

    while (!glfwWindowShouldClose(window_)) {
        auto current_time = glfwGetTime();
        auto frame_time = current_time - last_frame_time;
        last_frame_time = current_time;

        HandleKeyboardInput(frame_time);
        HandleMouseInput();

        auto aspect = (float) display_width / (float) display_height;
        auto zoom = glm::radians((float) camera_->Zoom);
        auto projection = glm::perspective(zoom, aspect, 0.1f, 100.0f);
        shader_->SetMat4("projection", projection);

        auto view = camera_->GetViewMatrix();
        shader_->SetMat4("view", view);

        Draw();
        glfwSwapBuffers(window_);

        glfwPollEvents();
    }
}

void Program::Draw() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void Program::InitializeObjects() {
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices_size_, vertices_, GL_STATIC_DRAW);

    auto pos_loc = glGetAttribLocation(shader_->GetProgram(), "vPos");
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(simple_vertex_t), BUFFER_OFFSET(0));
    glEnableVertexAttribArray(pos_loc);

    auto color_loc = glGetAttribLocation(shader_->GetProgram(), "vColor");
    glVertexAttribPointer(color_loc,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(simple_vertex_t),
                          BUFFER_OFFSET(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(color_loc);
}

void Program::HandleFramebufferSizeChange(GLFWwindow *window, int width, int height) {
    auto that = (Program *) glfwGetWindowUserPointer(window);
    std::cout << "INFO: Resized window to " << width << "x" << height << std::endl;
    that->display_width = width;
    that->display_height = height;
    glViewport(0, 0, width, height);
}

void Program::HandleKeyboardInput(double delta_frame) {
    // escape to exit
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, true);
    }

    // w s to translate forward and backward
    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
        camera_->ApplyTranslate(CAMERA_MOVEMENT_FORWARD, delta_frame);
    }
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
        camera_->ApplyTranslate(CAMERA_MOVEMENT_BACKWARD, delta_frame);
    }

    // a d to translate left and right
    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
        camera_->ApplyTranslate(CAMERA_MOVEMENT_LEFT, delta_frame);
    }
    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
        camera_->ApplyTranslate(CAMERA_MOVEMENT_RIGHT, delta_frame);
    }

    // r f to translate up and down
    if (glfwGetKey(window_, GLFW_KEY_R) == GLFW_PRESS) {
        camera_->ApplyTranslate(CAMERA_MOVEMENT_UP, delta_frame);
    }
    if (glfwGetKey(window_, GLFW_KEY_F) == GLFW_PRESS) {
        camera_->ApplyTranslate(CAMERA_MOVEMENT_DOWN, delta_frame);
    }
}

void Program::HandleMouseInput() {
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
            camera_->ApplyRotate(float(mouse_x), float(-mouse_y));
        }
    } else {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        mouse_hold = false;
    }
}
