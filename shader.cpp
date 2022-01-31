#include <fstream>
#include <iostream>
#include <sstream>
#include "shader.h"

ShaderProgram::ShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath) {
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

bool ShaderProgram::LoadShaderFromFile(const char *filePath, GLenum shaderType, GLuint &shader) {
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

bool ShaderProgram::EnsureShaderCompiled(GLuint shader, GLenum shaderType) {
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

bool ShaderProgram::EnsureProgramLinked(GLuint program) {
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

bool ShaderProgram::IsReady() const {
    return program_ != 0;
}

void ShaderProgram::Use() const {
    glUseProgram(this->program_);
}

void ShaderProgram::SetFloat(const char *name, float value) const {
    glUniform1f(glGetUniformLocation(program_, name), value);
}

void ShaderProgram::SetMat3(const char *name, glm::mat3 mat) const {
    glUniformMatrix3fv(glGetUniformLocation(program_, name), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::SetMat4(const char *name, glm::mat4 mat) const {
    glUniformMatrix4fv(glGetUniformLocation(program_, name), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::SetVec3(const char *name, glm::vec3 vec) const {
    glUniform3f(glGetUniformLocation(program_, name), vec.x, vec.y, vec.z);
}

