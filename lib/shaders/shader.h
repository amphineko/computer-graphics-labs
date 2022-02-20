#ifndef SHADER_H_
#define SHADER_H_

#include <fstream>
#include <iostream>
#include <sstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram {
public:
    ShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath) {
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        if (!LoadShaderFromFile(vertexShaderPath, GL_VERTEX_SHADER, vertexShader) ||
            !LoadShaderFromFile(fragmentShaderPath, GL_FRAGMENT_SHADER, fragmentShader)) {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return;
        }

        program_ = glCreateProgram();
        glAttachShader(program_, vertexShader);
        glAttachShader(program_, fragmentShader);
        glLinkProgram(program_);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        if (!EnsureProgramLinked(program_)) {
            glDeleteProgram(program_);
            program_ = 0;
        }
    }

    [[nodiscard]] bool IsReady() const { return program_ != 0; }

    void Use() const { glUseProgram(this->program_); }

    GLint GetInt(const char *name) const {
        GLint location = glGetUniformLocation(program_, name);
        if (location == -1) {
            std::cerr << "Could not find uniform " << name << std::endl;
            return 0;
        }

        GLint result;
        glGetUniformiv(program_, location, &result);
        return result;
    }

    void GetVec3(const char *name, glm::vec3 &value) const {
        GLint location = glGetUniformLocation(program_, name);
        glGetUniformfv(program_, location, glm::value_ptr(value));
    }

    void SetFloat(const char *name, float value) const { glUniform1f(glGetUniformLocation(program_, name), value); }

    void SetInt(const char *name, GLint value) const { glUniform1i(glGetUniformLocation(program_, name), value); }

    void SetMat3(const char *name, glm::mat3 mat) const {
        glUniformMatrix3fv(glGetUniformLocation(program_, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void SetMat4(const char *name, glm::mat4 mat) const {
        glUniformMatrix4fv(glGetUniformLocation(program_, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void SetVec3(const char *name, glm::vec3 vec) const {
        glUniform3f(glGetUniformLocation(program_, name), vec.x, vec.y, vec.z);
    }

    void SetVec3Array(const char *name, GLsizei count, const glm::vec3 *vec) const {
        glUniform3fv(glGetUniformLocation(program_, name), count, glm::value_ptr(vec[0]));
    }

private:
    GLuint program_ = 0;

    GLuint current_texture_ = GL_TEXTURE0;

    static bool EnsureProgramLinked(GLuint program) {
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            const GLsizei log_buffer_size = 1024;
            GLchar log[log_buffer_size];

            glGetProgramInfoLog(program, log_buffer_size, nullptr, log);
            std::cout << "Error: Could not link program: " << log << std::endl;

            return false;
        }

        return true;
    }

    static bool EnsureShaderCompiled(GLuint shader, GLenum shaderType) {
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            const GLsizei log_buffer_size = 1024;
            GLchar log[log_buffer_size];

            glGetShaderInfoLog(shader, log_buffer_size, nullptr, log);
            std::cout << "Error: Could not compile shader type " << shaderType << ": " << log << std::endl;

            return false;
        }

        return true;
    }

    static bool LoadShaderFromFile(const char *filePath, GLenum shaderType, GLuint &shader) {
        std::string shaderCode;

        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            shaderFile.open(filePath);

            std::stringstream shaderStream;
            shaderStream << shaderFile.rdbuf();

            shaderCode = shaderStream.str();

            shaderFile.close();
        } catch (std::ifstream::failure &e) {
            std::cout << "Error: Could not open shader file " << filePath << " (" << e.code() << ")" << std::endl;
            return false;
        }

        const char *pShaderCode = shaderCode.c_str();
        glShaderSource(shader, 1, &pShaderCode, nullptr);
        return EnsureShaderCompiled(shader, shaderType);
    }
};

#endif