#version 330 core

out vec4 FragColor;

in vec3 vPosition;
in vec2 vUv;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec3 vWorldViewDirection;

in vec3 vLightDirections[16];
in vec3 vViewDirection;

uniform vec3 cameraPosition;

uniform vec3 meshAmbient;
uniform vec3 meshDiffuse;
uniform vec3 meshSpecular;
uniform float meshShininess;

uniform int nDiffuseMap;
uniform int nSpecularMap;
uniform int nNormalMap;
uniform int nHeightMap;

uniform sampler2D diffuseMap0;
uniform sampler2D specularMap0;
uniform sampler2D normalMap0;
uniform sampler2D heightMap0;

void main() {
    vec3 lightDiffuse = vec3(0.5, 0.5, 0.5);

    vec3 matAmbient = vec3(0.25, 0.25, 0.25);
    vec3 matDiffuse = nDiffuseMap > 0 ? texture(diffuseMap0, vUv).xyz : meshDiffuse;
    vec3 matSpecular = nSpecularMap > 0 ? texture(specularMap0, vUv).xyz : meshSpecular;

    vec3 lightDirection = vLightDirections[0];

    float diffuseFactor = max(dot(lightDirection, vWorldNormal), 0.0);
    vec3 diffuse = matDiffuse * lightDiffuse * diffuseFactor;

    vec3 halfwayDirection = normalize(lightDirection + vWorldViewDirection);
    float specularFactor = pow(max(dot(halfwayDirection, vWorldNormal), 0.0), 32.0);
    vec3 specular = matSpecular * lightDiffuse * specularFactor;

    FragColor = vec4(diffuse + specular, 1.0);
}