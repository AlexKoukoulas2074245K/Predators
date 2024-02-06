#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

in vec2 uv_frag;
in vec3 frag_unprojected_pos;

uniform sampler2D tex;
uniform sampler2D dissolve_tex;
uniform vec4 ambient_light_color;
uniform vec4 point_light_colors[32];
uniform vec3 point_light_positions[32];
uniform float point_light_powers[32];
uniform float custom_alpha;
uniform float dissolve_threshold;
uniform float dissolve_magnitude;
uniform float origin_x;
uniform float origin_y;
uniform bool affected_by_light;
uniform int active_light_count;
out vec4 frag_color;

void main()
{
    float final_uv_x = uv_frag.x;
    float final_uv_y = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(final_uv_x/3 + 0.33333f, final_uv_y/3 + 0.05f));
    
    float distance_uv_x = (frag_unprojected_pos.x - origin_x) * dissolve_magnitude;
    float distance_uv_y = (frag_unprojected_pos.y - origin_y) * dissolve_magnitude;

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
        frag_color = mix(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f), distance_to_dissolve_color/glow_threshold);
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