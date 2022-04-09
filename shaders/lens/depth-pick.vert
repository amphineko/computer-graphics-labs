#version 410 core

out float vDepth;

uniform vec2 cursorPosition;

uniform sampler2D depthTexture;

uniform mat4 inverseProjectionMatrix;

void main() {
    vec4 ndcPos = vec4(0.0, 0.0, 2.0 * texture(depthTexture, cursorPosition.xy).x - 1.0, 1.0);
    vec4 viewPosW = inverseProjectionMatrix * ndcPos;
    vec3 viewPos = viewPosW.xyz / viewPosW.w;

    vDepth = -viewPos.z;
    gl_Position = vec4(cursorPosition * 2.0 - 1.0, -0.5, 1.0);
}
