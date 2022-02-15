#version 330 core

in vec3 vPosition;
in vec2 vUv;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;

in vec3 vViewDirection;

in vec3 vWorldPosition;
in vec3 vWorldNormal;

in vec3 vReflectDirection;
in vec3 vRefractDirectionR;
in vec3 vRefractDirectionG;
in vec3 vRefractDirectionB;

out vec4 FragColor;

uniform samplerCube envMap;

uniform float fresnelBias;
uniform float fresnelPower;
uniform float fresnelScale;

void main() {
    float ratio = clamp(fresnelBias + fresnelScale * pow(1.0 + dot(normalize(vViewDirection), vWorldNormal), fresnelPower), 0.0, 1.0);

    vec3 reflectColor = texture(envMap, vec3(-vReflectDirection.x, vReflectDirection.yz)).rgb;

    float refractColorR = texture(envMap, vec3(vRefractDirectionR.x, vRefractDirectionR.yz)).r;
    float refractColorG = texture(envMap, vec3(vRefractDirectionG.x, vRefractDirectionG.yz)).g;
    float refractColorB = texture(envMap, vec3(vRefractDirectionB.x, vRefractDirectionB.yz)).b;
    vec3 refractColor = vec3(refractColorR, refractColorG, refractColorB);

    FragColor = vec4(mix(refractColor.rgb, reflectColor.rgb, ratio), 1.0);
}