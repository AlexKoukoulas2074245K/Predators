#include "perlin_noise.inc"

const vec4 WEIGHT_INTERACTIVE_COLOR = vec4(1.0f, 0.0f, 1.0f, 1.0f);
const vec4 DAMAGE_INTERACTIVE_COLOR = vec4(0.0f, 1.0f, 0.0f, 1.0f);
const float INTERACTIVE_COLOR_DISTANCE_THRESHOLD = 0.4f;
const float PERLIN_TIME_SPEED = 6.0f;
const float PERLIN_RESOLUTION = 170.0f;
const float PERLIN_CLARITY = 1.110f;

float gray(vec4 col)
{
    return (col.r + col.g + col.b) / 3.0f;
}

vec4 calculate_golden_card_color(vec4 color, float time, vec2 uv, float light_pos_x, bool held_card, sampler2D main_texture, sampler2D golden_flakes_mask_tex)
{
    float gray_scale = gray(color);
                            
    float dx = gray(texture(main_texture, uv + vec2(1.0f/360.0f, 0.0f))) - gray_scale;
    float dy = gray(texture(main_texture, uv + vec2(0.0f, 1.0f/360.0f))) - gray_scale;
    vec3 normal = normalize(vec3(-dx, -dy, 0.4f));
    
    vec3 light_pos = vec3(light_pos_x, 0.0f, 0.4f);

    float fft = 0.1f;
    vec3 light_dir = light_pos - vec3(frag_unprojected_pos.xy, 0.4f);
    float light_dist = abs(light_pos.x - frag_unprojected_pos.x);
    vec3 lighting = vec3(0.0f);
    
    if (light_dist <= 0.445f && !held_card)
    {
        float diff_intensity = 0.3f * fft;
        float spec_intensity = 0.5f * fft;
        float diffuse = dot(normal, light_dir) * diff_intensity;
        float specular = pow(dot(normal, normalize(light_dir + vec3(0.0f, 0.0f, 1.0f))), 64.0f) * spec_intensity;
        vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
        float light_radius = 0.9f + fft;
        lighting = light_color * (diffuse + specular) * max(0.0f, light_radius - light_dist) / light_dist;
    }
    
    float perlin_noise = perlin(312.0f, time/10.0f, 5.0f);
    float ridged_noise = 1.0 - abs(perlin_noise);
    ridged_noise = pow(ridged_noise, 15.0f);
    vec4 golden_flakes_mask_color = texture(golden_flakes_mask_tex, uv);
    
    vec4 main_texture_color = texture(main_texture, uv);
    return vec4 (main_texture_color.rgb + lighting * 1.121f + ridged_noise * golden_flakes_mask_color.r * 0.2f, color.a);
}

vec4 calculate_card_color(vec4 color, vec2 uv, int weight_mode, int damage_mode, bool golden_card, bool held_card, float time, float light_pos_x, sampler2D main_texture, sampler2D golden_flakes_mask_tex)
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
    
    if (golden_card)
    {
        return calculate_golden_card_color(color, time, uv, light_pos_x, held_card, main_texture, golden_flakes_mask_tex);
    }
    
    return color;
}
