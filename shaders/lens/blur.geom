#version 410 core

#define MAX_BLUR_LAYERS 16

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

flat out int blurLevel;
out vec2 gUv;

in vec2 vUv[3];

void main() {
    for (int i = 0; i < MAX_BLUR_LAYERS; i++) {
        for (int j = 0; j < 3; j++) {
            blurLevel = i;
            gUv = vUv[j];

            gl_Layer = i;
            gl_Position = gl_in[j].gl_Position;

            EmitVertex();
        }
    }

    EndPrimitive();
}
