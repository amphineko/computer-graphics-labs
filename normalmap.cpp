#include "glm/gtx/rotate_vector.hpp"

#include "lib/cameras/camera_tp.h"
#include "lib/program.h"

class TransmittanceProgram : public Program {
public:
    TransmittanceProgram() {
        delete (FirstPersonCamera *)camera_;
        camera_ = new ThirdPersonCamera(0.0f, 0.0f, 7.5f, 0.0f, 270.0f);
        SetLightCount(1);
    }

    bool Initialize(const std::string *window_title) override {
        if (!Program::Initialize(window_title)) {
            return false;
        }

        // shaders_.push_back(new ShaderProgram("shaders/phong.vert", "shaders/blinn-phong.frag"));
        shaders_.push_back(new ShaderProgram("shaders/phong.vert", "shaders/cook-torrance.frag"));

        if (!Scene::CreateFromFile("resources/models/brickwall/scene.gltf", scene_, texture_manager_)) {
            std::cout << "FATAL: Failed to load scene" << std::endl;
            return false;
        }
        scene_->Initialize();
        scene_->GetRootNode()->Scale(1.0f);
        scene_->GetRootNode()->Translate(0.0f, 0.0f, -5.0f);

        last_frame_time = glfwGetTime();

        return true;
    }

protected:
    void Draw() override {
        Program::Draw();

        auto current_time = glfwGetTime();
        auto delta_time = current_time - last_frame_time;
        last_frame_time = current_time;

        light_position_ = glm::rotateZ(light_position_, float(delta_time * 1.0f));
        SetLight(light_position_, glm::vec3(0, 0, 0));

        auto shader = shaders_[0];
        shader->Use();

        scene_->Draw(shader);
    }

private:
    Scene *scene_ = nullptr;

    glm::vec3 light_position_ = glm::vec3(2.0f);

    double last_frame_time = 0;
};

int main() {
    auto window_title = std::string("CS7GV3: Normal Map");
    TransmittanceProgram program;
    if (program.Initialize(&window_title)) {
        program.Run();
        return 0;
    } else {
        return -1;
    }
}