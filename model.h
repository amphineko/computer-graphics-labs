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

    MeshVertex(
        glm::vec3 position, glm::vec3 normal, glm::vec2 tex_coord
    ) : position(position), normal(normal), tex_coord(tex_coord) {}
};

struct MeshTexture {
    [[maybe_unused]] GLuint id;
    [[maybe_unused]] std::string type;
};

class Mesh {
public:
    Mesh(aiMesh *mesh, const aiScene *scene);

    void Draw(ShaderProgram *shader);

    void Initialize();
private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;

    std::vector<MeshVertex> vertices_;
    std::vector<GLuint> indices_;
    std::vector<MeshTexture> textures_;
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
