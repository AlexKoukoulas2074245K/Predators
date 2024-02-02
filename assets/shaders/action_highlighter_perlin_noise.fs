#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

in vec2 uv_frag;
in vec3 frag_unprojected_pos;

uniform vec4 ambient_light_color;
uniform vec4 point_light_colors[32];
uniform vec3 point_light_positions[32];
uniform float point_light_powers[32];
uniform float time;
uniform float custom_alpha;
uniform float time_speed;
uniform float perlin_resolution;
uniform float perlin_clarity;
uniform bool invalid_action;
uniform bool affected_by_light;
uniform int active_light_count;
out vec4 frag_color;

#include "perlin_noise.inc"

void main()
{
    float perlinNoise = perlin(perlin_resolution, time, time_speed);
    
    float distanceFromCenter = 1.0f - distance(uv_frag, vec2(0.5f, 0.5f));
    distanceFromCenter -= 0.5f;
    
    if (invalid_action)
    {
        frag_color = vec4(vec3(perlinNoise + perlin_clarity).r, 0.0f, 0.0f, (perlinNoise + perlin_clarity) * distanceFromCenter);
    }
    else
    {
        frag_color = vec4(0.0f, vec3(perlinNoise + perlin_clarity).g, 0.0f, (perlinNoise + perlin_clarity) * distanceFromCenter);
    }
    
    
    if (affected_by_light)
    {
        vec4 light_accumulator = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        
        for (int i = 0; i < active_light_count; ++i)
        {
            float dst = distance(point_light_positions[i], frag_unprojected_pos);
            float attenuation = point_light_powers[i] / (dst * dst);
                    
            light_accumulator.rgb += (point_light_colors[i] * attenuation).rgb;
        }
        
        frag_color = frag_color * ambient_light_color + light_accumulator;
    }
    
    frag_color.a *= custom_alpha;
}
