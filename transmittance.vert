#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

out vec3 vPosition;
out vec2 vUv;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitagent;

out vec3 vWorldPosition;
out vec3 vWorldNormal;

out vec3 vViewDirection;

out vec3 vReflectDirection;
out vec3 vRefractDirectionR;
out vec3 vRefractDirectionG;
out vec3 vRefractDirectionB;

out vec3 vTexCoords;

uniform vec3 cameraPosition;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform float fresnelEtaR;
uniform float fresnelEtaG;
uniform float fresnelEtaB;

uniform int nNormalMap;
uniform sampler2D normalMap0;

void main() {
    vTexCoords = position;

    vPosition = vec3(viewMatrix * modelMatrix * vec4(position, 1.0));
    vUv = uv;
    vNormal = normal;
    if (nNormalMap == 1) {
        vec3 T = normalize(vec3(modelMatrix * vec4(tangent, 0.0)));
        vec3 B = normalize(vec3(modelMatrix * vec4(bitangent, 0.0)));
        vec3 N = normalize(vec3(modelMatrix * vec4(normal, 0.0)));

        vNormal = texture(normalMap0, uv).xyz;
        vNormal = normalize(vNormal * 2.0 - 1.0);
        vNormal = mat3(tangent, bitangent, normal) * vNormal;
    }

    vWorldPosition = vec3(modelMatrix * vec4(position, 1.0));
    vWorldNormal = vec3(modelMatrix * vec4(normal, 0.0));

    vViewDirection = vWorldPosition.xyz - cameraPosition;

    vReflectDirection = reflect(vViewDirection, vWorldNormal);
    vRefractDirectionR = refract(vViewDirection, vWorldNormal, fresnelEtaR);
    vRefractDirectionG = refract(vViewDirection, vWorldNormal, fresnelEtaG);
    vRefractDirectionB = refract(vViewDirection, vWorldNormal, fresnelEtaB);

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}