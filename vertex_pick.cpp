#include "lib/program.h"

#define VERTEX_POSITION_RANGE 50.0f

const std::vector<std::pair<std::string, std::string>> delta_shapes_ = {
    {"jaw_open", "resources/models/high-res-blendshapes/Mery_jaw_open.obj"},
    {"kiss", "resources/models/high-res-blendshapes/Mery_kiss.obj"},
    {"l_brow_lower", "resources/models/high-res-blendshapes/Mery_l_brow_lower.obj"},
    {"l_brow_narrow", "resources/models/high-res-blendshapes/Mery_l_brow_narrow.obj"},
    {"l_brow_raise", "resources/models/high-res-blendshapes/Mery_l_brow_raise.obj"},
    {"l_eye_closed", "resources/models/high-res-blendshapes/Mery_l_eye_closed.obj"},
    {"l_eye_lower_open", "resources/models/high-res-blendshapes/Mery_l_eye_lower_open.obj"},
    {"l_eye_upper_open", "resources/models/high-res-blendshapes/Mery_l_eye_upper_open.obj"},
    {"l_nose_wrinkle", "resources/models/high-res-blendshapes/Mery_l_nose_wrinkle.obj"},
    {"l_puff", "resources/models/high-res-blendshapes/Mery_l_puff.obj"},
    {"l_sad", "resources/models/high-res-blendshapes/Mery_l_sad.obj"},
    {"l_smile", "resources/models/high-res-blendshapes/Mery_l_smile.obj"},
    {"l_suck", "resources/models/high-res-blendshapes/Mery_l_suck.obj"},
    {"r_brow_lower", "resources/models/high-res-blendshapes/Mery_r_brow_lower.obj"},
    {"r_brow_narrow", "resources/models/high-res-blendshapes/Mery_r_brow_narrow.obj"},
    {"r_brow_raise", "resources/models/high-res-blendshapes/Mery_r_brow_raise.obj"},
    {"r_eye_closed", "resources/models/high-res-blendshapes/Mery_r_eye_closed.obj"},
    {"r_eye_lower_open", "resources/models/high-res-blendshapes/Mery_r_eye_lower_open.obj"},
    {"r_eye_upper_open", "resources/models/high-res-blendshapes/Mery_r_eye_upper_open.obj"},
    {"r_nose_wrinkle", "resources/models/high-res-blendshapes/Mery_r_nose_wrinkle.obj"},
    {"r_puff", "resources/models/high-res-blendshapes/Mery_r_puff.obj"},
    {"r_sad", "resources/models/high-res-blendshapes/Mery_r_sad.obj"},
    {"r_smile", "resources/models/high-res-blendshapes/Mery_r_smile.obj"},
    {"r_suck", "resources/models/high-res-blendshapes/Mery_r_suck.obj"},
};

class VertexPickProgram : public Program {
public:
    bool Initialize(const std::string &window_title, bool env_map) override {
        if (!Program::Initialize(window_title, env_map)) {
            return false;
        }

        phong_shader_ = new ShaderProgram("shaders/phong-delta.vert", "shaders/blinn-phong.frag");
        shaders_.push_back(phong_shader_);

        picker_shader_ = new FeedbackShaderProgram("shaders/vertex-pick.vert");
        shaders_.push_back(picker_shader_);

        for (auto shader : shaders_) {
            if (!shader->IsReady()) {
                return false;
            }
        }

        if (!Scene::CreateFromFile("resources/models/high-res-blendshapes/neutral.obj", obj_, texture_manager_)) {
            return false;
        }
        obj_->Initialize();
        obj_->Scale(0.5f);
        for (const auto &shape : delta_shapes_) {
            if (!Scene::LoadDeltaFromFile(shape.second.c_str(), obj_)) {
                return false;
            }
            current_delta_weights_.push_back(0.0f);
            end_delta_weights_.push_back(0.0f);
        }

        if (!Scene::CreateFromFile("resources/models/axis/scene.gltf", axis_, texture_manager_)) {
            return false;
        }
        axis_->Initialize();
        axis_->Scale(0.001f);

        SetLight(glm::vec3(0.0f, 10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f));

        camera_->SetPosition(-10.0f, 10.0f, 20.0f);
        camera_->SetRotation(0.0f, -60.0f);

        LoadAnimation();

        return true;
    }

private:
    ShaderProgram *phong_shader_;
    ShaderProgram *picker_shader_;

    Scene *obj_, *axis_;

    double mouse_x_, mouse_y_;
    MeshVertexPickResult vertex_, selected_vertex_{};

    std::vector<float> end_delta_weights_, current_delta_weights_;
    std::vector<std::vector<float>> weight_frames_;
    size_t current_animation_frame_ = 0, last_animation_frame_ = 0;
    bool enable_animation_ = false;

    void Draw() override {
        Program::Draw();

        // draw objects

        phong_shader_->Use();
        obj_->Draw(phong_shader_);

        if (selected_vertex_.vertex != nullptr) {
            auto position = selected_vertex_.world_position;
            axis_->SetPosition(position.x, position.y, position.z);

            axis_->Draw(phong_shader_);
        }

        // pick vertex from the screen

        picker_shader_->Use();

        glfwGetCursorPos(window_, &mouse_x_, &mouse_y_);
        mouse_x_ = mouse_x_ / float(window_width_) * 2.0 - 1.0;
        mouse_y_ = mouse_y_ / float(window_height_) * -2.0 + 1.0;

        picker_shader_->SetVec2("mousePos", glm::vec2(mouse_x_, mouse_y_));

        glCheckError();

        vertex_.distance = std::numeric_limits<float>::max();
        obj_->Pick(vertex_, picker_shader_);

        // register manipulator

        if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            selected_vertex_ = vertex_;
        }

        // linear weight effector

        float total_updated_delta = 0.0f;
        for (size_t i = 0; i < delta_shapes_.size(); ++i) {
            auto current = current_delta_weights_[i];
            auto target = end_delta_weights_[i];
            if (current != target) {
                auto delta = float((target - current) * 5.0f * last_frame_time_);
                if (std::abs(delta) > 0.0005f) {
                    current_delta_weights_[i] = current += delta;
                } else {
                    current_delta_weights_[i] = current = target;
                }
                total_updated_delta += std::abs(delta);
                obj_->SetDeltaWeight(i, current);
            }
        }
        if (total_updated_delta > 0.0001f) {
            obj_->UpdateDeltaWeights();
        }

        // animated weights

        if (enable_animation_) {
            if (total_updated_delta < 0.0005f) {
                ++current_animation_frame_;
                if (current_animation_frame_ >= weight_frames_.size()) {
                    current_animation_frame_ = 0;
                }

                for (size_t i = 0; i < delta_shapes_.size(); ++i) {
                    end_delta_weights_[i] = weight_frames_[current_animation_frame_][i];
                }
            }
        }
    }

    void DrawImGui() override {
        Program::DrawImGui();

        ImGui::Begin("Vertex Picker");

        if (ImGui::TreeNodeEx("Cursor", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("X: %f", mouse_x_);
            ImGui::Text("Y: %f", mouse_y_);
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Picked vertex", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Cursor dist.: %f", vertex_.distance);
            ImGui::Text("World X: %f", vertex_.world_position.x);
            ImGui::Text("World Y: %f", vertex_.world_position.y);
            ImGui::Text("World Z: %f", vertex_.world_position.z);

            if (selected_vertex_.vertex != nullptr) {
                auto x = selected_vertex_.vertex->position.x;
                auto y = selected_vertex_.vertex->position.y;
                auto z = selected_vertex_.vertex->position.z;

                if (ImGui::SliderFloat("X", &x, -VERTEX_POSITION_RANGE, VERTEX_POSITION_RANGE) ||
                    ImGui::SliderFloat("Y", &y, -VERTEX_POSITION_RANGE, VERTEX_POSITION_RANGE) ||
                    ImGui::SliderFloat("Z", &z, -VERTEX_POSITION_RANGE, VERTEX_POSITION_RANGE)) {

                    selected_vertex_.update_position(glm::vec3(x, y, z));
                }
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Weights", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Checkbox("Animation", &enable_animation_)) {
                current_animation_frame_ = last_animation_frame_ = 0.0f;
            }

            if (enable_animation_) {
                ImGui::Text("Current frame: %zu", current_animation_frame_);
            }

            for (size_t i = 0; i < end_delta_weights_.size(); ++i) {
                auto weight = end_delta_weights_[i];
                if (ImGui::SliderFloat(delta_shapes_[i].first.c_str(), &weight, 0.0f, 1.0f)) {
                    end_delta_weights_[i] = weight;
                }
            }

            ImGui::TreePop();
        }

        ImGui::End();
    }

    void LoadAnimation() {
        std::ifstream file("resources/models/high-res-blendshapes/animation.txt");
        float weight = 0.0f;

        while (file >> weight) {
            if (weight_frames_.empty() || weight_frames_.back().size() == delta_shapes_.size()) {
                weight_frames_.emplace_back();
            }

            weight_frames_.back().push_back(weight);
        }

        if (weight_frames_.back().size() != delta_shapes_.size()) {
            std::cerr << "ERROR: Animation file has wrong number of weights" << std::endl;
            weight_frames_.pop_back();
        }

        file.close();
    }
};

int main() {
    VertexPickProgram program;
    if (program.Initialize("CS7GV5: Vertex Pick", false)) {
        program.Run();
        return 0;
    } else {
        return 1;
    }
}
