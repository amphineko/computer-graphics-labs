#include "shader.h"

class FeedbackShaderProgram : public ShaderProgram {
public:
    FeedbackShaderProgram(const std::vector<std::pair<std::string, GLenum>> &shader_sources,
                          const std::vector<std::string> &feedback_varyings) {
        std::vector<GLuint> shaders;
        if (LoadShadersFromFiles(shader_sources, shaders)) {
            program_ = glCreateProgram();
            ConfigureTransformFeedbackVaryings(feedback_varyings);
            if (!LinkProgramFromShaders(shaders, program_)) {
                glDeleteProgram(program_);
                program_ = 0;
            }
        }
        DestroyShaders(shaders);
    }

    /**
     * @deprecated
     */
    explicit FeedbackShaderProgram(const char *vertex_shader_path)
        : FeedbackShaderProgram(
              {
                  {vertex_shader_path, GL_VERTEX_SHADER},
              },
              {"vCursorDistance"}) {}

private:
    void ConfigureTransformFeedbackVaryings(const std::vector<std::string> &feedback_varyings) {
        const GLchar *varyings[feedback_varyings.size()];
        for (int i = 0; i < feedback_varyings.size(); i++) {
            varyings[i] = feedback_varyings[i].c_str();
        }
        glTransformFeedbackVaryings(program_, GLsizei(feedback_varyings.size()), varyings, GL_INTERLEAVED_ATTRIBS);
    }
};