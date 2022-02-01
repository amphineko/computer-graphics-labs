#version 330 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBitangent;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoords;
out vec3 fTangent;
out vec3 fBitangent;

uniform mat4 model;// model transform
uniform mat4 model_normal;// transpose(inverse(model transform))
uniform mat4 view;
uniform mat4 projection;

void main() {
    fPosition = vPosition;
    fTexCoords = vTexCoord;

    fNormal = vNormal;
    fTangent = vTangent;
    fBitangent = vBitangent;

    gl_Position = projection * view * model * vec4(vPosition, 1.0);
}
