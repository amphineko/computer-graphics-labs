#include "glm/gtx/rotate_vector.hpp"
#include "imgui.h"

#include "lib/cameras/camera_tp.h"
#include "lib/program.h"

class MipMapProgram : public Program {
public:
    MipMapProgram() {
        delete (FirstPersonCamera *)camera_;
        camera_ = new ThirdPersonCamera(0.0f, 0.1f, 5.0f, 0.0f, 270.0f);
        SetLightCount(1);
    }

    bool Initialize(const std::string &window_title) { return Initialize(window_title, true); }

protected:
    void Draw() override {
        Program::Draw();

        if (!mouse_hold_) {
            for (auto checkerboard : checkerboards_) {
                checkerboard->Rotate(0.0f, last_frame_time_ * 0.05f, 0.0f);
            }
        }

        current_shader->Use();
        current_obj_->Draw(current_shader);
    }

private:
    std::vector<Scene *> checkerboards_;
    std::vector<Scene *> walls_;

    ShaderProgram *current_shader = nullptr;
    Scene *current_obj_ = nullptr;

    TextureManager linear_mipmap_textures_ = TextureManager(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    TextureManager nearest_mipmap_textures_ = TextureManager(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST, true);
    TextureManager linear_textures_ = TextureManager(GL_LINEAR, GL_LINEAR, false);
    TextureManager nearest_textures_ = TextureManager(GL_NEAREST, GL_NEAREST, false);

    bool InitializeObject(const std::string &path,
                          std::vector<Scene *> &objects,
                          std::vector<TextureManager *> &texture_managers) {
        for (auto manager : texture_managers) {
            Scene *obj;
            if (!Scene::CreateFromFile(path.c_str(), obj, *manager)) {
                std::cerr << "FATAL: Failed to load scene from file: " << path << std::endl;
                return false;
            }
            obj->Initialize();
            objects.push_back(obj);
        }
    }

    bool Initialize(const std::string &window_title, bool env_map) override {
        if (!Program::Initialize(window_title, true)) {
            return false;
        }

        shaders_.push_back(new ShaderProgram("shaders/phong.vert", "shaders/no-light.frag"));
        shaders_.push_back(new ShaderProgram("shaders/phong.vert", "shaders/blinn-phong.frag"));

        std::vector<TextureManager *> texture_managers{
            &linear_mipmap_textures_,
            &nearest_mipmap_textures_,
            &linear_textures_,
            &nearest_textures_,
        };

        InitializeObject("resources/models/checkerboard/scene.gltf", checkerboards_, texture_managers);
        for (auto checkerboard : checkerboards_) {
            checkerboard->SetPosition(0.0f, -5.0f, 0.0f);
        }
        InitializeObject("resources/models/brick_wall/scene.gltf", walls_, texture_managers);

        current_obj_ = checkerboards_[2];
        current_shader = shaders_[0];

        return true;
    }

    void DrawImGui() override {
        Program::DrawImGui();

        ImGui::Begin("Mipmap: Settings");

        ImGui::TreeNode("Rendering");
        if (ImGui::Button("Shader: No-Light")) {
            current_shader = shaders_[0];
        }
        if (ImGui::Button("Shader: Blinn-Phong")) {
            current_shader = shaders_[1];
        }

        ImGui::TreeNode("Checkerboard");
        if (ImGui::RadioButton("CB: Linear with Mipmap", current_obj_ == checkerboards_[0])) {
            current_obj_ = checkerboards_[0];
        }
        if (ImGui::RadioButton("CB: Nearest with Mipmap", current_obj_ == checkerboards_[1])) {
            current_obj_ = checkerboards_[1];
        }
        if (ImGui::RadioButton("CB: Linear", current_obj_ == checkerboards_[2])) {
            current_obj_ = checkerboards_[2];
        }
        if (ImGui::RadioButton("CB: Nearest", current_obj_ == checkerboards_[3])) {
            current_obj_ = checkerboards_[3];
        }

        ImGui::TreeNode("Brick Wall");
        if (ImGui::RadioButton("Wall: Linear with Mipmap", current_obj_ == walls_[0])) {
            current_obj_ = walls_[0];
        }
        if (ImGui::RadioButton("Wall: Nearest with Mipmap", current_obj_ == walls_[1])) {
            current_obj_ = walls_[1];
        }
        if (ImGui::RadioButton("Wall: Linear", current_obj_ == walls_[2])) {
            current_obj_ = walls_[2];
        }
        if (ImGui::RadioButton("Wall: Nearest", current_obj_ == walls_[3])) {
            current_obj_ = walls_[3];
        }

        ImGui::End();
    }
};

int main() {
    auto window_title = std::string("CS7GV3: Mipmaps");
    MipMapProgram program;
    if (program.Initialize(window_title)) {
        program.Run();
        return 0;
    } else {
        return 1;
    }
}