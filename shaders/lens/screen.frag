#version 410 core

in vec2 vUv;

out vec4 fragColor;

uniform sampler2D colorTexture;

void main() {
    fragColor = vec4(texture(colorTexture, vUv).rgb, 1.0);
}