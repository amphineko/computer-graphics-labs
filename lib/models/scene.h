#ifndef MODELS_SCENE_H_
#define MODELS_SCENE_H_

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "node.h"

class Scene {
public:
    static bool CreateFromFile(const char *file_path, Scene *&model, TextureManager &texture_manager) {
        auto base_path = std::filesystem::path(file_path).parent_path();

        Assimp::Importer importer;
        auto scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "FATAL: Failed to load mesh from file: " << file_path << std::endl;
            std::cerr << "FATAL: Importer reported an error: " << importer.GetErrorString() << std::endl;
            return false;
        }

        model = new Scene(scene, base_path, texture_manager);

        unsigned int node_count = 0, mesh_count = 0;
        model->GetModelSummary(mesh_count, node_count);
        std::cout << "DEBUG: Loaded model with " << mesh_count << " meshes and " << node_count << " nodes."
                  << std::endl;

        return true;
    }

    void Draw(ShaderProgram *shader_program) { root_node_->Draw(model_matrix_, 0, shader_program); }

    [[nodiscard]] float GetPitch() const { return pitch_; }

    [[nodiscard]] float GetYaw() const { return yaw_; }

    [[nodiscard]] float GetRoll() const { return roll_; }

    void GetModelSummary(unsigned int &mesh_count, unsigned int &node_count) {
        root_node_->GetNodeSummary(mesh_count, node_count);
    }

    Node *GetRootNode() { return root_node_; }

    void Initialize() { root_node_->Initialize(); }

    void SetPosition(const glm::vec3 position) {
        position_ = position;
        UpdateModelMatrix();
    }

    void SetPosition(float x, float y, float z) { SetPosition(glm::vec3(x, y, z)); }

    void SetRotation(float pitch, float yaw, float roll) {
        pitch_ = pitch;
        yaw_ = yaw;
        roll_ = roll;
        UpdateModelMatrix();
    }

    void SetScale(float scale) { scale_ = glm::vec3(scale); }

private:
    Node *root_node_;

    glm::vec3 position_ = glm::vec3(0.0f);
    glm::vec3 scale_ = glm::vec3(1.0f);
    float pitch_ = 0, yaw_ = 0, roll_ = 0;

    glm::mat4 model_matrix_ = glm::mat4(1.0f);

    explicit Scene(const aiScene *scene, const std::filesystem::path base_path, TextureManager &texture_manager) {
        root_node_ = new Node(scene->mRootNode, scene, base_path, texture_manager);
        UpdateModelMatrix();
    }

    void UpdateModelMatrix() {
        auto rotation = glm::yawPitchRoll(glm::radians(yaw_), glm::radians(pitch_), glm::radians(roll_));
        model_matrix_ = glm::scale(glm::translate(glm::mat4(1.0f), position_), scale_) * rotation;
    }
};

#endif // MODELS_SCENE_H_
