#include "lib/cameras/camera_tp.h"
#include "lib/kinematics/bone.h"
#include "lib/kinematics/inverse.h"
#include "lib/program.h"

class InverseKinematicsProgram : public Program {
public:
    InverseKinematicsProgram() {
        delete (FirstPersonCamera *)camera_;
        camera_ = new ThirdPersonCamera(-50.0f, 100.f, 100.0f, -20.0f, 315.0f);
    }

    bool Initialize(const std::string &window_title) {
        display_width_ = 1920;
        display_height_ = 1080;

        if (!Program::Initialize(window_title, false)) {
            return false;
        }

        shader_ = new ShaderProgram("shaders/phong.vert", "shaders/cook-torrance.frag");
        if (!shader_->IsReady()) {
            printf("FATAL: Failed to initialize shaders.\n");
            return false;
        }
        shaders_.push_back(shader_);

        if (!Scene::CreateFromFile("resources/models/robotic_arm/scene.gltf", arm_, textures_) ||
            !Scene::CreateFromFile("resources/models/table/scene.gltf", floor_, textures_) ||
            !Scene::CreateFromFile("resources/models/axis/scene.gltf", target_, textures_)) {

            std::cerr << "FATAL: Failed to load scene" << std::endl;
            return false;
        }

        arm_->Initialize();
        arm_->Scale(1.0f);

        if (!arm_->FindByName("Shoulder", base_node_) || !arm_->FindByName("UpperArm", upper_arm_) ||
            !arm_->FindByName("ForeArm", fore_arm_) || !arm_->FindByName("Hand", hand_) ||
            !arm_->FindByName("Finger", finger_)) {
            std::cerr << "FATAL: Failed to find animation objects" << std::endl;
            return false;
        }

        floor_->Initialize();
        floor_->Scale(1.0f);
        floor_->Translate(-4.0f, 0.0f, 1.0f);

        target_->Initialize();
        target_->Scale(0.005f);

        SetLightCount(4);
        SetLight(0, glm::vec3(50.0f, 100.0f, 50.0f), glm::vec3(0.0f));
        SetLight(1, glm::vec3(-50.0f, 50.0f, 50.0f), glm::vec3(0.0f));
        SetLight(2, glm::vec3(50.0f, 50.0f, -50.0f), glm::vec3(0.0f));
        SetLight(3, glm::vec3(-50.0f, 50.0f, -50.0f), glm::vec3(0.0f));

        InitializeKinematics();

        return glCheckError() == GL_NO_ERROR;
    };

private:
    Scene *arm_;
    Scene *floor_;
    Scene *target_;

    Node *base_node_;
    Node *upper_arm_;
    Node *fore_arm_;
    Node *hand_;
    Node *finger_;

    BoneChain *chain_;
    std::vector<Bone *> joints_;

    float target_x = 100.0f, target_y = 50.0f, target_z = 50.0f;

    bool enable_animation_ = false;
    bool enable_solver_ = true;
    float velocity_ = 0.001f;

    ShaderProgram *shader_;

    TextureManager textures_;

    void Draw() override {
        Program::Draw();

        if (enable_animation_) {
            auto pi = 3.1415926535897932384626433832795;
            auto t = fmod(current_frame_clock_ * 1.0f, 2.0 * pi) - pi;
            target_y = float(-abs(t) * cos(pi * sin(t) / t) * 25.0f) + 100.0f;
            target_z = float(t * sin(pi * .872 * sin(t) / t) * 25.0f);
        }
        target_->SetPosition(target_x, target_y, target_z);

        shader_->Use();

        arm_->Draw(shader_);
        floor_->Draw(shader_);
        target_->Draw(shader_);

        if (enable_solver_) {
            chain_->Solve(glm::vec3(target_x, target_y, target_z), float(last_frame_time_) * velocity_);
        }

        glCheckError();
    }

    void DrawImGui() override {
        Program::DrawImGui();
        ImGui::Begin("Inverse Kinematics: Settings");

        ImGui::TreeNode("Solver");
        ImGui::SliderFloat("Velocity", &velocity_, 0.005f, 0.01f);
        ImGui::Checkbox("Enable Solver", &enable_solver_);

        ImGui::TreeNode("Target");
        ImGui::SliderFloat("Target X", &target_x, -150.0f, 150.0f);
        ImGui::SliderFloat("Target Y", &target_y, 0.0f, 150.0f);
        ImGui::SliderFloat("Target Z", &target_z, -150.0f, 150.0f);
        ImGui::Checkbox("Enable Target Animation", &enable_animation_);
        ImGui::End();
    }

    void InitializeKinematics() {
        auto disabled = glm::vec2(0.0f, 0.0f);
        auto speed = glm::radians(0.5f);

        auto base = new Bone(glm::vec3(0.0f, 0.0f, 0.0f),
                             glm::vec3(0.0f, 0.0f, 5.0f),
                             disabled,
                             disabled,
                             glm::vec2(-114514.0f, 114514.0f),
                             speed,
                             base_node_);
        auto upper = new Bone(glm::vec3(0.0f, 0.0f, 0.0f),
                              glm::vec3(0.0f, 0.0f, 48.5f),
                              disabled,
                              glm::vec2(glm::radians(-45.0f), glm::radians(90.0f)),
                              disabled,
                              speed,
                              upper_arm_);
        auto fore = new Bone(glm::vec3(0.0f, 0.0f, 0.0f),
                             glm::vec3(40.0f, 0.0f, 8.0f),
                             disabled,
                             glm::vec2(glm::radians(-180.0f), glm::radians(0.0f)),
                             disabled,
                             speed,
                             fore_arm_);
        auto hand = new Bone(glm::vec3(0.0f, 0.0f, 0.0f),
                             glm::vec3(12.5f, 0.0f, 0.0f),
                             glm::vec2(-114514.0f, 114514.0f),
                             disabled,
                             disabled,
                             speed,
                             hand_);
        auto finger = new Bone(glm::vec3(0.0f, -3.5f, 0.0f),
                               glm::vec3(0.0f, 0.0f, -20.0f),
                               disabled,
                               glm::vec2(glm::radians(-180.0f), glm::radians(0.0f)),
                               disabled,
                               speed,
                               finger_);

        joints_.push_back(base);
        joints_.push_back(upper);
        joints_.push_back(fore);
        joints_.push_back(hand);
        joints_.push_back(finger);

        chain_ = new BoneChain(joints_);
    }
};

int main() {
    InverseKinematicsProgram program;
    if (!program.Initialize("CS7GV5: Inverse Kinematics")) {
        return -1;
    }
    program.Run();
}