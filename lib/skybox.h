#ifndef LIB_SKYBOX_H_
#define LIB_SKYBOX_H_

#include <map>

#include "models/textures.h"
#include "shaders/shader.h"

class Skybox {
public:
    void Draw(ShaderProgram *shader) {
        glDisable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_.name);
        shader->SetInt("cubeMap0", 0);

        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, triangle_count_);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
    }

    bool Initialize(float size,
                    const std::map<GLenum, std::string> &faces,
                    const std::string &base_path,
                    TextureManager &textureManager) {
        cube_map_ = textureManager.LoadCubeMap(NODE_TEXTURE_ROLE_CUBE_MAP, "skybox_", faces, base_path);

        float vertices[] = {-size, size,  -size, -size, -size, -size, size,  -size, -size, size,  -size, -size,
                            size,  size,  -size, -size, size,  -size, -size, -size, size,  -size, -size, -size,
                            -size, size,  -size, -size, size,  -size, -size, size,  size,  -size, -size, size,
                            size,  -size, -size, size,  -size, size,  size,  size,  size,  size,  size,  size,
                            size,  size,  -size, size,  -size, -size, -size, -size, size,  -size, size,  size,
                            size,  size,  size,  size,  size,  size,  size,  -size, size,  -size, -size, size,
                            -size, size,  -size, size,  size,  -size, size,  size,  size,  size,  size,  size,
                            -size, size,  size,  -size, size,  -size, -size, -size, -size, -size, -size, size,
                            size,  -size, -size, size,  -size, -size, -size, -size, size,  size,  -size, size};
        triangle_count_ = sizeof(vertices) / sizeof(float) / 3;

        glGenVertexArrays(1, &vao_);
        glBindVertexArray(vao_);

        glGenBuffers(1, &vbo_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
    }

private:
    NodeTexture cube_map_;

    GLuint vao_, vbo_;
    GLsizei triangle_count_;
};

#endif // LIB_SKYBOX_H_
