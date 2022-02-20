#include "glm/gtx/rotate_vector.hpp"
#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "lib/cameras/camera_tp.h"
#include "lib/program.h"

class NormalMapProgram : public Program {
public:
    NormalMapProgram() {
        delete (FirstPersonCamera *)camera_;
        camera_ = new ThirdPersonCamera(0.0f, 0.0f, 5.0f, 0.0f, 270.0f);
        SetLightCount(1);
    }

    bool Initialize(const std::string &window_title) { return Initialize(window_title, true); }

protected:
    void Draw() override {
        Program::Draw();

        {
            auto current_time = glfwGetTime();
            auto delta_time = current_time - last_frame_time;
            last_frame_time = current_time;

            light_position_ = glm::rotateZ(light_position_, float(delta_time * 1.0f));
            SetLight(light_position_, glm::vec3(0, 0, 0));

            current_shader->Use();
            fresnel_obj_->Draw(current_shader);
        }

        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            {
                ImGui::Begin("Normal Map: Settings");
                if (ImGui::Button("Shader: Cook-Torrance")) {
                    current_shader = shaders_[0];
                }
                if (ImGui::Button("Shader: Blinn-Phong")) {
                    current_shader = shaders_[1];
                }
                if (ImGui::Checkbox("Debug: Normals", &debug_normals_)) {
                    current_shader->SetInt("debugNormals", debug_normals_);
                }
                ImGui::End();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }

private:
    Scene *fresnel_obj_ = nullptr;

    ShaderProgram *current_shader = nullptr;

    glm::vec3 light_position_ = glm::vec3(2.0f);

    bool debug_normals_ = false;
    double last_frame_time = 0;

    bool Initialize(const std::string &window_title, bool env_map) override {
        if (!Program::Initialize(window_title, true)) {
            return false;
        }

        shaders_.push_back(new ShaderProgram("shaders/phong.vert", "shaders/cook-torrance.frag"));
        shaders_.push_back(new ShaderProgram("shaders/phong.vert", "shaders/blinn-phong.frag"));
        current_shader = shaders_[0];

        if (!Scene::CreateFromFile("resources/models/brickwall/scene.gltf", fresnel_obj_, texture_manager_)) {
            std::cout << "FATAL: Failed to load scene" << std::endl;
            return false;
        }
        fresnel_obj_->Initialize();
        fresnel_obj_->GetRootNode()->Scale(1.0f);
        fresnel_obj_->GetRootNode()->Translate(0.0f, -5.0f, 0.0f);

        last_frame_time = glfwGetTime();

        return true;
    }
};

int main() {
    auto window_title = std::string("CS7GV3: Normal Map");
    NormalMapProgram program;
    if (program.Initialize(window_title)) {
        program.Run();
        return 0;
    } else {
        return 1;
    }
}