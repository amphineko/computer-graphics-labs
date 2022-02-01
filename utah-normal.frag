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

void main() {
    vec3 normal;
    if (n_texture_normal > 0) {
        normal = normalize(mat3(fTangent, fBitangent, fNormal) * (texture(texture_normal0, fTexCoord).xyz * 2.0 - 1.0));
    } else {
        normal = normalize(fNormal);
    }
    vec3 t_normal = normalize(model_normal * vec4(normal, 0.0)).xyz;

    FragColor = vec4((normalize(t_normal + 1.0) / 2.0), 1.0);
}
