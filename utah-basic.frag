#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 FragColor;

uniform mat4 model_normal;// transpose(inverse(model transform))

uniform sampler2D texture_diffuse1;

uniform vec3 camera_direction;
uniform vec3 camera_position;
uniform vec3 light_direction;
uniform vec3 light_position;

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

vec3 blinnPhongBRDF(vec3 light_direction, vec3 view_direction, vec3 normal, vec3 diffuse, vec3 specular, float shininess) {
    vec3 halfway_direction = normalize(view_direction + light_direction);
    return diffuse + pow(max(dot(halfway_direction, normal), 0.0), shininess) * specular;
}

void main() {
    vec3 light_color = vec3(1.0, 1.0, 1.0);// TODO: read light color from shader input

    vec3 light_direction = normalize(light_position - fPosition);
    vec3 view_direction = normalize(camera_position - fPosition);

    vec3 normal = normalize(model_normal * vec4(fNormal, 0.0)).xyz;

    vec3 diffuse = n_texture_diffuse > 0 ? texture(texture_diffuse0, fTexCoords).xyz : mesh_diffuse;
    vec3 specular = n_texture_specular > 0 ? texture(texture_specular0, fTexCoords).xyz : mesh_specular;
    // vec3 normal = n_texture_normal > 0 ? normalize(texture(texture_normal0, fTexCoords).xyz * 2.0 - 1.0) : fNormal;
    float height = n_texture_height > 0 ? texture(texture_height0, fTexCoords).x : 0.0;

    vec3 radiance = diffuse * mesh_ambient;

    float irradiance = max(dot(light_direction, normal), 0.0);
    if(irradiance > 0.0) {
        vec3 brdf = blinnPhongBRDF(light_direction, view_direction, normal, diffuse, specular, mesh_shininess);
        radiance += brdf * irradiance * light_color;
    }

    radiance = pow(radiance, vec3(1.0 / 2.2));// gamma correction

    FragColor = vec4(radiance * vec3(1.0), 1.0);
}