#include "utah.h"

int main() {
    UtahProgram program;
    std::string window_title = "Lab: Utah Teapot";
    program.Initialize(&window_title);
    program.Run();
    return 0;
}
