#ifndef MODELS_TEXTURES_H_
#define MODELS_TEXTURES_H_

#include <filesystem>
#include <iostream>
#include <map>
#include <string>

#include "../utils.h"
#include <stbi/stb_image.h>

typedef unsigned int MeshTextureRole;
typedef unsigned int NodeTextureRole;

const MeshTextureRole MESH_TEXTURE_ROLE_DIFFUSE = 1;
const MeshTextureRole MESH_TEXTURE_ROLE_SPECULAR = 2;
const MeshTextureRole MESH_TEXTURE_ROLE_NORMAL = 3;
const MeshTextureRole MESH_TEXTURE_ROLE_HEIGHT = 4;

const NodeTextureRole NODE_TEXTURE_ROLE_CUBE_MAP = 5;
const NodeTextureRole NODE_TEXTURE_ROLE_ENV_MAP = 6;

struct MeshTexture {
    GLuint name;
    MeshTextureRole role;
};

struct NodeTexture {
    GLuint name;
    NodeTextureRole role;
};

class TextureManager {
public:
    TextureManager() : TextureManager(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true) {}

    TextureManager(GLint min_filter, GLint mag_filter, bool mipmap) {
        min_filter_ = min_filter;
        mag_filter_ = mag_filter;
        mipmap_ = mipmap;
    }

    ~TextureManager() {
        for (const auto &texture : mesh_textures_) {
            glDeleteTextures(1, &texture.second.name);
        }
        mesh_textures_.clear();

        for (const auto &texture : node_textures_) {
            glDeleteTextures(1, &texture.second.name);
        }
        node_textures_.clear();
    }

    NodeTexture LoadCubeMap(NodeTextureRole type,
                            const std::string &map_name,
                            const std::map<GLenum, std::string> &filenames,
                            const std::filesystem::path &base_path) {
        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

        for (auto [texture_target, filename] : filenames) {
            LoadTextureImageFromFile(texture_target, base_path / filename);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (min_filter_ != 0) {
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filter_);
        }
        if (mag_filter_ != 0) {
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mag_filter_);
        }

        if (mipmap_) {
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }

        NodeTexture texture{.name = texture_id, .role = type};
        node_textures_[map_name] = texture;

        return texture;
    }

    MeshTexture
    LoadTexture2D(MeshTextureRole role, const std::string &filename, const std::filesystem::path &base_path) {
        if (mesh_textures_.find(filename) != mesh_textures_.end()) {
            return mesh_textures_[filename];
        }

        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        LoadTextureImageFromFile(GL_TEXTURE_2D, base_path / filename);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        if (min_filter_ != 0) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter_);
        }
        if (mag_filter_ != 0) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter_);
        }

        if (mipmap_) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        MeshTexture texture{.name = texture_id, .role = role};
        mesh_textures_[filename] = texture;

        glCheckError();

        return texture;
    }

private:
    std::map<std::string, NodeTexture> node_textures_;
    std::map<std::string, MeshTexture> mesh_textures_;

    GLint min_filter_ = 0, mag_filter_ = 0;
    bool mipmap_ = false;

    static void LoadTextureImageFromFile(GLenum texture_target, const std::filesystem::path &path) {
        int width, height, n_channels;
        stbi_uc *data = stbi_load(path.c_str(), &width, &height, &n_channels, 0);
        if (data) {
            GLenum format;
            switch (n_channels) {
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
                format = 0;
                break;
            }

            if (format != 0) {
                glTexImage2D(texture_target, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            } else {
                std::cerr << "ERROR: texture has unsupported " << n_channels << " channels: " << path << std::endl;
            }
        } else {
            std::cerr << "ERROR: failed to load texture image: " << path << std::endl;
            if (stbi_failure_reason()) {
                std::cout << "ERROR: stbi_failure_reason: " << stbi_failure_reason() << std::endl;
            }
        }

        stbi_image_free(data);
    }
};

#endif // MODELS_TEXTURES_H_
