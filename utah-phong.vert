#version 330 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBitangent;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoord;
out vec3 fTangent;
out vec3 fBitangent;

uniform mat4 model;
uniform mat4 model_normal;
uniform mat4 model_rotation;
uniform mat4 view;
uniform mat4 projection;

void main() {
    fPosition = vPosition;
    fTexCoord = vTexCoord;

    fNormal = vNormal;
    fTangent = vTangent;
    fBitangent = vBitangent;

    gl_Position = projection * view * model * vec4(vPosition, 1.0);
}
