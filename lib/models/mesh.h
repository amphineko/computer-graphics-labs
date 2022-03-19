#ifndef MODELS_MESH_H_
#define MODELS_MESH_H_

#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "glm/glm.hpp"

#include <iostream>
#include <vector>

#include "../shaders/shader.h"
#include "../shaders/vertex_picker.h"

#include "textures.h"

#define NOT_NAN(x) (std::fpclassify(x) != FP_NAN)

const glm::vec3 kDefaultMeshNormal = glm::vec3(0.0f, 0.0f, 1.0f);
const glm::vec2 kDefaultMeshUV = glm::vec2(0.0f, 0.0f);
const glm::vec3 kDefaultMeshTangent = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 kDefaultMeshBitangent = glm::vec3(0.0f, 1.0f, 0.0f);

struct MeshVertex {
    [[maybe_unused]] glm::vec3 position;
    [[maybe_unused]] glm::vec2 uv;
    [[maybe_unused]] glm::vec3 normal;
    [[maybe_unused]] glm::vec3 tangent;
    [[maybe_unused]] glm::vec3 bitangent;

    explicit MeshVertex(glm::vec3 position)
        : position(position), uv(kDefaultMeshUV), normal(kDefaultMeshNormal), tangent(kDefaultMeshTangent),
          bitangent(kDefaultMeshBitangent) {}
};

class Mesh;

struct MeshVertexPickResult {
    float distance;
    const MeshVertex *vertex;
    glm::vec3 world_position;

    size_t vertex_index;

    bool operator<(const MeshVertexPickResult &other) const { return distance < other.distance; }
};

struct MeshVertexPickResultComparator {
    bool operator()(const MeshVertexPickResult &lhs, const MeshVertexPickResult &rhs) const {
        return lhs.distance < rhs.distance;
    }
};

class Mesh {
public:
    Mesh(const aiMesh *mesh, const aiScene *scene, const std::string &base_path, TextureManager &manager) {
        LoadVertices(mesh);
        LoadMaterials(mesh, scene, base_path, manager);
    }

    ~Mesh() {
        for (auto i : delta_positions_) {
            delete i;
        }
        delete[] tf_out_;
    }

    void Draw(GLuint texture_unit, ShaderProgram *shader) const {
        shader->SetVec3("meshAmbient", ambient_);
        shader->SetVec3("meshDiffuse", diffuse_);
        shader->SetVec3("meshSpecular", specular_);
        shader->SetFloat("meshShininess", shininess_);

        GLint n_diffuse = 0, n_specular = 0, n_normal = 0, n_height = 0, n_unknown = 0;
        for (auto texture : textures_) {
            std::string name;
            switch (texture.role) {
            case MESH_TEXTURE_ROLE_DIFFUSE:
                name = "diffuseMap" + std::to_string(n_diffuse++);
                break;
            case MESH_TEXTURE_ROLE_SPECULAR:
                name = "specularMap" + std::to_string(n_specular++);
                break;
            case MESH_TEXTURE_ROLE_NORMAL:
                name = "normalMap" + std::to_string(n_normal++);
                break;
            case MESH_TEXTURE_ROLE_HEIGHT:
                name = "heightMap" + std::to_string(n_height++);
                break;
            default:
                name = "texture" + std::to_string(n_unknown++);
                std::cerr << "WARN: Unknown texture role " << texture.role << std::endl;
                break;
            }

            glActiveTexture(GL_TEXTURE0 + texture_unit);
            glBindTexture(GL_TEXTURE_2D, texture.name);
            shader->SetInt(name.c_str(), GLint(texture_unit));

            ++texture_unit;
        }

        shader->SetInt("nDiffuseMap", n_diffuse);
        shader->SetInt("nSpecularMap", n_specular);
        shader->SetInt("nNormalMap", n_normal);
        shader->SetInt("nHeightMap", n_height);

        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, GLsizei(indices_.size()), GL_UNSIGNED_INT, (const void *)nullptr);
        glBindVertexArray(0);
    }

    void Initialize() {
        glGenVertexArrays(1, &vao_);
        glBindVertexArray(vao_);

        // vertex buffer
        glGenBuffers(1, &vbo_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(
            GL_ARRAY_BUFFER, (GLsizeiptr)(vertices_.size() * sizeof(MeshVertex)), &vertices_[0], GL_STATIC_DRAW);

        // bind position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *)(offsetof(MeshVertex, position)));
        glEnableVertexAttribArray(0);

        // bind texture coordinate attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *)(offsetof(MeshVertex, uv)));
        glEnableVertexAttribArray(1);

        // bind normal attribute
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *)(offsetof(MeshVertex, normal)));
        glEnableVertexAttribArray(2);

        // bind tangent attribute
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *)(offsetof(MeshVertex, tangent)));
        glEnableVertexAttribArray(3);

        // bind bitangent attribute
        glVertexAttribPointer(
            4, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *)(offsetof(MeshVertex, bitangent)));
        glEnableVertexAttribArray(4);

        // element buffer
        glGenBuffers(1, &ebo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(indices_.size() * sizeof(GLuint)), &indices_[0], GL_STATIC_DRAW);

        // delta-position buffer
        glGenBuffers(1, &vbo_delta_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_delta_);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vertices_.size() * sizeof(glm::vec3)), nullptr, GL_STATIC_DRAW);

        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid *)nullptr);
        glEnableVertexAttribArray(5);

        glCheckError();

        // transform feedback: inputs
        glGenVertexArrays(1, &tf_vao_);
        glBindVertexArray(tf_vao_);

        glGenBuffers(1, &tf_vbo_in_);
        glBindBuffer(GL_ARRAY_BUFFER, tf_vbo_in_);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vertices_.size() * sizeof(MeshVertex)), nullptr, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *)(offsetof(MeshVertex, position)));
        glEnableVertexAttribArray(0);

        glCheckError();

        // transform feedback: outputs
        glGenTransformFeedbacks(1, &tfo_);
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfo_);

        glGenBuffers(1, &tf_vbo_out_);
        glBindBuffer(GL_ARRAY_BUFFER, tf_vbo_out_);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vertices_.size() * sizeof(GLfloat)), nullptr, GL_STATIC_DRAW);

        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tf_vbo_out_);

        glGenQueries(1, &tfq_);

        tf_out_ = new GLfloat[vertices_.size()];

        glCheckError();
    }

    void LoadDeltaMesh(const aiMesh *mesh) {
        delta_positions_.push_back(new std::vector<glm::vec3>());
        delta_weights_.push_back(0.0f);

        auto back = delta_positions_.back();

        back->reserve(mesh->mNumVertices);

        if (vertices_.size() != mesh->mNumVertices) {
            std::cerr << "ERROR: delta mesh has different number of vertices" << std::endl;
        }

        for (size_t i = 0; i < std::min(vertices_.size(), size_t(mesh->mNumVertices)); ++i) {
            back->push_back(vertices_[i].position -
                            glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
        }
    }

    void Pick(MeshVertexPickResult &result, glm::mat4 model) const {
        // draw for feedback

        glEnable(GL_RASTERIZER_DISCARD);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfo_);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tf_vbo_out_);

        glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, tfq_);
        glBeginTransformFeedback(GL_POINTS);

        glBindVertexArray(vao_);
        glDrawArrays(GL_POINTS, 0, GLsizei(vertices_.size()));

        glEndTransformFeedback();
        glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

        glDisable(GL_RASTERIZER_DISCARD);

        glCheckError();

        // query feedback result

        GLint vertices_count = 0;
        glGetQueryObjectiv(tfq_, GL_QUERY_RESULT, &vertices_count);
        if (vertices_count != vertices_.size()) {
            std::cerr << "ERROR: Transform feedback written " << vertices_count << " != " << vertices_.size()
                      << std::endl;
            return;
        }

        glCheckError();

        glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, GLsizeiptr(sizeof(GLfloat) * vertices_count), tf_out_);

        if (glCheckError() == GL_NO_ERROR) {
            for (size_t i = 0; i < vertices_count; ++i) {
                if (tf_out_[i] < result.distance) {
                    result.distance = tf_out_[i];
                    result.vertex = &vertices_[i];
                    result.world_position = glm::vec3(model * glm::vec4(vertices_[i].position, 1.0f));
                }
            }
        }
    }

    void SetDeltaWeight(size_t index, float weight) { delta_weights_[index] = weight; }

    void UpdateDeltaWeights() {
        for (size_t i = 0; i < vertices_.size(); ++i) {
            delta_weighted_[i] = glm::vec3(0.0f);
        }

        for (size_t weight_i = 0; weight_i < delta_weights_.size(); ++weight_i) {
            for (size_t vert_i = 0; vert_i < vertices_.size(); ++vert_i) {
                delta_weighted_[vert_i] += delta_weights_[weight_i] * delta_positions_[weight_i]->at(vert_i);
            }
        }

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_delta_);
        glBufferData(
            GL_ARRAY_BUFFER, (GLsizeiptr)(vertices_.size() * sizeof(glm::vec3)), &delta_weighted_[0], GL_STATIC_DRAW);
    }

    void UpdateVertexPosition(float x, float y, float z, float delta_x, float delta_y, float delta_z, glm::mat4 model) {
        bool updated = false;

        for (size_t i = 0; i < vertices_.size(); ++i) {
            auto dist = glm::distance(glm::vec3(model * glm::vec4(vertices_[i].position, 1.0f)), glm::vec3(x, y, z));
            if (dist < 0.5f) {
                vertices_[i].position += glm::vec3(delta_x, delta_y, delta_z) * (dist / 5.0f);
                updated = true;
            }
        }

        if (updated) {
            glBindVertexArray(vao_);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_);
            glBufferData(
                GL_ARRAY_BUFFER, (GLsizeiptr)(vertices_.size() * sizeof(MeshVertex)), &vertices_[0], GL_STATIC_DRAW);
        }
    }

private:
    std::vector<MeshVertex> vertices_;
    std::vector<unsigned int> indices_;
    std::vector<MeshTexture> textures_;

    std::vector<glm::vec3> delta_weighted_;

    std::vector<std::vector<glm::vec3> *> delta_positions_;
    std::vector<float> delta_weights_;

    glm::vec3 ambient_ = glm::vec3(0, 0, 0);
    glm::vec3 diffuse_ = glm::vec3(0, 0, 0);
    glm::vec3 specular_ = glm::vec3(0, 0, 0);
    float shininess_ = 0;

    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0, vbo_delta_ = 0;

    GLuint tfo_ = 0, tfq_ = 0;                           // transform feedback object, query
    GLuint tf_vao_ = 0, tf_vbo_in_ = 0, tf_vbo_out_ = 0; // input vao, input vbo, output vbo
    GLfloat *tf_out_;                                    // result copy-back buffer

    void LoadMaterials(const aiMesh *mesh,
                       const aiScene *scene,
                       const std::filesystem::path &base_path,
                       TextureManager &manager) {
        auto material = scene->mMaterials[mesh->mMaterialIndex];

        aiColor3D ambient, diffuse, specular, shininess;
        material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
        material->Get(AI_MATKEY_SHININESS, shininess);

        ambient_ = glm::vec3(ambient.r, ambient.g, ambient.b);
        diffuse_ = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
        specular_ = glm::vec3(specular.r, specular.g, specular.b);
        shininess_ = shininess.r;

        LoadTexture(material, aiTextureType_DIFFUSE, MESH_TEXTURE_ROLE_DIFFUSE, base_path, manager);
        LoadTexture(material, aiTextureType_SPECULAR, MESH_TEXTURE_ROLE_SPECULAR, base_path, manager);
        LoadTexture(material, aiTextureType_NORMALS, MESH_TEXTURE_ROLE_NORMAL, base_path, manager);
        LoadTexture(material, aiTextureType_HEIGHT, MESH_TEXTURE_ROLE_HEIGHT, base_path, manager);

        if (textures_.empty()) {
            std::cerr << "WARNING: Mesh has no textures" << std::endl;
        }
    }

    void LoadTexture(const aiMaterial *material,
                     aiTextureType type,
                     MeshTextureRole role,
                     const std::filesystem::path &base_path,
                     TextureManager &manager) {
        // TODO: resolve relative path with model file path

        for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
            aiString aiPath;
            material->GetTexture(type, i, &aiPath);

            auto path = std::string(aiPath.C_Str());
            textures_.emplace_back(manager.LoadTexture2D(role, path, base_path));
        }
    }

    void LoadVertices(const aiMesh *mesh) {
        vertices_.reserve(mesh->mNumVertices); // optimization: pre-allocate memory for insertions

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            vertices_.emplace_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
        }

        if (mesh->mNormals) {
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
                vertices_[i].normal =
                    NOT_NAN(mesh->mNormals[i].x)
                        ? glm::normalize(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z))
                        : kDefaultMeshNormal;
                // TODO: should we normalize the normals?
            }
        }

        if (mesh->mTextureCoords[0]) {
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
                vertices_[i].uv = NOT_NAN(mesh->mTextureCoords[0][i].x)
                                      // NOTE: don't normalize tex coords
                                      ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
                                      : kDefaultMeshUV;
            }
        }

        if (mesh->mTangents) {
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
                vertices_[i].tangent =
                    NOT_NAN(mesh->mTangents[i].x)
                        ? glm::normalize(glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z))
                        : kDefaultMeshTangent;
            }
        }

        if (mesh->mBitangents) {
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
                vertices_[i].bitangent =
                    NOT_NAN(mesh->mBitangents[i].x)
                        ? glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z)
                        : kDefaultMeshBitangent;
            }
        }

        delta_weighted_.reserve(vertices_.size());
        for (unsigned int i = 0; i < vertices_.size(); ++i) {
            delta_weighted_[i] = glm::vec3(0.0f, 0.0f, 0.0f);
        }

        // load face indices

        unsigned int totalIndices = 0;
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            totalIndices += mesh->mFaces[i].mNumIndices; // collect indices count
        }
        indices_.reserve(indices_.size() + totalIndices); // optimization: pre-allocate memory for insertions
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            for (int j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
                indices_.push_back(mesh->mFaces[i].mIndices[j]);
            }
        }
    };
};

#endif // MODELS_MESH_H_
