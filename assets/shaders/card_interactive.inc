#include "perlin_noise.inc"

const vec4 WEIGHT_INTERACTIVE_COLOR = vec4(1.0f, 0.0f, 1.0f, 1.0f);
const vec4 DAMAGE_INTERACTIVE_COLOR = vec4(0.0f, 1.0f, 0.0f, 1.0f);
const float INTERACTIVE_COLOR_DISTANCE_THRESHOLD = 0.4f;
const float PERLIN_TIME_SPEED = 6.0f;
const float PERLIN_RESOLUTION = 170.0f;
const float PERLIN_CLARITY = 1.110f;

vec4 calculate_golden_card_color(vec4 color)
{
    return vec4(1.0f);
}

vec4 calculate_interactive_color(vec4 color, int weight_mode, int damage_mode, float time)
{
    float perlinNoise = perlin(PERLIN_RESOLUTION, time, PERLIN_TIME_SPEED);
    
    if (distance(color, WEIGHT_INTERACTIVE_COLOR) < INTERACTIVE_COLOR_DISTANCE_THRESHOLD)
    {
        switch(weight_mode)
        {
            case 0: return vec4(1.0f, 1.0f, 1.0f, 1.0f);
            case 1: return vec4(0.0f, vec3(perlinNoise + PERLIN_CLARITY).g, 0.0f, 1.0f);
            case 2: return vec4(vec3(perlinNoise + PERLIN_CLARITY).r, 0.0f, 0.0f, 1.0f);
        }
    }
    
    if (distance(color, DAMAGE_INTERACTIVE_COLOR) < INTERACTIVE_COLOR_DISTANCE_THRESHOLD * 1.4f)
    {
        switch(damage_mode)
        {
            case 0: return vec4(1.0f, 1.0f, 1.0f, 1.0f);
            case 1: return vec4(0.0f, vec3(perlinNoise + PERLIN_CLARITY).g, 0.0f, 1.0f);
            case 2: return vec4(vec3(perlinNoise + PERLIN_CLARITY).r, 0.0f, 0.0f, 1.0f);
        }
    }
    
    return color;
}
