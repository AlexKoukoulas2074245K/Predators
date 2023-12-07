#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

in vec2 uv_frag;
in vec3 frag_unprojected_pos;

uniform sampler2D tex;
uniform sampler2D history_entry_mask;
uniform vec4 ambient_light_color;
uniform vec4 point_light_colors[32];
uniform vec3 point_light_positions[32];
uniform float point_light_powers[32];
uniform float custom_alpha;
uniform float light_pos_x;
uniform float cutoff_min_x;
uniform float cutoff_max_x;
uniform float time;
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
    float final_uv_x = uv_frag.x;
    float final_uv_y = 1.0f - uv_frag.y;
    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));
    vec4 mask_texture_color = texture(history_entry_mask, vec2(final_uv_x + 0.02f, final_uv_y * 0.955f + 0.02f));
    
    if (mask_texture_color.a < 0.1f)
    {
        discard;
    }
    else if (mask_texture_color.r > 0.8f)
    {
        float perlinNoise = perlin(perlin_resolution, time, time_speed);
        float distanceFromCenter = 1.0f - distance(uv_frag, vec2(0.5f, 0.5f));
        distanceFromCenter -= 0.58f;
        
        if (invalid_action)
        {
            frag_color = vec4(vec3(perlinNoise + perlin_clarity).r, 0.0f, 0.0f, (perlinNoise + perlin_clarity) * distanceFromCenter * mask_texture_color.a);
        }
        else
        {
            frag_color = vec4(0.0f, vec3(perlinNoise + perlin_clarity).g, 0.0f, (perlinNoise + perlin_clarity) * distanceFromCenter * mask_texture_color.a);
        }
    }
        
    const float CUTOFF_VALUE = 0.02f;
    if (frag_unprojected_pos.x < cutoff_min_x + CUTOFF_VALUE)
    {
        if (mask_texture_color.r > 0.8f)
        {
            frag_color.a *= mask_texture_color.a * (max(cutoff_min_x, frag_unprojected_pos.x) - cutoff_min_x)/CUTOFF_VALUE;
        }
        else
        {
            frag_color.a *= (max(cutoff_min_x, frag_unprojected_pos.x) - cutoff_min_x)/CUTOFF_VALUE;
        }
    }
    if (frag_unprojected_pos.x > cutoff_max_x - CUTOFF_VALUE)
    {
        if (mask_texture_color.r > 0.8f)
        {
            frag_color.a *= mask_texture_color.a * (cutoff_max_x - min(cutoff_max_x, frag_unprojected_pos.x))/CUTOFF_VALUE;
        }
        else
        {
            frag_color.a *= (cutoff_max_x - min(cutoff_max_x, frag_unprojected_pos.x))/CUTOFF_VALUE;
        }
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
