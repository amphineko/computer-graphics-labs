#ifndef MODELING_NODE_H_
#define MODELING_NODE_H_

#include "glm/gtx/euler_angles.hpp"

#include "../shaders/shader.h"
#include "mesh.h"

inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4 *from) {
    glm::mat4 to;

    to[0][0] = (GLfloat)from->a1;
    to[0][1] = (GLfloat)from->b1;
    to[0][2] = (GLfloat)from->c1;
    to[0][3] = (GLfloat)from->d1;

    to[1][0] = (GLfloat)from->a2;
    to[1][1] = (GLfloat)from->b2;
    to[1][2] = (GLfloat)from->c2;
    to[1][3] = (GLfloat)from->d2;

    to[2][0] = (GLfloat)from->a3;
    to[2][1] = (GLfloat)from->b3;
    to[2][2] = (GLfloat)from->c3;
    to[2][3] = (GLfloat)from->d3;

    to[3][0] = (GLfloat)from->a4;
    to[3][1] = (GLfloat)from->b4;
    to[3][2] = (GLfloat)from->c4;
    to[3][3] = (GLfloat)from->d4;

    return to;
}

class Node {
public:
    Node(const aiNode *node, const aiScene *scene, const std::filesystem::path &base_path, TextureManager &manager) {
        transform_ = aiMatrix4x4ToGlm(&node->mTransformation);

        for (auto i = 0; i < node->mNumMeshes; i++) {
            meshes_.push_back(new Mesh(scene->mMeshes[node->mMeshes[i]], scene, base_path, manager));
        }

        for (auto i = 0; i < node->mNumChildren; i++) {
            children_.push_back(new Node(node->mChildren[i], scene, base_path, manager));
        }
    }

    /**
     * @param parent_textures - texture units used by parent nodes
     */
    void Draw(const glm::mat4 parent_transform, GLuint parent_texture_unit, ShaderProgram *shader) {
        auto local_transform = parent_transform * transform_;
        shader->SetMat4("modelMatrix", local_transform);
        shader->SetMat4("modelNormalMatrix", glm::transpose(glm::inverse(local_transform)));

        auto texture_unit = parent_texture_unit;
        if (env_map_.role == NODE_TEXTURE_ROLE_ENV_MAP) {
            glActiveTexture(GL_TEXTURE0 + texture_unit);
            glBindTexture(GL_TEXTURE_CUBE_MAP, env_map_.name);
            shader->SetInt("envMap", GLint(texture_unit));
            ++texture_unit;
        }

        for (auto &mesh : meshes_) {
            mesh->Draw(texture_unit, shader);
        }

        for (auto &child : children_) {
            // TODO: should drop bound texture in this node?
            child->Draw(local_transform, texture_unit, shader);
        }
    }

    void GetNodeSummary(unsigned int &mesh_count, unsigned &node_count) {
        mesh_count += meshes_.size();
        node_count += 1;

        for (auto &child : children_) {
            child->GetNodeSummary(mesh_count, node_count);
        }
    }

    void Initialize() {
        for (auto &mesh : meshes_) {
            mesh->Initialize();
        }

        for (auto &child : children_) {
            child->Initialize();
        }
    }

    void LoadEnvMap(const std::string &name,
                    const std::map<GLenum, std::string> &paths,
                    const std::filesystem::path &base_path,
                    TextureManager &manager) {
        env_map_ = manager.LoadCubeMap(NODE_TEXTURE_ROLE_ENV_MAP, name, paths, base_path);
    }

    void Rotate(float delta_pitch, float delta_yaw, float delta_roll) {
        transform_ = transform_ * glm::yawPitchRoll(delta_yaw, delta_pitch, delta_roll);
    }

    void Scale(float delta_scale) { transform_ = glm::scale(transform_, glm::vec3(delta_scale)); }

    void SetEnvMap(NodeTexture env_map) { env_map_ = env_map; }

    void Translate(float delta_x, float delta_y, float delta_z) {
        transform_ = glm::translate(transform_, glm::vec3(delta_x, delta_y, delta_z));
    }

private:
    std::vector<Node *> children_;
    std::vector<Mesh *> meshes_;

    NodeTexture env_map_{.role = 0};

    glm::mat4 transform_ = glm::mat4(1.0f);
};

#endif // MODELING_NODE_H_
