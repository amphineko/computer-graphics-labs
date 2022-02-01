#ifndef SHADER_H_
#define SHADER_H_

#include "glm/glm.hpp"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram {
public:
    ShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath);

    [[nodiscard]] GLuint GetProgram() const { return program_; }

    [[nodiscard]] bool IsReady() const { return program_ != 0; }

    void Use() const { glUseProgram(this->program_); }

    static bool EnsureProgramLinked(GLuint program);

    static bool EnsureShaderCompiled(GLuint shader, GLenum shaderType);

    static bool LoadShaderFromFile(const char *filePath, GLenum shaderType, GLuint &shader);

    void SetFloat(const char *name, float value) const { glUniform1f(glGetUniformLocation(program_, name), value); }

    void SetMat3(const char *name, glm::mat3 mat) const {
        glUniformMatrix3fv(glGetUniformLocation(program_, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void SetMat4(const char *name, glm::mat4 mat) const {
        glUniformMatrix4fv(glGetUniformLocation(program_, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void SetInt(const char *name, GLint value) const { glUniform1i(glGetUniformLocation(program_, name), value); }

    void SetVec3(const char *name, glm::vec3 vec) const {
        glUniform3f(glGetUniformLocation(program_, name), vec.x, vec.y, vec.z);
    }

private:
    GLuint program_ = 0;
};

#endif