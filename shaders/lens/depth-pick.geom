#version 410 core

layout (points) in;
layout (line_strip, max_vertices = 4) out;

out float gDepth;

in float vDepth[];

const vec4 positionOffsets1[] = vec4[2](vec4(-0.05, 0.0, 0.0, 0.0), vec4(0.0, -0.05, 0.0, 0.0));
const vec4 positionOffsets2[] = vec4[2](vec4(0.05, 0.0, 0.0, 0.0), vec4(0.0, 0.05, 0.0, 0.0));

void main() {
    for (int i = 0; i < 2; i++) {
        gDepth = vDepth[0];
        gl_Position = gl_in[0].gl_Position + positionOffsets1[i];
        EmitVertex();

        gDepth = vDepth[0];
        gl_Position = gl_in[0].gl_Position + positionOffsets2[i];
        EmitVertex();

        EndPrimitive();
    }
}