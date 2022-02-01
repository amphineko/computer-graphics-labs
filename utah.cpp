#include "utah.h"

UtahProgram::UtahProgram() {
    delete (FirstPersonCamera *)camera_;

    camera_ = new ThirdPersonCamera(0.0f, 37.5f, 75.0f, -30.0f, 270.0f);
}

void UtahProgram::Draw() {
    Program::Draw();

    if (model_cone_ == nullptr || model_teapot_ == nullptr) {
        return;
    }

    auto current_clock = glfwGetTime();
    auto delta_time = current_clock - last_frame_clock_;
    last_frame_clock_ = current_clock;

    if (shader_basic_ != nullptr) {
        shader_basic_->Use();
    }

    auto cone_roll = model_cone_->GetRoll() + float(delta_time) * 100.0f;
    model_cone_->SetRotation(-90.0f, cone_roll, 0.0f);
    model_cone_->Draw(shader_basic_);

    auto teapot_yaw = model_teapot_->GetYaw() + float(delta_time) * 100.0f;
    model_teapot_->SetRotation(0.0f, 0.0f, teapot_yaw);
    model_teapot_->Draw(shader_basic_);
}

bool UtahProgram::Initialize(std::string window_title) {
    if (!Program::Initialize(window_title)) {
        return false;
    }

    shader_basic_ = new ShaderProgram("utah-basic.vert", "utah-basic.frag");
    if (!shader_basic_->IsReady()) {
        shader_basic_->~ShaderProgram();
        return false;
    }
    shaders_.push_back(shader_basic_);

    model_cone_ = new Model();
    model_cone_->LoadSceneFromFile("models/cone/scene.gltf");
    model_cone_->Initialize();
    model_cone_->SetPosition(20.0f, -5.0f, 20.0f);
    model_cone_->SetScale(0.75f);

    model_teapot_ = new Model();
    model_teapot_->LoadSceneFromFile("models/utah-teapot.obj");
    model_teapot_->Initialize();
    model_teapot_->SetPosition(0.0f, 0.0f, 0.0f);
    model_teapot_->SetScale(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    return true;
}
