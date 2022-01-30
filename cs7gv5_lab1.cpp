#include "program.cpp"

vertex_t vertices[] = {
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

class CS7GV5Lab1 : public Program {
public:
    CS7GV5Lab1() : Program("cs7gv5_lab1.vert", "cs7gv5_lab1.frag") {
        vertices_ = &vertices[0];
        vertices_size_ = sizeof(vertices);

        camera_->SetPosition(-3.0f, 4.5f, 3.0f);
        camera_->SetRotation(-45.0f, 315.0f);
    }

protected:
    virtual void Draw() {
        Program::Draw();
        for (GLint i = 0; i < sizeof(vertices) / sizeof(vertex_t) / 3; ++i) {
            glDrawArrays(GL_TRIANGLES, i * 3, 3);
        }
    }
};

int main() {
    CS7GV5Lab1 program;
    if (!program.Initialize()) { return -1; }
    program.Run();
    return 0;
}
