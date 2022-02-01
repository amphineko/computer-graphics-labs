#include <stbi/stb_image.h>

#include "model.h"

#define IS_NAN(x) (std::fpclassify(x) == FP_NAN)

Mesh::Mesh(aiMesh *mesh, const aiScene *scene) {
    vertices_.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        vertices_.emplace_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
    }

    if (mesh->mNormals) {
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            if (IS_NAN(mesh->mNormals[i].x))
                continue;
            vertices_[i].normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }
    }

    if (mesh->mTextureCoords[0]) {
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            if (IS_NAN(mesh->mTextureCoords[0][i].x))
                continue;
            vertices_[i].tex_coord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
    }

    if (mesh->mTangents) {
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            if (IS_NAN(mesh->mTangents[i].x))
                continue;
            vertices_[i].tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        }
    }

    if (mesh->mBitangents) {
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            if (IS_NAN(mesh->mBitangents[i].x))
                continue;
            vertices_[i].bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        }
    }

    unsigned int totalIndices = 0;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        totalIndices += mesh->mFaces[i].mNumIndices;
    }
    indices_.reserve(indices_.size() + totalIndices);
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        for (int j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
            indices_.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }

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

    LoadTexture(material, aiTextureType_DIFFUSE, MESH_TEXTURE_TYPE_DIFFUSE);
    LoadTexture(material, aiTextureType_SPECULAR, MESH_TEXTURE_TYPE_SPECULAR);
    LoadTexture(material, aiTextureType_NORMALS, MESH_TEXTURE_TYPE_NORMALS);
    LoadTexture(material, aiTextureType_HEIGHT, MESH_TEXTURE_TYPE_HEIGHT);

    if (textures_.empty()) {
        MeshTexture texture{.id = LoadTextureFromFile("models/missing_texture.png"), .type = MESH_TEXTURE_TYPE_DIFFUSE};
        textures_.emplace_back(texture);

        std::cout << "DEBUG: Mesh has no textures, using default texture" << std::endl;
    } else {
        std::cout << "DEBUG: Mesh has " << textures_.size() << " textures" << std::endl;
    }
}

void Mesh::Draw(ShaderProgram *shader) {
    // TODO: configure textures for shader

    shader->SetVec3("mesh_ambient", ambient_);
    shader->SetVec3("mesh_diffuse", diffuse_);
    shader->SetVec3("mesh_specular", specular_);
    shader->SetFloat("mesh_shininess", shininess_);

    auto n_diffuse = 0, n_specular = 0, n_normal = 0, n_height = 0, i = 0;
    for (auto texture : textures_) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, texture.id);

        std::string name;
        switch (texture.type) {
        case MESH_TEXTURE_TYPE_DIFFUSE:
            name = "texture_diffuse" + std::to_string(n_diffuse++);
            break;
        case MESH_TEXTURE_TYPE_SPECULAR:
            name = "texture_specular" + std::to_string(n_specular++);
            break;
        case MESH_TEXTURE_TYPE_NORMALS:
            name = "texture_normal" + std::to_string(n_normal++);
            break;
        case MESH_TEXTURE_TYPE_HEIGHT:
            name = "texture_height" + std::to_string(n_height++);
            break;
        }
        shader->SetInt(name.c_str(), i);

        ++i;
    }

    shader->SetInt("n_texture_diffuse", n_diffuse);
    shader->SetInt("n_texture_specular", n_specular);
    shader->SetInt("n_texture_normal", n_normal);
    shader->SetInt("n_texture_height", n_height);

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, GLsizei(indices_.size()), GL_UNSIGNED_INT, (const void *)0);
    glBindVertexArray(0);
}

void Mesh::Initialize() {
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // vertex buffer
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vertices_.size() * sizeof(MeshVertex)), &vertices_[0], GL_STATIC_DRAW);

    // bind position_ attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *)(offsetof(MeshVertex, position)));
    glEnableVertexAttribArray(0);

    // bind normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *)(offsetof(MeshVertex, normal)));
    glEnableVertexAttribArray(1);

    // bind texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *)(offsetof(MeshVertex, tex_coord)));
    glEnableVertexAttribArray(2);

    // bind tangent attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *)(offsetof(MeshVertex, tangent)));
    glEnableVertexAttribArray(3);

    // bind bitangent attribute
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (GLvoid *)(offsetof(MeshVertex, bitangent)));
    glEnableVertexAttribArray(4);

    // element buffer
    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(indices_.size() * sizeof(GLuint)), &indices_[0], GL_STATIC_DRAW);
}

void Mesh::LoadTexture(aiMaterial *material, aiTextureType type, MeshTextureType texture_type) {
    for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
        aiString path;
        material->GetTexture(type, i, &path);

        auto texture_id = LoadTextureFromFile(path.C_Str());
        MeshTexture texture{
            .id = texture_id,
            .type = texture_type,
        };
        textures_.emplace_back(texture);
    }
}

GLuint Mesh::LoadTextureFromFile(const char *path) {
    GLuint texture_id;
    glGenTextures(1, &texture_id);

    int width, height, nr_channels;
    unsigned char *image = stbi_load(path, &width, &height, &nr_channels, 0);

    if (!image) {
        std::cerr << "ERROR: Failed to open texture file: " << path << std::endl;
        return texture_id;
    }

    GLint format;
    switch (nr_channels) {
    case STBI_grey:
        format = GL_RED;
        break;
    case STBI_rgb:
        format = GL_RGB;
        break;
    case STBI_rgb_alpha:
        format = GL_RGBA;
        break;
    default:
        std::cerr << "ERROR: Unsupported texture format: " << path << std::endl;
        stbi_image_free(image);
        return texture_id;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(image);
    return texture_id;
}

Model::Model() { UpdateModelMatrix(); }

void Model::Draw(ShaderProgram *shader) {
    shader->SetMat4("model", model_matrix_);
    shader->SetMat4("model_normal", glm::transpose(glm::inverse(model_matrix_)));
    for (auto &mesh : meshes_) {
        mesh.Draw(shader);
    }
}

void Model::Initialize() {
    for (auto &mesh : meshes_) {
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
    std::cout << "DEBUG: Loaded model with " << meshes_.size() << " meshes" << std::endl;
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
    auto rotation = glm::yawPitchRoll(glm::radians(yaw_), glm::radians(pitch_), glm::radians(roll_));
    model_matrix_ = glm::scale(glm::translate(glm::mat4(1.0f), position_), scale_) * rotation;
}
