#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;
in vec3 fTangent;
in vec3 fBitangent;

out vec4 FragColor;

uniform mat4 model_normal;

uniform vec3 camera_position;
uniform vec3 light_ambient;
uniform vec3 light_diffuse;
uniform vec3 light_position;
uniform vec3 light_specular;

uniform vec3 mesh_ambient;
uniform vec3 mesh_diffuse;
uniform vec3 mesh_specular;
uniform float mesh_shininess;

uniform int n_texture_diffuse;
uniform int n_texture_specular;
uniform int n_texture_normal;
uniform int n_texture_height;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_normal0;
uniform sampler2D texture_height0;

#define F0 0.8
#define roughness 0.1
#define k 0.2
#define PI 3.1415926535897932384626433832795

vec3 CookTorrance(vec3 materialDiffuseColor, vec3 materialSpecularColor, vec3 normal, vec3 lightDir, vec3 viewDir, vec3 lightColor) {
    float NdotL = max(0, dot(normal, lightDir));
    float Rs = 0.0;
    if (NdotL > 0) {
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
    return materialDiffuseColor * lightColor * NdotL + lightColor * materialSpecularColor * NdotL * (k + Rs * (1.0 - k));
}

void main() {
    vec3 normal;
    if (n_texture_normal > 0) {
        normal = normalize(mat3(fTangent, fBitangent, fNormal) * (texture(texture_normal0, fTexCoord).xyz * 2.0 - 1.0));
    } else {
        normal = normalize(fNormal);
    }
    vec3 t_normal = normalize(model_normal * vec4(normal, 0.0)).xyz;

    vec3 light_direction = normalize(light_position - fPosition);
    vec3 view_direction = normalize(camera_position - fPosition);

    vec3 reflect_direction = normalize(reflect(-light_direction, t_normal));

    vec3 tex_diffuse = n_texture_diffuse > 0 ? texture(texture_diffuse0, fTexCoord).xyz : mesh_diffuse;
    vec3 tex_specular = n_texture_specular > 0 ? texture(texture_specular0, fTexCoord).xyz : mesh_specular;

    vec3 result = CookTorrance(tex_diffuse, tex_specular, t_normal, light_direction, view_direction, light_ambient + light_diffuse);
    FragColor = vec4(result, 1.0);
}