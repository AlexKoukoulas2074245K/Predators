#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

in vec2 uv_frag;
in vec3 frag_unprojected_pos;

uniform sampler2D tex;
uniform vec4 ambient_light_color;
uniform vec4 point_light_colors[32];
uniform vec3 point_light_positions[32];
uniform float point_light_powers[32];
uniform float time;
uniform float custom_alpha;
uniform bool affected_by_light;
uniform int active_light_count;
out vec4 frag_color;

#include "perlin_noise.inc"

const vec4 INTERACTIVE_COLOR = vec4(1.0f, 0.0f, 1.0f, 1.0f);
const float INTERACTIVE_COLOR_DISTANCE_THRESHOLD = 0.4f;
const float GOLDEN_CARD_TEXT_PERLIN_TIME_SPEED = 6.0f;
const float GOLDEN_CARD_TEXT_PERLIN_RESOLUTION = 170.0f;
const float GOLDEN_CARD_TEXT_PERLIN_CLARITY = 1.110f;
const float GOLDEN_CARD_FLAKES_PERLIN_TIME_SPEED = 6.0f;
const float GOLDEN_CARD_FLAKES_PERLIN_RESOLUTION = 170.0f;
const float GOLDEN_CARD_FLAKES_RIDGED_POWER = 15.0f;
const float GOLDEN_CARD_FLAKES_CONTRIBUTION = 0.2f;

void main()
{
    float perlinNoise = perlin(GOLDEN_CARD_TEXT_PERLIN_RESOLUTION, time, GOLDEN_CARD_TEXT_PERLIN_TIME_SPEED);
    
    float final_uv_x = uv_frag.x * 0.999f;
    float final_uv_y = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));
    if (frag_color.a < 0.1) discard;
    
    if (distance(frag_color, INTERACTIVE_COLOR) < INTERACTIVE_COLOR_DISTANCE_THRESHOLD)
    {
        frag_color = vec4(vec3(perlinNoise + GOLDEN_CARD_TEXT_PERLIN_CLARITY).r, vec3(perlinNoise + GOLDEN_CARD_TEXT_PERLIN_CLARITY).g, 0.0f, (perlinNoise + GOLDEN_CARD_TEXT_PERLIN_CLARITY));
    }
    else
    {
        float perlin_noise = perlin(GOLDEN_CARD_FLAKES_PERLIN_RESOLUTION, time/10.0f, GOLDEN_CARD_FLAKES_PERLIN_TIME_SPEED);
        float ridged_noise = 1.0 - abs(perlin_noise);
        ridged_noise = pow(ridged_noise, GOLDEN_CARD_FLAKES_RIDGED_POWER);
        
        frag_color = vec4(frag_color.rgb + ridged_noise * GOLDEN_CARD_FLAKES_CONTRIBUTION, frag_color.a);
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
