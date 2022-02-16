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

uniform int nLights;
uniform vec3 lightDiffuses[16];

#define F0 0.8
#define roughness 0.1
#define k 0.2
#define PI 3.1415926535897932384626433832795

vec3 cookTorrance(vec3 diffuse, vec3 specular, vec3 normal, vec3 lightDir, vec3 viewDir, vec3 lightColor)
{
    float NdotL = max(0, dot(normal, lightDir));
    float Rs = 0.0;

    if (NdotL > 0)
    {
        vec3 H = normalize(lightDir + viewDir);
        float NdotH = max(0, dot(normal, H));
        float NdotV = max(0, dot(normal, viewDir));
        float VdotH = max(0, dot(lightDir, H));

        // Fresnel reflectance
        float F = pow(1.0 - VdotH, 5.0);
        F *= (1.0 - F0);
        F += F0;

        // Microfacet distribution by Beckmann
        float m_squared = roughness * roughness;
        float r1 = 1.0 / (4.0 * m_squared * pow(NdotH, 4.0));
        float r2 = (NdotH * NdotH - 1.0) / (m_squared * NdotH * NdotH);
        float D = r1 * exp(r2);

        // Geometric shadowing
        float two_NdotH = 2.0 * NdotH;
        float g1 = (two_NdotH * NdotV) / VdotH;
        float g2 = (two_NdotH * NdotL) / VdotH;
        float G = min(1.0, min(g1, g2));

        Rs = (F * D * G) / (PI * NdotL * NdotV);
    }

    return diffuse * lightColor * NdotL + lightColor * specular * NdotL * (k + Rs * (1.0 - k));
}


void main() {
    vec3 lightDiffuse = vec3(lightDiffuses[0]);

    vec3 matAmbient = vec3(0.25, 0.25, 0.25);
    vec3 matDiffuse = nDiffuseMap > 0 ? texture(diffuseMap0, vUv).xyz : meshDiffuse;
    vec3 matSpecular = nSpecularMap > 0 ? texture(specularMap0, vUv).xyz : meshSpecular;

    vec3 lightDirection = vLightDirections[0];

    vec3 reflectDirection = normalize(reflect(lightDirection, vWorldNormal));

    vec3 diffuse = nDiffuseMap > 0 ? texture(diffuseMap0, vUv).xyz : meshDiffuse;
    vec3 specular = nSpecularMap > 0 ? texture(specularMap0, vUv).xyz : meshSpecular;

    vec3 result = cookTorrance(diffuse, specular, vWorldNormal, lightDirection, vWorldViewDirection, lightDiffuse);
    FragColor = vec4(result, 1.0);
}