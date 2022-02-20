#version 410 core

in vec3 vUv;

out vec4 FragColor;

uniform samplerCube cubeMap0;

void main()
{
    FragColor = texture(cubeMap0, vUv);
}