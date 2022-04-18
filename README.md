# CS7GV{3,5}: Real-time Rendering and Animation

This repository contains implementations of assignments for the course _CS7GV3: Real-time Rendering_ and _CS7GV5: Real-time Animation_.

## Shared Components

```
├── lib
│   │
│   ├── cameras                 - Camera-related classes
│   │   ├── camera_fp.h         - First-person camera
│   │   ├── camera_tp.h         - Third-person (orbit) camera
│   │   └── common.h
│   │
│   ├── kinematics              - Inverse kinematics
│   │   ├── bone.h
│   │   └── inverse.h
│   │
│   ├── models                  - Scene & model classes
│   │   ├── mesh.h              - Low-level mesh class
│   │   ├── node.h              - Model node class
│   │   ├── scene.h             - Scene class (root node)
│   │   └── textures.h          - Global texture manager
│   │
│   ├── program.h               - Base class for OpenGL programs
│   │                             (to be extended by assignment-specific programs)
│   │
│   ├── shaders
│   │   ├── feedback_shader.h   - Shader with transform feedback varyings
│   │   └── shader.h            - Shader program loader & wrapper
│   │
│   ├── skybox.h                - Skybox: cubemap + shader
│   └── utils.h                 - OpenGL debugging & error handling
│
├── reports                     - LaTeX assignment report sources
│
├── resources                   - Resource files
│   │
│   ├── fonts                   - Fonts for text and ImGui
│   │
│   ├── models                  - Model files (see models/README.md)
│   │
│   └── textures                - Texture files (see textures/README.md)
│
├── shaders                     - Shader files (see below for assignment-specific shaders)
│  
├── vendors                     - Third-party libraries (glad, glm, imgui, ozz-animation, stb_image)
```

## CS7GV3: Real-time Rendering

```
├── lens.cpp                    - Assignment 5: Research Implementation (Depth-of-Field)
├── mipmap.cpp                  - Assignment 4: Mipmapping
├── normalmap.cpp               - Assignment 3: Normal mapping
|-- transmittance.cpp           - Assignment 2 & 1: Transmittance Effects and Reflectance
```

## CS7GV5: Real-time Animation

```
├── inverse_kinematics.cpp      - Assignment 2: Inverse Kinematics
├── vertex_pick.cpp             - Assignment 3: Facial Animation
```

## Build Instructions

This repository uses the [CMake](https://cmake.org/) build system.

```
# checkout submodules
git submodule update --init

# build all targets
cmake --build .
```

## License

Source code that authored by amphineko, is licensed under the MIT license.

Resources and other assets may be licensed under different licenses, see the individual files.
