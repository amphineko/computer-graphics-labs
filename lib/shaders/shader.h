#ifndef SHADER_H_
#define SHADER_H_

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "../utils.h"

#define MAX_N_LIGHTS 16

class ShaderProgram {
public:
    ShaderProgram(const char *vertexShaderPath, const char *geometryShaderPath, const char *fragmentShaderPath)
        : ShaderProgram({
              {vertexShaderPath, GL_VERTEX_SHADER},
              {geometryShaderPath, GL_GEOMETRY_SHADER},
              {fragmentShaderPath, GL_FRAGMENT_SHADER},
          }) {}

    ShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath)
        : ShaderProgram({
              {vertexShaderPath, GL_VERTEX_SHADER},
              {fragmentShaderPath, GL_FRAGMENT_SHADER},
          }) {}

    ShaderProgram(const std::vector<std::pair<std::string, GLenum>> &shader_sources) {
        std::vector<GLuint> shaders;
        if (LoadShadersFromFiles(shader_sources, shaders)) {
            program_ = glCreateProgram();
            if (!LinkProgramFromShaders(shaders, program_)) {
                glDeleteProgram(program_);
                program_ = 0;
            }
        }
        DestroyShaders(shaders);
    }

    void ConfigureCamera(const glm::vec3 &camera_position, const glm::mat4 &projection, const glm::mat4 &view) {
        Use();
        glCheckError();

        SetVec3("cameraPosition", camera_position);
        SetMat4("projectionMatrix", projection);
        SetMat4("viewMatrix", view);
        glCheckError();
    }

    void ConfigureLights(GLuint n_lights,
                         const glm::vec3 *positions,
                         const glm::vec3 *directions,
                         const glm::vec3 *diffuses) {
        Use();
        glCheckError();

        SetInt("nLights", n_lights);
        SetVec3Array("lightDiffuses", MAX_N_LIGHTS, diffuses);
        SetVec3Array("lightDirections", MAX_N_LIGHTS, directions);
        SetVec3Array("lightPositions", MAX_N_LIGHTS, positions);
        glCheckError();
    }

    [[nodiscard]] bool IsReady() const { return program_ != 0; }

    void Use() const { glUseProgram(this->program_); }

    void SetFloat(const char *name, float value) const { glUniform1f(glGetUniformLocation(program_, name), value); }

    void SetInt(const char *name, GLint value) const { glUniform1i(glGetUniformLocation(program_, name), value); }

    void SetMat3(const char *name, glm::mat3 mat) const {
        glUniformMatrix3fv(glGetUniformLocation(program_, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void SetMat4(const char *name, glm::mat4 mat) const {
        glUniformMatrix4fv(glGetUniformLocation(program_, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void SetVec2(const char *name, glm::vec2 vec) const {
        glUniform2fv(glGetUniformLocation(program_, name), 1, glm::value_ptr(vec));
    }

    void SetVec3(const char *name, glm::vec3 vec) const {
        glUniform3fv(glGetUniformLocation(program_, name), 1, glm::value_ptr(vec));
    }

    void SetVec3Array(const char *name, GLsizei count, const glm::vec3 *vec) const {
        glUniform3fv(glGetUniformLocation(program_, name), count, glm::value_ptr(vec[0]));
    }

protected:
    GLuint program_ = 0;

    ShaderProgram() = default;

    static void DestroyShaders(const std::vector<GLuint> &entries) {
        for (auto &entry : entries) {
            glDeleteShader(entry);
        }
    }

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

    static bool LinkProgramFromShaders(const std::vector<GLuint> &shaders, GLuint &program) {
        for (auto shader : shaders) {
            glAttachShader(program, shader);
        }
        glLinkProgram(program);

        return EnsureProgramLinked(program);
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

    static bool LoadShadersFromFiles(const std::vector<std::pair<std::string, GLenum>> &shader_sources,
                                     std::vector<GLuint> &shaders) {
        for (auto &source : shader_sources) {
            const auto &[path, type] = source;

            auto shader = glCreateShader(type);
            shaders.push_back(shader);

            if (!LoadShaderFromFile(path.c_str(), type, shader)) {
                return false;
            }
        }

        return true;
    }

private:
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
};

#endif