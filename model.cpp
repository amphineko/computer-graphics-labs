#include "model.h"

Mesh::Mesh(aiMesh *mesh, const aiScene *scene) {
    vertices_.reserve(mesh->mNumVertices);
    for (GLuint i = 0; i < mesh->mNumVertices; i++) {
        auto tex_coord = mesh->mTextureCoords[0]
                         ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
                         : glm::vec2(0.0f, 0.0f);
        vertices_.emplace_back(
            glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
            glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z),
            tex_coord);
    }

    for (GLuint i = 0; i < mesh->mNumFaces; i++) {
        for (GLuint j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
            indices_.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }

    // TODO: load materials
}

void Mesh::Draw(ShaderProgram *shader) {
    // TODO: configure textures for shader

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, GLsizei(indices_.size()), GL_UNSIGNED_INT, (const void *) 0);
    glBindVertexArray(0);
}

void Mesh::Initialize() {
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // vertex buffer
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (vertices_.size() * sizeof(MeshVertex)), &vertices_[0], GL_STATIC_DRAW);

    // bind position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *) (offsetof(MeshVertex, position)));
    glEnableVertexAttribArray(0);

    // bind normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *) (offsetof(MeshVertex, normal)));
    glEnableVertexAttribArray(1);

    // bind texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *) (offsetof(MeshVertex, tex_coord)));
    glEnableVertexAttribArray(2);

    // element buffer
    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr) (indices_.size() * sizeof(GLuint)),
                 &indices_[0],
                 GL_STATIC_DRAW);
}

Model::Model() {
    UpdateModelMatrix();
}

void Model::Draw(ShaderProgram *shader) {
    shader->SetMat4("model", model_matrix_);
    shader->SetMat3("model_normal", glm::mat3(glm::transpose(glm::inverse(model_matrix_))));
    for (auto &mesh: meshes_) {
        mesh.Draw(shader);
    }
}

void Model::Initialize() {
    for (auto &mesh: meshes_) {
        mesh.Initialize();
    }
    std::cout << "INFO: Model initialized" << std::endl;
}

void Model::LoadNode(aiNode *node, const aiScene *scene) {
    meshes_.reserve(meshes_.size() + node->mNumMeshes);
    for (GLuint i = 0; i < node->mNumMeshes; i++) {
        meshes_.emplace_back(scene->mMeshes[node->mMeshes[i]], scene);
    }

    for (GLuint i = 0; i < node->mNumChildren; i++) {
        LoadNode(node->mChildren[i], scene);
    }
}

void Model::LoadScene(const aiScene *scene) {
    LoadNode(scene->mRootNode, scene);
}

bool Model::LoadSceneFromFile(const std::string &file_path) {
    Assimp::Importer importer;
    auto scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "FATAL: Failed to load mesh from file: " << file_path << std::endl;
        std::cerr << "FATAL: Importer reported an error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    LoadScene(scene);
    return true;
}

void Model::SetPosition(float x, float y, float z) {
    position_ = glm::vec3(x, y, z);
    UpdateModelMatrix();
}

void Model::SetRotation(float pitch, float roll, float yaw) {
    pitch_ = pitch;
    roll_ = roll;
    yaw_ = yaw;
    UpdateModelMatrix();
}

void Model::SetScale(float scale) {
    scale_ = glm::vec3(scale, scale, scale);
    UpdateModelMatrix();
}

void Model::UpdateModelMatrix() {
    model_matrix_ = glm::mat4(1.0f);
    model_matrix_ = glm::translate(model_matrix_, position_);
    model_matrix_ = glm::scale(model_matrix_, scale_);
    model_matrix_ = glm::rotate(model_matrix_, glm::radians(pitch_), glm::vec3(1.0f, 0.0f, 0.0f));
    model_matrix_ = glm::rotate(model_matrix_, glm::radians(roll_), glm::vec3(0.0f, 0.0f, 1.0f));
    model_matrix_ = glm::rotate(model_matrix_, glm::radians(yaw_), glm::vec3(0.0f, 1.0f, 0.0f));
}
