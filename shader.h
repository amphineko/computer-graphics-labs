#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include "glm/glm.hpp"

class ShaderProgram {
public:
    ShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath);

    [[nodiscard]] GLuint GetProgram() const { return program_; }
    [[nodiscard]] bool IsReady() const;

    void Use() const;

    void SetFloat(const char *name, float value) const;
    void SetMat3(const char *name, glm::mat3 mat) const;
    void SetMat4(const char *name, glm::mat4 mat) const;
    void SetVec3(const char *name, glm::vec3 vec) const;

    static bool EnsureProgramLinked(GLuint program);

    static bool EnsureShaderCompiled(GLuint shader, GLenum shaderType);
    static bool LoadShaderFromFile(const char *filePath, GLenum shaderType, GLuint &shader);

private:
    GLuint program_ = 0;
};

#endif