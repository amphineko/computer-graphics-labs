#version 330 core

in vec3 vPos;
in vec4 vColor;

out vec4 color;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * vec4(vPos, 1.0);
    color = vColor;
}