#include "utah.h"

UtahProgram::UtahProgram() : Program("utah.vert", "utah.frag") {
    camera_->SetPosition(5.0f, 30.0f, 50.0f);
    camera_->SetRotation(-30.0f, 270.0f);
    camera_->TranslateSpeed = 10.0f;
}

void UtahProgram::Draw() {
    Program::Draw();

    if (model_ == nullptr || model_small_ == nullptr) {
        return;
    }

    auto current_clock = glfwGetTime();
    auto delta_time = current_clock - last_frame_clock_;
    last_frame_clock_ = current_clock;

    shader_->SetVec3("camera_position", camera_->GetPosition());
    shader_->SetVec3("light_direction", glm::vec3(0.0f, 0.0f, 0.0f));
    shader_->SetVec3("light_position", glm::vec3(5.0f, 30.0f, 50.0f));

    model_->SetRotation(0, 0, model_->GetYaw() + float(delta_time) * 500.0f);
    model_->Draw(shader_);

    model_small_->SetRotation(-90.0f, model_small_->GetRoll() - float(delta_time) * 100.0f, 0.0f);
    model_small_->Draw(shader_);
}

bool UtahProgram::Initialize(std::string window_title) {
    if (!Program::Initialize(window_title)) {
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    return true;
}

void UtahProgram::InitializeObjects() {
    model_ = new Model();
    model_->LoadSceneFromFile("models/utah-teapot.obj");
    model_->Initialize();
    model_->SetPosition(0.0f, 0.0f, 0.0f);

    model_small_ = new Model();
    model_small_->LoadSceneFromFile("models/cone/source/Conus_LP.fbx");
    model_small_->Initialize();
    model_small_->SetPosition(20.0f, 0.0f, 20.0f);
    model_small_->SetScale(0.25f);
}
