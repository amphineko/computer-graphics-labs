#version 410 core

#define MAX_BLUR_LAYERS 16

in vec2 vUv;

out vec4 fragColor;

uniform sampler2DArray colorTexture;
uniform sampler2D depthTexture;

uniform float focusDistance;
uniform float focusDistanceMax;
uniform float focusStep;

uniform int drawDepth;
uniform float drawDepthMax;
uniform float drawDepthMin;

uniform int drawBlur;
uniform int drawBlurLevel;

uniform mat4 inverseProjectionMatrix;

void main() {
    vec4 ndcPos = vec4(vUv, 2.0 * texture(depthTexture, vUv.xy).x - 1.0, 1.0);
    vec4 viewPosW = inverseProjectionMatrix * ndcPos;
    vec3 viewPos = viewPosW.xyz / viewPosW.w;

    float depth = -viewPos.z;

    float pixelStep = clamp(abs(depth - focusDistance) / focusStep, 0, MAX_BLUR_LAYERS - 1);

    vec3 result = texture(colorTexture, vec3(vUv, pixelStep)).rgb * 0.625;
    result += texture(colorTexture, vec3(vUv, clamp(pixelStep - 1, 0, MAX_BLUR_LAYERS - 1))).rgb * 0.125;
    result += texture(colorTexture, vec3(vUv, clamp(pixelStep + 1, 0, MAX_BLUR_LAYERS - 1))).rgb * 0.125;

    fragColor = vec4(result, 1.0);

    if (drawDepth == 1) {
        fragColor = vec4(vec3((-viewPos.z - drawDepthMin) / drawDepthMax), 1.0);
    }

    if (drawBlur == 1) {
        fragColor = vec4(texture(colorTexture, vec3(vUv, drawBlurLevel)).rgb, 1.0);
    }
}
