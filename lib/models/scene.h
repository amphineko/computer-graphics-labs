#ifndef MODELS_SCENE_H_
#define MODELS_SCENE_H_

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "node.h"

class Scene : public Node {
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
        model->GetSceneSummary(mesh_count, node_count);
        std::cout << "DEBUG: Loaded model with " << mesh_count << " meshes and " << node_count << " nodes."
                  << std::endl;

        return true;
    }

    static bool LoadDeltaFromFile(const char *file_path, Scene *model) {
        auto base_path = std::filesystem::path(file_path).parent_path();

        Assimp::Importer importer;
        auto scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "FATAL: Failed to load mesh from file: " << file_path << std::endl;
            std::cerr << "FATAL: Importer reported an error: " << importer.GetErrorString() << std::endl;
            return false;
        }

        model->LoadDeltaNodes(scene->mRootNode, scene);

        return true;
    }

    void Draw(ShaderProgram *shader_program) { Node::Draw(0, shader_program); }

    void GetSceneSummary(unsigned int &mesh_count, unsigned int &node_count) {
        children_[0]->GetNodeSummary(mesh_count, node_count);
    }

    Node *GetRootNode() { return children_[0]; }

    void LoadDeltaNodes(const aiNode *node, const aiScene *scene) { children_[0]->LoadDeltaNodes(node, scene); }

private:
    explicit Scene(const aiScene *scene, const std::filesystem::path &base_path, TextureManager &texture_manager) {
        children_.push_back(new Node(scene->mRootNode, scene, base_path, texture_manager, this));
    }
};

#endif // MODELS_SCENE_H_
