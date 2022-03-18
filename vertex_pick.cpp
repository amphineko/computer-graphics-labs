#include "lib/program.h"

class VertexPickProgram : public Program {
public:
    bool Initialize(const std::string &window_title, bool env_map) override {
        if (!Program::Initialize(window_title, env_map)) {
            return false;
        }

        phong_shader_ = new ShaderProgram("shaders/phong.vert", "shaders/blinn-phong.frag");
        shaders_.push_back(phong_shader_);

        picker_shader_ = new VertexPickerShaderProgram("shaders/vertex-pick.vert");
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

        if (!Scene::CreateFromFile("resources/models/axis/scene.gltf", axis_, texture_manager_)) {
            return false;
        }
        axis_->Initialize();
        axis_->Scale(0.001f);

        SetLight(glm::vec3(0.0f, 10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f));

        camera_->SetPosition(-10.0f, 10.0f, 20.0f);
        camera_->SetRotation(0.0f, -60.0f);
    }

private:
    ShaderProgram *phong_shader_;
    ShaderProgram *picker_shader_;

    Scene *obj_, *axis_;

    double mouse_x_, mouse_y_;
    MeshVertexPickResult vertex_;

    void Draw() override {
        Program::Draw();

        // draw objects

        phong_shader_->Use();
        obj_->Draw(phong_shader_);
        axis_->Draw(phong_shader_);

        // pick vertex from the screen

        picker_shader_->Use();

        glfwGetCursorPos(window_, &mouse_x_, &mouse_y_);
        mouse_x_ = mouse_x_ / float(display_width_) * 2.0 - 1.0;
        mouse_y_ = mouse_y_ / float(display_height_) * -2.0 + 1.0;

        picker_shader_->SetVec2("mousePos", glm::vec2(mouse_x_, mouse_y_));

        glCheckError();

        vertex_.distance = std::numeric_limits<float>::max();
        obj_->Pick(vertex_, picker_shader_);

        axis_->SetPosition(vertex_.world_position.x, vertex_.world_position.y, vertex_.world_position.z);
    }

    void DrawImGui() override {
        ImGui::Begin("Vertex Picker");

        ImGui::TreeNode("Cursor");
        ImGui::Text("On-screen coordinates: (%f, %f)", mouse_x_, mouse_y_);

        ImGui::TreeNode("Picked vertex");
        ImGui::Text("Distance to cursor: %f", vertex_.distance);
        ImGui::Text("World Position: (%f, %f, %f)",
                    vertex_.world_position.x,
                    vertex_.world_position.y,
                    vertex_.world_position.z);

        ImGui::End();
    }
};

int main() {
    VertexPickProgram program;
    program.Initialize("CS7GV5: Vertex Pick", false);
    program.Run();
    return 0;
}
