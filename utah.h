#ifndef UTAH_H_
#define UTAH_H_

#include "program.h"
#include "model.h"

class UtahProgram : public Program {
public:
    UtahProgram();

    bool Initialize(std::string window_title) override;

protected:
    void Draw() override;

    void InitializeObjects() override;

private:
    Model *floor_ = nullptr;
    Model *model_ = nullptr;
    Model *model_small_ = nullptr;

    double last_frame_clock_ = 0.0;
};

#endif
