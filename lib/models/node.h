#ifndef MODELING_NODE_H_
#define MODELING_NODE_H_

#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/transform.hpp"

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
    Node(const aiNode *node,
         const aiScene *scene,
         const std::filesystem::path &base_path,
         TextureManager &manager,
         const Node *parent) {

        local_transform_ = aiMatrix4x4ToGlm(&node->mTransformation);
        if (parent) {
            parent_ = parent;
            world_transform_ = parent->GetWorldTransform() * local_transform_;
        }

        name_ = std::string(node->mName.C_Str());

        glm::quat rotation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(local_transform_, scale_, rotation, translation_, skew, perspective);

        rotation_ = glm::eulerAngles(rotation);

        if (skew[0] != 0.0f || skew[1] != 0.0f || skew[2] != 0.0f) {
            std::cout << "WARNING: Model node with skew is not supported" << std::endl;
        }

        if (perspective[0] != 0.0f || perspective[1] != 0.0f || perspective[2] != 0.0f || perspective[3] != 1.0f) {
            std::cout << "WARNING: Model node with perspective is not supported" << std::endl;
        }

        for (auto i = 0; i < node->mNumMeshes; i++) {
            meshes_.push_back(new Mesh(scene->mMeshes[node->mMeshes[i]], scene, base_path, manager));
        }

        for (auto i = 0; i < node->mNumChildren; i++) {
            children_.push_back(new Node(node->mChildren[i], scene, base_path, manager, this));
        }
    }

    /**
     * @param parent_textures - texture units used by parent nodes
     */
    void Draw(GLuint parent_texture_unit, ShaderProgram *shader) {
        shader->SetMat4("modelMatrix", world_transform_);
        shader->SetMat4("modelNormalMatrix", glm::transpose(glm::inverse(world_transform_)));

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

        glCheckError();

        for (auto &child : children_) {
            child->Draw(texture_unit, shader);
        }
    }

    bool FindByName(const std::string &name, Node *&node) {
        if (name == name_) {
            node = this;
            return true;
        }

        for (auto &child : children_) {
            if (child->FindByName(name, node)) {
                return true;
            }
        }

        return false;
    }

    void GetNodeSummary(unsigned int &mesh_count, unsigned &node_count) {
        mesh_count += meshes_.size();
        node_count += 1;

        for (auto &child : children_) {
            child->GetNodeSummary(mesh_count, node_count);
        }
    }

    glm::vec3 GetRotation() { return rotation_; }

    [[nodiscard]] glm::vec3 GetWorldPosition() const { return world_transform_[3]; }

    [[nodiscard]] glm::mat4 GetWorldTransform() const { return world_transform_; }

    void Initialize() {
        for (auto &mesh : meshes_) {
            mesh->Initialize();
        }

        for (auto &child : children_) {
            child->Initialize();
        }
    }

    /**
     * @deprecated Environment map is now rendered in real-time
     */
    [[maybe_unused]] void LoadEnvMap(const std::string &name,
                                     const std::map<GLenum, std::string> &paths,
                                     const std::filesystem::path &base_path,
                                     TextureManager &manager) {
        env_map_ = manager.LoadCubeMap(NODE_TEXTURE_ROLE_ENV_MAP, name, paths, base_path);
    }

    virtual void Pick(MeshVertexPickResult &result, ShaderProgram *shader) const {
        shader->SetMat4("modelMatrix", world_transform_);

        for (auto &mesh : meshes_) {
            mesh->Pick(result, world_transform_);
        }

        for (auto &child : children_) {
            child->Pick(result, shader);
        }

        glCheckError();
    }

    void Rotate(float delta_pitch, float delta_yaw, float delta_roll) {
        rotation_ += glm::vec3(delta_pitch, delta_yaw, delta_roll);
        UpdateTransformMatrix();
    }

    void Scale(float delta_scale) {
        scale_ *= delta_scale;
        UpdateTransformMatrix();
    }

    void SetEnvMap(NodeTexture env_map) { env_map_ = env_map; }

    void SetPosition(float x, float y, float z) {
        translation_ = glm::vec3(x, y, z);
        UpdateTransformMatrix();
    }

    void SetRotation(const glm::vec3 &value) {
        rotation_ = value;
        UpdateTransformMatrix();
    }

    void Translate(float delta_x, float delta_y, float delta_z) {
        translation_ += glm::vec3(delta_x, delta_y, delta_z);
        UpdateTransformMatrix();
    }

    void UpdateTransformMatrix() {
        local_transform_ = glm::mat4(1.0f);
        local_transform_ *= glm::translate(translation_);
        local_transform_ *= glm::eulerAngleXYZ(rotation_.x, rotation_.y, rotation_.z);
        local_transform_ *= glm::scale(scale_);

        world_transform_ = parent_ ? parent_->GetWorldTransform() * local_transform_ : local_transform_;

        for (auto &child : children_) {
            child->UpdateTransformMatrix();
        }
    }

protected:
    std::vector<Node *> children_;

    glm::mat4 world_transform_{1.0f};

    Node() {}

private:
    std::vector<Mesh *> meshes_;

    const Node *parent_ = nullptr;
    std::string name_;

    NodeTexture env_map_{.role = 0};

    glm::vec3 scale_{1.0f};
    glm::vec3 rotation_{0.0f};
    glm::vec3 translation_{0.0f};

    glm::mat4 local_transform_{1.0f};
};

#endif // MODELING_NODE_H_
