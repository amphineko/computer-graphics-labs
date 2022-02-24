#include "lib/cameras/camera_tp.h"
#include "lib/kinematics/bone.h"
#include "lib/kinematics/inverse.h"
#include "lib/program.h"
#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/local_to_model_job.h"
#include "ozz/animation/runtime/sampling_job.h"
#include "ozz/animation/runtime/skeleton.h"
#include "ozz/base/containers/vector.h"
#include "ozz/base/io/archive.h"
#include "ozz/base/maths/simd_math.h"
#include "ozz/base/maths/soa_transform.h"
#include "ozz/base/maths/vec_float.h"

class InverseKinematicsProgram : public Program {
public:
    InverseKinematicsProgram() {
        delete (FirstPersonCamera *)camera_;
        camera_ = new ThirdPersonCamera(50.0f, 100.f, 150.0f, -20.0f, 270.0f);
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
            !Scene::CreateFromFile("resources/models/table/scene.gltf", table_, textures_) ||
            !Scene::CreateFromFile("resources/models/axis/scene.gltf", axis_, textures_) ||
            !Scene::CreateFromFile("resources/models/crate/scene.gltf", crate_, textures_)) {

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

        table_->Initialize();
        table_->Scale(1.0f);
        table_->Translate(-4.0f, 0.0f, 1.0f);

        axis_->Initialize();
        axis_->Scale(0.005f);

        crate_->Initialize();
        crate_->Scale(10.0f);
        crate_->Translate(100.0f, 0.0f, 0.0f);

        SetLightCount(4);
        SetLight(0, glm::vec3(0.0f, 100.0f, 150.0f), glm::vec3(0.0f));
        SetLight(1, glm::vec3(-50.0f, 50.0f, 50.0f), glm::vec3(0.0f));
        SetLight(2, glm::vec3(50.0f, 50.0f, -50.0f), glm::vec3(0.0f));
        SetLight(3, glm::vec3(-50.0f, 50.0f, -50.0f), glm::vec3(0.0f));

        InitializeKinematics();

        if (!InitializeAnimation()) {
            return false;
        }

        return glCheckError() == GL_NO_ERROR;
    };

private:
    Scene *arm_, *table_, *axis_, *crate_;
    Node *base_node_, *upper_arm_, *fore_arm_, *hand_, *finger_;

    BoneChain *chain_;
    std::vector<Bone *> joints_;

    BoneChain *chain_locked_;
    std::vector<Bone *> joints_locked_;
    bool two_bones_ = false;

    ozz::animation::Animation animation_;
    ozz::animation::Skeleton skeleton_;

    ozz::animation::SamplingJob sampling_job_;
    ozz::animation::SamplingJob::Context sampling_context_;
    ozz::vector<ozz::math::SoaTransform> animation_transforms_;

    ozz::animation::LocalToModelJob local_to_model_job_;
    ozz::vector<ozz::math::Float4x4> animation_models_;

    bool enable_animation_ = false;
    float animation_speed_ = 7.5f;

    float target_x = 25.0f, target_y = 50.0f, target_z = 0.0f;

    bool enable_solver_ = true;
    float velocity_ = 0.005f;

    bool track_crate_ = false;
    bool track_sine_ = false;

    bool draw_end_position_ = true;
    bool draw_model_ = true;
    bool draw_target_ = true;

    ShaderProgram *shader_;

    TextureManager textures_;

    void Draw() override {
        Program::Draw();

        // sine function animation

        if (track_sine_) {
            auto pi = 3.1415926535897932384626433832795;
            auto t = fmod(current_frame_clock_ * 1.0f, 2.0 * pi) - pi;
            target_y = float(-abs(t) * cos(pi * sin(t) / t) * 25.0f) + 100.0f;
            target_z = float(t * sin(pi * .872 * sin(t) / t) * 25.0f);
        }

        // scripted animation

        sampling_job_.ratio =
            enable_animation_
                ? float(fmod(current_frame_clock_ / animation_speed_, animation_.duration())) / animation_.duration()
                : 0.0f;
        if (sampling_job_.Run() && local_to_model_job_.Run() && enable_animation_) {
            ozz::math::SimdFloat4 base_position{0.0f, 0.0f, 0.0f, 1.0f};
            auto position = TransformPoint(animation_models_[animation_models_.size() - 1], base_position);

            auto x = position.x * 10.0f + 50.0f;
            auto y = position.y * 15.0f + 25.0f;
            auto z = position.z * -15.0f + 75.0f;
            crate_->SetPosition(x, y - 25.0f, z);

            if (track_crate_) {
                target_x = x;
                target_y = y;
                target_z = z;
            }
        }

        // draw

        shader_->Use();

        if (draw_model_) {
            arm_->Draw(shader_);
        }

        crate_->Draw(shader_);
        table_->Draw(shader_);

        if (draw_end_position_) {
            for (auto &joint : joints_) {
                auto end = joint->GetEndPosition();
                axis_->SetPosition(end.x, end.y, end.z);

                float x, y, z;
                glm::extractEulerAngleXYZ(joint->GetWorldTransform(), x, y, z);
                axis_->SetRotation(glm::vec3(x, y, z));

                axis_->Draw(shader_);
            }
        }

        if (draw_target_) {
            axis_->SetPosition(target_x, target_y, target_z);
            axis_->Draw(shader_);
        }

        if (enable_solver_) {
            if (two_bones_) {
                chain_locked_->Solve(glm::vec3(target_x, target_y, target_z), float(last_frame_time_) * velocity_);
            } else {
                chain_->Solve(glm::vec3(target_x, target_y, target_z), float(last_frame_time_) * velocity_);
            }
        }

        glCheckError();
    }

    void DrawImGui() override {
        Program::DrawImGui();
        ImGui::Begin("Inverse Kinematics: Settings");

        ImGui::TreeNode("Solver");
        ImGui::SliderFloat("Velocity", &velocity_, 0.005f, 0.01f);
        ImGui::Checkbox("Enable Solver", &enable_solver_);
        ImGui::Checkbox("Simple 2-bone only", &two_bones_);
        ImGui::Checkbox("Track Crate", &track_crate_);
        ImGui::Checkbox("Track Sine Function", &track_sine_);

        ImGui::TreeNode("Target");
        ImGui::SliderFloat("Target X", &target_x, -150.0f, 150.0f);
        ImGui::SliderFloat("Target Y", &target_y, 0.0f, 150.0f);
        ImGui::SliderFloat("Target Z", &target_z, -150.0f, 150.0f);

        ImGui::TreeNode("Animation");
        ImGui::Checkbox("Enable Animation", &enable_animation_);
        ImGui::SliderFloat("Animation Speed", &animation_speed_, 1.0f, 10.0f);

        ImGui::TreeNode("Rendering");
        ImGui::Checkbox("Draw End Position", &draw_end_position_);
        ImGui::Checkbox("Draw Model", &draw_model_);
        ImGui::Checkbox("Draw Target", &draw_target_);

        ImGui::End();
    }

    bool InitializeAnimation() {
        if (!LoadOzzArchive("resources/models/crate/CrateAction.001.ozz", animation_)) {
            std::cerr << "FATAL: Failed to load animation" << std::endl;
            return false;
        }

        if (!LoadOzzArchive("resources/models/crate/skeleton.ozz", skeleton_)) {
            std::cerr << "FATAL: Failed to load skeleton" << std::endl;
            return false;
        }

        sampling_context_.Resize(skeleton_.num_joints());

        animation_transforms_.resize(skeleton_.num_soa_joints());
        sampling_job_.animation = &animation_;
        sampling_job_.context = &sampling_context_;
        sampling_job_.output = ozz::make_span(animation_transforms_);
        sampling_job_.ratio = 1.0f;

        animation_models_.resize(skeleton_.num_joints());
        local_to_model_job_.skeleton = &skeleton_;
        local_to_model_job_.input = ozz::make_span(animation_transforms_);
        local_to_model_job_.output = ozz::make_span(animation_models_);

        return true;
    }

    void InitializeKinematics() {
        auto disabled = glm::vec2(0.0f, 0.0f);
        auto speed = glm::radians(0.5f);

        auto base_origin = glm::vec3(0.0f, 0.0f, 0.0f);
        auto base_length = glm::vec3(0.0f, 0.0f, 5.0f);
        auto base_rot_z = glm::vec2(-114514.0f, 114514.0f);
        auto base = new Bone(base_origin, base_length, disabled, disabled, base_rot_z, speed, base_node_);
        auto base_locked = new Bone(base_origin, base_length, disabled, disabled, disabled, speed, base_node_);

        auto upper = new Bone(glm::vec3(0.0f, 0.0f, 0.0f),
                              glm::vec3(0.0f, 0.0f, 48.5f),
                              disabled,
                              glm::vec2(glm::radians(-45.0f), glm::radians(90.0f)),
                              disabled,
                              speed,
                              upper_arm_);

        auto fore_origin = glm::vec3(0.0f, 0.0f, 0.0f);
        auto fore_length = glm::vec3(40.0f, 0.0f, 8.0f);
        auto fore = new Bone(fore_origin,
                             fore_length,
                             disabled,
                             glm::vec2(glm::radians(-180.0f), glm::radians(0.0f)),
                             disabled,
                             speed,
                             fore_arm_);

        auto hand_origin = glm::vec3(0.0f, 0.0f, 0.0f);
        auto hand_length = glm::vec3(12.5f, 0.0f, 0.0f);
        auto hand_rot_x = glm::vec2(-114514.0f, 114514.0f);
        auto hand = new Bone(hand_origin, hand_length, hand_rot_x, disabled, disabled, speed, hand_);
        auto hand_locked = new Bone(hand_origin, hand_length, disabled, disabled, disabled, speed, hand_);

        auto finger_origin = glm::vec3(0.0f, -3.5f, 0.0f);
        auto finger_length = glm::vec3(0.0f, 0.0f, -20.0f);
        auto finger = new Bone(finger_origin,
                               finger_length,
                               disabled,
                               glm::vec2(glm::radians(-180.0f), glm::radians(0.0f)),
                               disabled,
                               speed,
                               finger_);
        auto finger_locked = new Bone(finger_origin, finger_length, disabled, disabled, disabled, speed, finger_);

        joints_.push_back(base);
        joints_.push_back(upper);
        joints_.push_back(fore);
        joints_.push_back(hand);
        joints_.push_back(finger);
        chain_ = new BoneChain(joints_);

        joints_locked_.push_back(base_locked);
        joints_locked_.push_back(upper);
        joints_locked_.push_back(fore);
        joints_locked_.push_back(hand_locked);
        joints_locked_.push_back(finger_locked);
        chain_locked_ = new BoneChain(joints_locked_);
    }

    template <typename T> static bool LoadOzzArchive(const std::string &path, T &output) {
        ozz::io::File file(path.c_str(), "rb");
        if (!file.opened()) {
            std::cerr << "FATAL: Failed to open archive file " << path << std::endl;
            return false;
        }

        ozz::io::IArchive archive(&file);
        if (!archive.TestTag<T>()) {
            return false;
        }

        archive >> output;

        return true;
    }
};

int main() {
    InverseKinematicsProgram program;
    if (!program.Initialize("CS7GV5: Inverse Kinematics")) {
        return -1;
    }
    program.Run();
}