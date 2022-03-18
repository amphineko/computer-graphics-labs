#include "shader.h"

class VertexPickerShaderProgram : public ShaderProgram {
public:
    VertexPickerShaderProgram(const char *vertex_shader_path) {
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        if (!LoadShaderFromFile(vertex_shader_path, GL_VERTEX_SHADER, vertex_shader)) {
            glDeleteShader(vertex_shader);
            return;
        }

        program_ = glCreateProgram();
        glAttachShader(program_, vertex_shader);

        const GLchar *varyings[] = {feedback_varyings_[0].c_str()};
        glTransformFeedbackVaryings(program_, 1, varyings, GL_INTERLEAVED_ATTRIBS);

        glLinkProgram(program_);

        glDeleteShader(vertex_shader);

        if (!EnsureProgramLinked(program_)) {
            glDeleteProgram(program_);
            program_ = 0;
        }

        glCheckError();
    }

private:
    const std::string feedback_varyings_[1] = {"vCursorDistance"};
};