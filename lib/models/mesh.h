#ifndef MODELS_MESH_H_
#define MODELS_MESH_H_

#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "glm/glm.hpp"

#include <iostream>
#include <vector>

#include "../shaders/shader.h"

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

class Mesh {
public:
    Mesh(const aiMesh *mesh, const aiScene *scene, const std::string &base_path, TextureManager &manager) {
        LoadVertices(mesh);
        LoadMaterials(mesh, scene, base_path, manager);
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
    }

private:
    std::vector<MeshVertex> vertices_;
    std::vector<unsigned int> indices_;
    std::vector<MeshTexture> textures_;

    glm::vec3 ambient_ = glm::vec3(0, 0, 0);
    glm::vec3 diffuse_ = glm::vec3(0, 0, 0);
    glm::vec3 specular_ = glm::vec3(0, 0, 0);
    float shininess_ = 0;

    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;

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
