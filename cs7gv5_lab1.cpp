#include "program.h"

class CS7GV5Lab1 : public Program {
public:
    CS7GV5Lab1() : Program("cs7gv5_lab1.vert", "cs7gv5_lab1.frag") {
        vertices_ = const_cast<simple_vertex_t *>(program_vertices_);
        vertices_size_ = sizeof(program_vertices_);

        camera_->SetPosition(-1.75f, 2.0f, 3.0f);
        camera_->SetRotation(-30.0f, 300.0f);
    }

protected:
    void Draw() final {
        Program::Draw();
        for (GLint i = 0; i < vertices_size_ / sizeof(simple_vertex_t) / 3; ++i) {
            glDrawArrays(GL_TRIANGLES, i * 3, 3);
        }
    }

private:
    constexpr static const simple_vertex_t program_vertices_[] = {
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
    if (!program.Initialize("CS7GV5 Lab 1: Hello Triangle")) { return -1; }
    program.Run();
    return 0;
}
