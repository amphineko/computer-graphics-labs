#ifndef MESH_H_
#define MESH_H_

#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "shader.h"

struct MeshVertex {
    [[maybe_unused]] glm::vec3 position;
    [[maybe_unused]] glm::vec3 normal;
    [[maybe_unused]] glm::vec2 tex_coord;
    [[maybe_unused]] glm::vec3 tangent;
    [[maybe_unused]] glm::vec3 bitangent;

    explicit MeshVertex(glm::vec3 position)
        : position(position),
          normal(glm::vec3(0.0f)),
          tex_coord(glm::vec2(0.0f)),
          tangent(glm::vec3(0.0f)),
          bitangent(glm::vec3(0.0f)) {}

//    MeshVertex(
//        glm::vec3 position, glm::vec3 normal, glm::vec2 tex_coord, glm::vec3 tangent, glm::vec3 bitangent
//    ) : position(position), normal(normal), tex_coord(tex_coord), tangent(tangent), bitangent(bitangent) {}
};

typedef unsigned int MeshTextureType;

const MeshTextureType MESH_TEXTURE_TYPE_DIFFUSE = 0;
const MeshTextureType MESH_TEXTURE_TYPE_SPECULAR = 1;
const MeshTextureType MESH_TEXTURE_TYPE_NORMALS = 2;
const MeshTextureType MESH_TEXTURE_TYPE_HEIGHT = 3;

struct MeshTexture {
    [[maybe_unused]] GLuint id;
    [[maybe_unused]] MeshTextureType type;
};

class Mesh {
public:
    Mesh(aiMesh *mesh, const aiScene *scene);

    void Draw(ShaderProgram *shader);

    void Initialize();

    static GLuint LoadTextureFromFile(const char *path);

private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;

    std::vector<MeshVertex> vertices_;
    std::vector<GLuint> indices_;
    std::vector<MeshTexture> textures_;

    glm::vec3 ambient_ = glm::vec3(0.0f);
    glm::vec3 diffuse_ = glm::vec3(0.0f);
    glm::vec3 specular_ = glm::vec3(0.0f);
    float shininess_ = 0.0f;

    void LoadTexture(aiMaterial *material, aiTextureType type, MeshTextureType texture_type);
};

class Model {
public:
    Model();

    void Draw(ShaderProgram *shader);

    void Initialize();

    bool LoadSceneFromFile(const std::string &file_path);

    [[nodiscard]] glm::vec3 GetPosition() const { return position_; }

    [[nodiscard]] float GetRoll() const { return roll_; }

    [[nodiscard]] float GetYaw() const { return yaw_; }

    void SetPosition(float x, float y, float z);

    void SetRotation(float pitch, float roll, float yaw);

    void SetScale(float scale);

private:
    std::vector<Mesh> meshes_;

    glm::vec3 position_ = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 scale_ = glm::vec3(1.0f);
    float pitch_ = 0, roll_ = 0, yaw_ = 0;
    glm::mat4 model_matrix_ = glm::mat4(1.0f);

    void LoadNode(aiNode *node, const aiScene *scene);

    void LoadScene(const aiScene *scene);

    void UpdateModelMatrix();
};

#endif //MESH_H_
