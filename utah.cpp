#include "utah.h"

UtahProgram::UtahProgram() {
    delete (FirstPersonCamera *)camera_;

    camera_ = new ThirdPersonCamera(0.0f, 50.0f, 60.0f, -30.0f, 270.0f);
}

void UtahProgram::Draw() {
    Program::Draw();

    if (model_obj1_ == nullptr || model_obj2_ == nullptr) {
        return;
    }

    auto current_clock = glfwGetTime();
    auto delta_time = current_clock - last_frame_clock_;
    last_frame_clock_ = current_clock;

    current_shader_->Use();

    current_shader_->SetVec3("light_ambient", glm::vec3(0.05f, 0.05f, 0.05f));
    current_shader_->SetVec3("light_diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
    current_shader_->SetVec3("light_specular", glm::vec3(1.0f, 1.0f, 1.0f));

    auto obj1_yaw = model_obj1_->GetYaw() + float(delta_time) * 100.0f;
    model_obj1_->SetRotation(-90.0f, 0.0f, obj1_yaw);
    model_obj1_->Draw(current_shader_);

    auto obj2_yaw = model_obj2_->GetYaw() + float(delta_time) * 100.0f;
    model_obj2_->SetRotation(0.0f, 0.0f, obj2_yaw);
    model_obj2_->Draw(current_shader_);

    HandleKeyboardInput(0);
}

bool UtahProgram::Initialize(const std::string *window_title) {
    if (!Program::Initialize(window_title)) {
        return false;
    }

    shaders_.push_back(new ShaderProgram("utah-phong.vert", "utah-cook-torrance.frag"));
    shaders_.push_back(new ShaderProgram("utah-phong.vert", "utah-phong.frag"));
    shaders_.push_back(new ShaderProgram("utah-phong.vert", "utah-gooch.frag"));
    shaders_.push_back(new ShaderProgram("utah-phong.vert", "utah-normal.frag"));
    current_shader_ = shaders_[0];

    model_obj1_ = new Model();
    model_obj1_->LoadSceneFromFile("models/fancy_teapot/scene.gltf");
    model_obj1_->Initialize();
    model_obj1_->SetPosition(20.0f, 0.0f, 0.0f);
    model_obj1_->SetScale(2.0f);

    model_obj2_ = new Model();
    model_obj2_->LoadSceneFromFile("models/teapot/scene.gltf");
    model_obj2_->Initialize();
    model_obj2_->SetPosition(-20.0f, 0.0f, 0.0f);
    model_obj2_->SetScale(2.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    return true;
}

void UtahProgram::HandleKeyboardInput(double) {
    int shader_id = -1;
    if (glfwGetKey(window_, GLFW_KEY_F1) == GLFW_PRESS)
        shader_id = 0;
    if (glfwGetKey(window_, GLFW_KEY_F2) == GLFW_PRESS)
        shader_id = 1;
    if (glfwGetKey(window_, GLFW_KEY_F3) == GLFW_PRESS)
        shader_id = 2;
    if (glfwGetKey(window_, GLFW_KEY_F4) == GLFW_PRESS)
        shader_id = 3;
    if (shader_id != -1)
        current_shader_ = shaders_[std::max(0, std::min((int)shaders_.size(), shader_id))];
}
