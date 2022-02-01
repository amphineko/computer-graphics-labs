#include "cameras/camera_tp.h"
#include "model.h"
#include "program.h"

struct SimpleVertex {
    GLfloat x, y, z, r, g, b, a;
};

class CS7GV5Lab1 : public Program {
public:
    CS7GV5Lab1() : Program() {
        delete (FirstPersonCamera *)camera_;
        camera_ = new ThirdPersonCamera(0.0f, 0.0f, 5.0f, 0.0f, 270.0f);

        vertices_ = const_cast<SimpleVertex *>(program_vertices_);
        vertices_size_ = sizeof(program_vertices_);
    }

    bool Initialize(std::string window_title) override {
        if (!Program::Initialize(window_title)) {
            return false;
        }

        shader_ = new ShaderProgram("cs7gv5_lab1.vert", "cs7gv5_lab1.frag");
        if (!shader_->IsReady()) {
            shader_->~ShaderProgram();
            return false;
        }
        shader_->Use();
        shaders_.push_back(shader_);

        glGenVertexArrays(1, &vao_);
        glBindVertexArray(vao_);

        glGenBuffers(1, &vbo_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices_size_, vertices_, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (const void *)offsetof(SimpleVertex, x));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (const void *)offsetof(SimpleVertex, r));
        glEnableVertexAttribArray(1);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        return true;
    }

protected:
    void Draw() final {
        Program::Draw();

        shader_->Use();
        shader_->SetMat4("model", glm::mat4(1.0f));

        glBindVertexArray(vao_);
        auto triangle_max = sizeof(program_vertices_) / sizeof(SimpleVertex) / 3;
        for (GLint i = 0; i < triangle_max; i++) {
            // draw triangle with every 3 vertices
            glDrawArrays(GL_TRIANGLES, i * 3, 3);
        }
        glBindVertexArray(0);
    }

private:
    ShaderProgram *shader_ = nullptr;

    GLuint vao_ = 0, vbo_ = 0;

    Model *model_cone_ = nullptr;

    SimpleVertex *vertices_;
    GLsizeiptr vertices_size_;

    constexpr static const SimpleVertex program_vertices_[] = {
        {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},

        {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},

        {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f},

        {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f},

        {1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
    };
};

int main() {
    CS7GV5Lab1 program;
    if (!program.Initialize("CS7GV5 Lab 1: Hello Triangle")) {
        return -1;
    }
    program.Run();
    return 0;
}
