#version 330 core

out vec4 fragColor;

flat in int blurLevel;
in vec2 gUv;

uniform sampler2D colorTexture;

const float accWeights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
const float offsetWeights[] = float[](0.0, 0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.01, 0.011, 0.012, 0.013, 0.014, 0.015);

void main()
{
    vec3 result = texture(colorTexture, gUv).rgb * accWeights[0];
    for (int i = 1; i < 5; ++i)
    {
        float accWeight = accWeights[i];

        vec2 offsetX = vec2(offsetWeights[blurLevel], 0.0) * i;
        vec2 offsetY = vec2(0.0, offsetWeights[blurLevel]) * i;

        result += texture(colorTexture, gUv + offsetX).rgb * accWeight;
        result += texture(colorTexture, gUv - offsetX).rgb * accWeight;

        result += texture(colorTexture, gUv + offsetY).rgb * accWeight;
        result += texture(colorTexture, gUv - offsetY).rgb * accWeight;
    }
    fragColor = vec4(result, 1.0);
}