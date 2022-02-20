#include "glm/gtx/rotate_vector.hpp"
#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "lib/cameras/camera_tp.h"
#include "lib/program.h"
#include "lib/skybox.h"

class NormalMapProgram : public Program {
public:
    NormalMapProgram() {
        delete (FirstPersonCamera *)camera_;
        camera_ = new ThirdPersonCamera(25.0f, 25.0f, -25.0f, -15.0f, 135.0f);
        SetLightCount(1);
    }

    bool Initialize(const std::string &window_title) { return Initialize(window_title, true); }

protected:
    void Draw() override {
        Program::DrawEnvMap(glm::vec3(0.0, obj_center_height_, 0.0)); // TODO: use teapot position

        SetLight(0, glm::vec3(0.0f, 50.0f, 50.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        SetLightCount(1);

        auto current_time = glfwGetTime();
        auto delta_time = current_time - last_frame_time;
        last_frame_time = current_time;

        if (!mouse_hold_) {
            fresnel_obj_->Rotate(0.0f, float(delta_time) * 0.5f, 0.0f);
        }

        Program::Draw();

        skybox_shader_->Use();
        skybox_->Draw(skybox_shader_);

        phong_shader_->Use();
        table_->Draw(phong_shader_);

        fresnel_shader_->Use();
        fresnel_shader_->SetFloat("fresnelEtaR", fresnel_eta_r_);
        fresnel_shader_->SetFloat("fresnelEtaG", fresnel_eta_g_);
        fresnel_shader_->SetFloat("fresnelEtaB", fresnel_eta_b_);
        fresnel_shader_->SetFloat("fresnelBias", fresnel_bias_);
        fresnel_shader_->SetFloat("fresnelPower", fresnel_power_);
        fresnel_shader_->SetFloat("fresnelScale", fresnel_scale_);
        fresnel_obj_->Draw(fresnel_shader_);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("Transmittance: Settings");
            ImGui::TreeNode("Fresnel effects");
            ImGui::SliderFloat("eta-red", &fresnel_eta_r_, 0.0f, 1.0f);
            ImGui::SliderFloat("eta-green", &fresnel_eta_g_, 0.0f, 1.0f);
            ImGui::SliderFloat("eta-blue", &fresnel_eta_b_, 0.0f, 1.0f);
            ImGui::SliderFloat("Bias", &fresnel_bias_, 0.0f, 1.0f);
            ImGui::SliderFloat("Power", &fresnel_power_, 0.0f, 10.0f);
            ImGui::SliderFloat("Scale", &fresnel_scale_, 0.0f, 10.0f);
            ImGui::TreeNode("Dynamic EnvMap");
            ImGui::SliderFloat("Center height", &obj_center_height_, 0.0f, 100.0f);
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glCheckError();
    }

    void DrawEnvMapFace(glm::vec3 position, glm::vec3 direction, glm::vec3 up) override {
        Program::DrawEnvMapFace(position, direction, up);

        skybox_shader_->Use();
        skybox_->Draw(skybox_shader_);

        phong_shader_->Use();
        table_->Draw(phong_shader_);
    }

private:
    Scene *fresnel_obj_ = nullptr;
    Scene *table_ = nullptr;
    Skybox *skybox_ = nullptr;

    ShaderProgram *fresnel_shader_ = nullptr;
    ShaderProgram *phong_shader_ = nullptr;
    ShaderProgram *skybox_shader_ = nullptr;

    glm::vec3 light_position_ = glm::vec3(0.0f, 25.0f, 25.0f);

    float fresnel_eta_r_ = 0.40f;
    float fresnel_eta_g_ = 0.50f;
    float fresnel_eta_b_ = 0.60f;
    float fresnel_bias_ = 0.10f;
    float fresnel_power_ = 5.0f;
    float fresnel_scale_ = 1.0f;

    float obj_center_height_ = 10.0f;

    bool debug_normals_ = false;
    double last_frame_time = 0;

    bool Initialize(const std::string &window_title, bool env_map) override {
        if (!Program::Initialize(window_title, true)) {
            return false;
        }

        fresnel_shader_ = new ShaderProgram("shaders/fresnel.vert", "shaders/fresnel.frag");
        phong_shader_ = new ShaderProgram("shaders/phong.vert", "shaders/cook-torrance.frag");
        skybox_shader_ = new ShaderProgram("shaders/skybox.vert", "shaders/skybox.frag");

        shaders_.push_back(fresnel_shader_);
        shaders_.push_back(phong_shader_);
        shaders_.push_back(skybox_shader_);

        for (auto shader : shaders_) {
            if (!shader->IsReady()) {
                return false;
            }
        }

        // object

        if (!Scene::CreateFromFile("resources/models/teapot/scene.gltf", fresnel_obj_, texture_manager_)) {
            std::cout << "FATAL: Failed to load scene" << std::endl;
            return false;
        }
        fresnel_obj_->Initialize();
        fresnel_obj_->Scale(2.0f);
        fresnel_obj_->Translate(0.0f, 0.0f, 0.0f);
        fresnel_obj_->SetEnvMap(env_map_);

        if (!Scene::CreateFromFile("resources/models/table/scene.gltf", table_, texture_manager_)) {
            std::cout << "FATAL: Failed to load scene" << std::endl;
            return false;
        }
        table_->Initialize();
        table_->Scale(0.5f);
        table_->Translate(0.0f, -5.0f, 0.0f);

        // background

        std::map<GLenum, std::string> skybox_maps{
            {GL_TEXTURE_CUBE_MAP_POSITIVE_X, "posx.jpeg"},
            {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "negx.jpeg"},
            {GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "posy.jpeg"},
            {GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "negy.jpeg"},
            {GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "posz.jpeg"},
            {GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "negz.jpeg"},
        };

        skybox_ = new Skybox();
        skybox_->Initialize(50.0, skybox_maps, "resources/textures/skybox", texture_manager_);

        last_frame_time = glfwGetTime();

        return true;
    }
};

int main() {
    auto window_title = std::string("CS7GV3: Transmittance");
    NormalMapProgram program;
    if (program.Initialize(window_title)) {
        program.Run();
        return 0;
    } else {
        return 1;
    }
}