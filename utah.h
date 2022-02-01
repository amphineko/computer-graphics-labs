#ifndef UTAH_H_
#define UTAH_H_

#include "cameras/camera_tp.h"
#include "model.h"
#include "program.h"

class UtahProgram : public Program {
public:
    UtahProgram();

    bool Initialize(std::string window_title) override;

protected:
    void Draw() override;

private:
    ShaderProgram *shader_basic_ = nullptr;

    Model *model_cone_ = nullptr;
    Model *model_teapot_ = nullptr;

    double last_frame_clock_ = 0.0;
};

#endif
