#ifndef PROGRAM_H_
#define PROGRAM_H_

#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "cameras/camera_fp.h"
#include "models/scene.h"
#include "shaders/shader.h"
#include "utils.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <numeric>

#define ENV_MAP_SIZE 1024
#define FOV 60.0f
#define MAX_N_LIGHTS 16

static const std::map<GLenum, glm::vec3> cube_map_faces = {
    {GL_TEXTURE_CUBE_MAP_POSITIVE_X, glm::vec3(1.0f, 0.0f, 0.0f)},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, glm::vec3(-1.0f, 0.0f, 0.0f)},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Y, glm::vec3(0.0f, 1.0f, 0.0f)},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, glm::vec3(0.0f, -1.0f, 0.0f)},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Z, glm::vec3(0.0f, 0.0f, 1.0f)},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, glm::vec3(0.0f, 0.0f, -1.0f)},
};

class Program {
public:
    Program() { camera_ = new FirstPersonCamera(0, 0, 3, 0, 270); }

    virtual bool Initialize(const std::string &window_title, bool env_map) {
        window_title_ = window_title;

        std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;

        if (!InitializeWindow()) {
            DestroyWindow();
            return false;
        }

        if (env_map && !InitializeEnvMap()) {
            DestroyEnvMap();
            DestroyWindow();
            return false;
        }

        InitializeImGui();

        return true;
    }

    void Run() {
        glfwSetWindowSize(window_, display_width_, display_height_);

        last_frame_clock_ = glfwGetTime();
        while (!glfwWindowShouldClose(window_)) {
            glfwPollEvents();

            if (!io_->WantCaptureMouse) {
                HandleKeyboardInput(last_frame_time_);
                HandleMouseInput();
            }

            // statistics

            current_frame_clock_ = glfwGetTime();
            last_frame_time_ = current_frame_clock_ - last_frame_clock_;
            last_frame_clock_ = current_frame_clock_;

            frame_times.push_back(last_frame_time_);
            total_frame_time += last_frame_time_;

            if (total_frame_time > 1.0) {
                auto frame_count = double(frame_times.size());
                frame_per_sec = frame_count / total_frame_time;
                frame_time_avg = total_frame_time / frame_count;
                frame_time_var = std::accumulate(frame_times.begin(),
                                                 frame_times.end(),
                                                 0.0,
                                                 [&](double sum, double x) {
                                                     return sum + (x - frame_time_avg) * (x - frame_time_avg);
                                                 }) /
                                 frame_count;

                frame_times.clear();
                total_frame_time = 0;
            }

            Draw();

            {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                DrawImGui();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }

            glfwSwapBuffers(window_);
        }
    }

protected:
    BaseCamera *camera_;
    NodeTexture env_map_ = {.name = 0, .role = NODE_TEXTURE_ROLE_ENV_MAP};
    std::vector<ShaderProgram *> shaders_{};
    TextureManager texture_manager_;

    int display_width_ = 1024, display_height_ = 768;
    bool mouse_hold_ = false;

    GLFWwindow *window_ = nullptr;
    ImGuiIO *io_ = nullptr;

    double last_frame_time_ = 0, current_frame_clock_ = 0;

    virtual void Draw() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, display_width_, display_height_);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glCheckError();

        auto viewMatrix = camera_->GetViewMatrix();
        ConfigureShaders(camera_->GetPosition(), display_width_, display_height_, FOV, viewMatrix);
    }

    virtual void DrawImGui() {
        ImGui::Begin("Stats");
        ImGui::Text("FPS: %.2f", frame_per_sec);
        ImGui::Text("Frame time: %.2f ms", last_frame_time_ * 1000);
        ImGui::Text("Frame time avg: %.2f ms", frame_time_avg * 1000);
        ImGui::Text("Frame time var: %.2f ms", frame_time_var * 1000);
        ImGui::End();
    }

    virtual void DrawEnvMap(glm::vec3 position) {
        glBindFramebuffer(GL_FRAMEBUFFER, env_map_.name);
        glViewport(0, 0, ENV_MAP_SIZE, ENV_MAP_SIZE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, env_map_.name, 0);
        DrawEnvMapFace(position, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, env_map_.name, 0);
        DrawEnvMapFace(position, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, env_map_.name, 0);
        DrawEnvMapFace(position, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, env_map_.name, 0);
        DrawEnvMapFace(position, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, env_map_.name, 0);
        DrawEnvMapFace(position, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, env_map_.name, 0);
        DrawEnvMapFace(position, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

        glCheckError();
    }

    virtual void DrawEnvMapFace(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        auto viewMatrix = glm::lookAt(position, position + direction, up);
        ConfigureShaders(position, ENV_MAP_SIZE, ENV_MAP_SIZE, 90, viewMatrix);
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

    void SetLightCount(size_t n_lights) { n_lights_ = GLint(n_lights); }

private:
    std::string window_title_;

    GLuint env_map_fbo_;
    GLuint env_map_depth_rbo_;

    GLint n_lights_ = 1;
    glm::vec3 light_diffuses_[MAX_N_LIGHTS] = {glm::vec3(1.0f, 1.0f, 1.0f)};
    glm::vec3 light_directions_[MAX_N_LIGHTS] = {glm::vec3(0, 0, 0)};
    glm::vec3 light_positions_[MAX_N_LIGHTS] = {glm::vec3(0, 0, 0)};

    double last_frame_clock_;

    std::vector<double> frame_times;
    double total_frame_time = 0;
    double frame_per_sec = 0, frame_time_avg = 0, frame_time_var = 0;

    /**
     * @param width viewport width
     * @param height viewport height
     */
    void ConfigureShaders(glm::vec3 camera_position, uint width, uint height, float fov, glm::mat4 view_matrix) {
        auto aspect = (float)width / (float)height;
        auto projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 1000.0f);

        for (auto &shader : shaders_) {
            shader->Use();

            shader->SetVec3("cameraPosition", camera_position);
            shader->SetMat4("projectionMatrix", projection);
            shader->SetMat4("viewMatrix", view_matrix);

            shader->SetInt("nLights", n_lights_);
            shader->SetVec3Array("lightDiffuses", MAX_N_LIGHTS, light_diffuses_);
            shader->SetVec3Array("lightDirections", MAX_N_LIGHTS, light_directions_);
            shader->SetVec3Array("lightPositions", MAX_N_LIGHTS, light_positions_);

            glCheckError();
        }
    }

    void DestroyEnvMap() {
        if (env_map_depth_rbo_) {
            glDeleteRenderbuffers(1, &env_map_depth_rbo_);
            env_map_depth_rbo_ = 0;
        }

        if (env_map_fbo_) {
            glDeleteFramebuffers(1, &env_map_fbo_);
            env_map_fbo_ = 0;
        }

        if (env_map_.name != 0) {
            glDeleteTextures(1, &env_map_.name);
            env_map_.name = 0;
        }
    }

    void DestroyWindow() {
        DestroyEnvMap();
        if (window_ != nullptr) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
    }

    static void HandleFramebufferSizeChange(GLFWwindow *window, int width, int height) {
        auto that = (Program *)glfwGetWindowUserPointer(window);
        std::cout << "INFO: Resized window to " << width << "x" << height << std::endl;
        that->display_width_ = width;
        that->display_height_ = height;
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
        double center_x = double(display_width_) / 2, center_y = double(display_height_) / 2;
        if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (!mouse_hold_) {
                glfwSetCursorPos(window_, center_x, center_y);
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                mouse_hold_ = true;
            } else {
                glfwGetCursorPos(window_, &mouse_x, &mouse_y);
                glfwSetCursorPos(window_, center_x, center_y);

                mouse_x = (mouse_x - center_x) / display_width_;
                mouse_y = (mouse_y - center_y) / display_height_;
                camera_->Rotate(float(-mouse_y), float(mouse_x));
            }
        } else {
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            mouse_hold_ = false;
        }
    }

    bool InitializeEnvMap() {
        glGenFramebuffers(1, &env_map_fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, env_map_fbo_);

        glGenTextures(1, &env_map_.name);
        glBindTexture(GL_TEXTURE_CUBE_MAP, env_map_.name);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        for (auto [face, _] : cube_map_faces) {
            glTexImage2D(face, 0, GL_RGBA, ENV_MAP_SIZE, ENV_MAP_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, env_map_.name, 0);

        glGenRenderbuffers(1, &env_map_depth_rbo_);
        glBindRenderbuffer(GL_RENDERBUFFER, env_map_depth_rbo_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, ENV_MAP_SIZE, ENV_MAP_SIZE);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, env_map_depth_rbo_);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR: Environment map framebuffer is not complete" << std::endl;
            return false;
        }

        return glCheckError() == GL_NO_ERROR;
    }

    void InitializeImGui() {
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io_ = &io;

        if (io.Fonts->AddFontFromFileTTF("resources/fonts/SourceCodePro-Regular.ttf", 16.0f) == nullptr) {
            io.Fonts->AddFontDefault();
        }
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    bool InitializeWindow() {

        // create window

        if (!glfwInit()) {
            std::cerr << "FATAL: Failed to initialize GLFW" << std::endl;
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        glfwWindowHint(GLFW_SAMPLES, 16);

        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        window_ = glfwCreateWindow(display_width_, display_height_, window_title_.c_str(), nullptr, nullptr);
        if (window_ == nullptr) {
            std::cerr << "FATAL: Failed to open GLFW window." << std::endl;
            return false;
        }
        glfwMakeContextCurrent(window_);

        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, &HandleFramebufferSizeChange);

        // load extensions

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "FATAL: Failed to initialize GLAD" << std::endl;
            return false;
        }

        std::cout << "INFO: OpenGL session created, version " << glGetString(GL_VERSION) << std::endl;

        // enable debug output

#ifdef NDEBUG
        GLint flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(HandleDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        } else {
            std::cerr << "WARNING: OpenGL context is not debug context" << std::endl;
        }
#endif

        // enable additional features

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_MULTISAMPLE);

        return glCheckError() == GL_NO_ERROR;
    }

    static void HandleDebugOutput(GLenum source,
                                  GLenum type,
                                  unsigned int id,
                                  GLenum severity,
                                  GLsizei length,
                                  const char *message,
                                  const void *userParam) {
        std::cerr << "GL_DEBUG_OUTPUT: " << message << std::endl;

        switch (source) {
        case GL_DEBUG_SOURCE_API:
            std::cerr << "API: ";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            std::cerr << "WINDOW_SYSTEM: ";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            std::cerr << "SHADER_COMPILER: ";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            std::cerr << "THIRD_PARTY: ";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            std::cerr << "APPLICATION: ";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            std::cerr << "OTHER: ";
            break;
        default:
            std::cerr << "UNKNOWN: ";
            break;
        }

        switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            std::cerr << "ERROR: ";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            std::cerr << "DEPRECATED_BEHAVIOR: ";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            std::cerr << "UNDEFINED_BEHAVIOR: ";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            std::cerr << "PORTABILITY: ";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            std::cerr << "PERFORMANCE: ";
            break;
        case GL_DEBUG_TYPE_OTHER:
            std::cerr << "OTHER: ";
            break;
        default:
            std::cerr << "UNKNOWN: ";
            break;
        }

        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            std::cerr << "HIGH: ";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            std::cerr << "MEDIUM: ";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            std::cerr << "LOW: ";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            std::cerr << "NOTIFICATION: ";
            break;
        default:
            std::cerr << "UNKNOWN: ";
            break;
        }

        std::cerr << message << std::endl;
    }
};

#endif
