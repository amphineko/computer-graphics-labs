#version 410 core

layout(location = 0) in vec3 position;

out float vCursorDistance;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform vec2 mousePos;

void main() {
    vec4 vPosition = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
    vCursorDistance = distance(vPosition.xyz / vPosition.w, vec3(mousePos, 0.0));
    gl_Position = vPosition;
}
