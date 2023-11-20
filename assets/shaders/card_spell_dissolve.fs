precision mediump float;

in vec2 uv_frag;
in vec3 frag_unprojected_pos;

uniform sampler2D tex;
uniform sampler2D golden_flakes_mask_tex;
uniform sampler2D dissolve_tex;
uniform vec4 ambient_light_color;
uniform vec4 point_light_colors[32];
uniform vec3 point_light_positions[32];
uniform float point_light_powers[32];
uniform float custom_alpha;
uniform float dissolve_threshold;
uniform float dissolve_magnitude;
uniform float card_origin_x;
uniform float card_origin_y;
uniform float time;
uniform float light_pos_x;
uniform bool affected_by_light;
uniform bool golden_card;
uniform bool held_card;
uniform int active_light_count;
uniform int weight_interactive_mode;
uniform int damage_interactive_mode;
out vec4 frag_color;

#include "card_interactive.inc"

void main()
{
    float final_uv_x = uv_frag.x;
    float final_uv_y = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));
    
    frag_color = calculate_card_color(frag_color, vec2(final_uv_x, final_uv_y), weight_interactive_mode, damage_interactive_mode, golden_card, held_card, time, light_pos_x, tex, golden_flakes_mask_tex);
    
    float distance_uv_x = (frag_unprojected_pos.x - card_origin_x) * dissolve_magnitude;
    float distance_uv_y = (frag_unprojected_pos.y - card_origin_y) * dissolve_magnitude;

    vec4 dissolve_color = texture(dissolve_tex, vec2(distance_uv_x, distance_uv_y));
    if (dissolve_color.r <= dissolve_threshold)
    {
        discard;
        return;
    }

    float distance_to_dissolve_color = abs(dissolve_color.r - dissolve_threshold);
    float glow_threshold = 0.05f;
    if (distance_to_dissolve_color < glow_threshold && frag_color.a > 0.1f)
    {
        frag_color = mix(vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), distance_to_dissolve_color/glow_threshold);
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
}
