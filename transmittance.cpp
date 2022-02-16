#include "lib/cameras/camera_tp.h"
#include "lib/program.h"

class TransmittanceProgram : public Program {
public:
    TransmittanceProgram() {
        delete (FirstPersonCamera *)camera_;
        camera_ = new ThirdPersonCamera(0.0f, 5.0f, 20.0f, 0.0f, 270.0f);
        SetLight(glm::vec3(0.0f, 25.0f, 50.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        SetLightCount(1);
    }

    bool Initialize(const std::string *window_title) override {
        if (!Program::Initialize(window_title)) {
            return false;
        }

        shaders_.push_back(new ShaderProgram("shaders/fresnel.vert", "shaders/fresnel.frag"));

        if (!Scene::CreateFromFile("resources/models/teapot/scene.gltf", scene_, texture_manager_)) {
            std::cout << "FATAL: Failed to load scene" << std::endl;
            return false;
        }
        scene_->Initialize();
        scene_->GetRootNode()->Scale(1.0f);

        std::map<GLenum, std::string> env_map_files;
        env_map_files.emplace(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "posx.jpeg");
        env_map_files.emplace(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "negx.jpeg");
        env_map_files.emplace(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "posy.jpeg");
        env_map_files.emplace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "../checkerboard.png");
        env_map_files.emplace(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "posz.jpeg");
        env_map_files.emplace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "negz.jpeg");

        scene_->GetRootNode()->LoadEnvMap(
            "env_map", env_map_files, std::filesystem::path("resources/textures/skybox"), texture_manager_);

        last_frame_time = glfwGetTime();

        return true;
    }

protected:
    void Draw() override {
        Program::Draw();

        auto current_time = glfwGetTime();
        auto delta_time = current_time - last_frame_time;
        last_frame_time = current_time;

        auto shader = shaders_[0];

        shader->Use();

        shader->SetFloat("fresnelEtaR", 0.60);
        shader->SetFloat("fresnelEtaG", 0.65);
        shader->SetFloat("fresnelEtaB", 0.70);
        shader->SetFloat("fresnelBias", 0.1);
        shader->SetFloat("fresnelPower", 5.0);
        shader->SetFloat("fresnelScale", 1.0);

        scene_->GetRootNode()->Rotate(0, 0, delta_time * 0.5);
        scene_->Draw(shader);
    }

private:
    Scene *scene_ = nullptr;

    double last_frame_time = 0;
};

int main() {
    auto window_title = std::string("CS7GV3: Transmittance");
    TransmittanceProgram program;
    if (program.Initialize(&window_title)) {
        program.Run();
        return 0;
    } else {
        return -1;
    }
}