#ifndef UTAH_H_
#define UTAH_H_

#include "cameras/camera_tp.h"
#include "model.h"
#include "program.h"

class UtahProgram : public Program {
public:
    UtahProgram();

    bool Initialize(const std::string *window_title) override;

protected:
    void Draw() override;

private:
    Model *model_obj1_ = nullptr;
    Model *model_obj2_ = nullptr;

    double last_frame_clock_ = 0.0;

    ShaderProgram *current_shader_ = nullptr;

    void HandleKeyboardInput(double frame_time);
};

#endif
