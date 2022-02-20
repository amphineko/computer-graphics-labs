#version 410 core

layout (location = 0) in vec3 position;

out vec3 vUv;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
    vUv = position;

    vec4 result = projectionMatrix * mat4(mat3(viewMatrix)) * vec4(position, 1.0);
    gl_Position = result.xyww;
}