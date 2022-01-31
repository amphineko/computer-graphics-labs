#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoords;

uniform mat4 model;
uniform mat3 model_normal;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fPosition = vPosition;
    fNormal = model_normal * vNormal;
    fTexCoords = vTexCoord;

    gl_Position = projection * view * model * vec4(vPosition, 1.0);
}
