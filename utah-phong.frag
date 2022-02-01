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

vec3 blinnPhongBRDF(vec3 light_direction, vec3 view_direction, vec3 normal, vec3 diffuse, vec3 specular, float shininess) {
    vec3 halfway_direction = normalize(view_direction + light_direction);
    return diffuse + pow(max(dot(halfway_direction, normal), 0.0), shininess) * specular;
}

void main() {
    vec3 tex_diffuse = n_texture_diffuse > 0 ? texture(texture_diffuse0, fTexCoord).xyz : mesh_diffuse;
    vec3 tex_specular = n_texture_specular > 0 ? texture(texture_specular0, fTexCoord).xyz : mesh_specular;

    vec3 normal;
    if (n_texture_normal > 0) {
        normal = normalize(mat3(fTangent, fBitangent, fNormal) * (texture(texture_normal0, fTexCoord).xyz * 2.0 - 1.0));
    } else {
        normal = normalize(fNormal);
    }
    vec3 t_normal = normalize(model_normal * vec4(normal, 0.0)).xyz;

    vec3 light_direction = normalize(light_position - fPosition);
    vec3 view_direction = normalize(camera_position - fPosition);
    vec3 halfway_direction = normalize(light_direction + view_direction);

    // ambient 

    vec3 ambient = tex_diffuse * light_ambient;

    // diffuse

    float diffuse_factor = max(dot(t_normal, light_direction), 0.0);
    vec3 diffuse = tex_diffuse * light_diffuse * diffuse_factor;

    // specular

    float specular_factor = pow(max(dot(t_normal, halfway_direction), 0.0), 32.0);
    vec3 specular = tex_specular * light_specular * specular_factor;

    // // vec3 result = (ambient_color + diffuse_color + specular_color);

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
