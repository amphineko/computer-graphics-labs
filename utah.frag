#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;

uniform vec3 camera_direction;
uniform vec3 camera_position;
uniform vec3 light_direction;
uniform vec3 light_position;

uniform vec3 mesh_ambient;
uniform vec3 mesh_diffuse;
uniform vec3 mesh_specular;
uniform float mesh_shininess;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_normal0;
uniform sampler2D texture_height0;

void main()
{
    // TODO: apply rest of the texture maps

    // texture

    vec3 texture_color = texture(texture_diffuse0, fTexCoords).rgb;
    vec3 ambient_color = mesh_ambient * texture_color;

    vec3 normal = normalize(fNormal);
    vec3 light_direction = normalize(light_position - fPosition);

    // diffuse

    float diffuse_factor = max(dot(normal, light_direction), 0.0);
    vec3 diffuse_color = mesh_diffuse * texture_color * diffuse_factor;

    // specular

    vec3 view_direction = normalize(camera_position - fPosition);
    vec3 reflect_direction = reflect(-light_direction, normal);

    float specular_factor = pow(max(dot(view_direction, reflect_direction), 0.0), 16.0f);
    vec3 specular_color = mesh_specular * texture_color * specular_factor;

    vec3 result = (ambient_color + diffuse_color + specular_color) * texture_color;
    FragColor = vec4(result, 1.0);
}