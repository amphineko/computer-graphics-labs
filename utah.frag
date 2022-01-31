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

float near = 0.1;
float far = 100.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    // texture

    vec3 color = vec3(31.0f / 255.0f, 71.0f / 255.0f, 136.0f / 255.0f);
    vec3 ambient_color = 0.5 * color;

    vec3 normal = normalize(fNormal);
    vec3 light_direction = normalize(light_position - fPosition);

    // diffuse

    float diffuse_factor = max(dot(normal, light_direction), 0.0);
    vec3 diffuse_color = diffuse_factor * color;

    // specular

    vec3 view_direction = normalize(camera_position - fPosition);
    vec3 reflect_direction = reflect(-light_direction, normal);

    float specular_factor = pow(max(dot(view_direction, reflect_direction), 0.0), 16.0f);
    vec3 specular_color = specular_factor * color;

    vec3 result = (ambient_color + diffuse_color + specular_color) * color;
    FragColor = vec4(result, 1.0);
}