#include "glm/gtx/rotate_vector.hpp"
#include "imgui.h"

#include "lib/cameras/camera_tp.h"
#include "lib/program.h"

class MipMapProgram : public Program {
public:
    MipMapProgram() {
        delete (FirstPersonCamera *)camera_;
        camera_ = new ThirdPersonCamera(0.0f, 0.0f, 2.0f, 0.0f, 270.0f);
        SetLightCount(1);
    }

    bool Initialize(const std::string &window_title) { return Initialize(window_title, true); }

protected:
    void Draw() override {
        Program::Draw();

        light_position_ = glm::rotateZ(light_position_, float(last_frame_time_ * 1.0f));
        SetLight(light_position_, glm::vec3(0, 0, 0));

        current_shader->Use();
        current_obj_->Draw(current_shader);
    }

private:
    Scene *linear_mipmap_obj_ = nullptr;
    Scene *nearest_mipmap_obj_ = nullptr;
    Scene *linear_obj_ = nullptr;
    Scene *nearest_obj_ = nullptr;

    ShaderProgram *current_shader = nullptr;
    Scene *current_obj_ = nullptr;

    glm::vec3 light_position_ = glm::vec3(2.0f);

    float camera_yaw_ = 270.0f;

    bool debug_normals_ = false;

    TextureManager linear_mipmap_textures_ = TextureManager(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    TextureManager nearest_mipmap_textures_ = TextureManager(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST, true);
    TextureManager linear_textures_ = TextureManager(GL_LINEAR, GL_LINEAR, false);
    TextureManager nearest_textures_ = TextureManager(GL_NEAREST, GL_NEAREST, false);

    bool Initialize(const std::string &window_title, bool env_map) override {
        if (!Program::Initialize(window_title, true)) {
            return false;
        }

        shaders_.push_back(new ShaderProgram("shaders/phong.vert", "shaders/cook-torrance.frag"));
        shaders_.push_back(new ShaderProgram("shaders/phong.vert", "shaders/blinn-phong.frag"));

        if (!Scene::CreateFromFile(
                "resources/models/brickwall/scene.gltf", linear_mipmap_obj_, linear_mipmap_textures_)) {
            std::cout << "FATAL: Failed to load scene" << std::endl;
            return false;
        }
        linear_mipmap_obj_->Initialize();
        linear_mipmap_obj_->GetRootNode()->Scale(1.0f);
        linear_mipmap_obj_->GetRootNode()->Translate(0.0f, -5.0f, 0.0f);

        if (!Scene::CreateFromFile(
                "resources/models/brickwall/scene.gltf", nearest_mipmap_obj_, nearest_mipmap_textures_)) {
            std::cout << "FATAL: Failed to load scene" << std::endl;
            return false;
        }
        nearest_mipmap_obj_->Initialize();
        nearest_mipmap_obj_->GetRootNode()->Scale(1.0f);
        nearest_mipmap_obj_->GetRootNode()->Translate(0.0f, -5.0f, 0.0f);

        if (!Scene::CreateFromFile("resources/models/brickwall/scene.gltf", linear_obj_, linear_textures_)) {
            std::cout << "FATAL: Failed to load scene" << std::endl;
            return false;
        }
        linear_obj_->Initialize();
        linear_obj_->GetRootNode()->Scale(1.0f);
        linear_obj_->GetRootNode()->Translate(0.0f, -5.0f, 0.0f);

        if (!Scene::CreateFromFile("resources/models/brickwall/scene.gltf", nearest_obj_, nearest_textures_)) {
            std::cout << "FATAL: Failed to load scene" << std::endl;
            return false;
        }
        nearest_obj_->Initialize();
        nearest_obj_->GetRootNode()->Scale(1.0f);
        nearest_obj_->GetRootNode()->Translate(0.0f, -5.0f, 0.0f);

        current_shader = shaders_[0];
        current_obj_ = linear_mipmap_obj_;

        return true;
    }

    void DrawImGui() override {
        ImGui::Begin("Mipmap: Settings");

        ImGui::TreeNode("Rendering");
        if (ImGui::Button("Shader: Cook-Torrance")) {
            current_shader = shaders_[0];
        }
        if (ImGui::Button("Shader: Blinn-Phong")) {
            current_shader = shaders_[1];
        }

        ImGui::TreeNode("Mipmap");
        if (ImGui::Button("Object: Linear with Mipmap")) {
            current_obj_ = linear_mipmap_obj_;
        }
        if (ImGui::Button("Object: Nearest with Mipmap")) {
            current_obj_ = nearest_mipmap_obj_;
        }
        if (ImGui::Button("Object: Linear")) {
            current_obj_ = linear_obj_;
        }
        if (ImGui::Button("Object: Nearest")) {
            current_obj_ = nearest_obj_;
        }

        if (ImGui::Checkbox("Debug: Normals", &debug_normals_)) {
            current_shader->SetInt("debugNormals", debug_normals_);
        }
        ImGui::End();
    }
};

int main() {
    auto window_title = std::string("CS7GV3: Normal Map");
    MipMapProgram program;
    if (program.Initialize(window_title)) {
        program.Run();
        return 0;
    } else {
        return 1;
    }
}