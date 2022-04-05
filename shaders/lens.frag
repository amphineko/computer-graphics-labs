#version 410 core

in vec2 vUv;

out vec4 fragColor;

uniform sampler2D color;
uniform sampler2D depth;

uniform int drawDepth;
uniform float drawDepthMax;
uniform float drawDepthMin;

uniform float zFar;
uniform float zNear;

uniform mat4 inverseProjectionMatrix;

void main() {
    fragColor = vec4(texture(color, vUv).xyz, 1.0);

    vec4 ndcPos = vec4(vUv, 2.0 * texture(depth, vUv.xy).x - 1.0, 1.0);
    vec4 viewPosW = inverseProjectionMatrix * ndcPos;
    vec3 viewPos = viewPosW.xyz / viewPosW.w;

    if (drawDepth == 1) {
        fragColor = vec4(vec3((-viewPos.z - drawDepthMin) / drawDepthMax), 1.0);
    }
}
