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
uniform float custom_alpha;
uniform bool affected_by_light;
uniform int active_light_count;
out vec4 frag_color;

void main()
{
    float final_uv_x = uv_frag.x * 0.999f;
    float final_uv_y = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));

    if (frag_color.a < 0.1) discard;
    
    frag_color.a *= custom_alpha;
    
    if (final_uv_x > 0.95f || final_uv_y > 0.95f)
    {
        frag_color.a *= mix(0.8f, 0.0f, (max(final_uv_x, final_uv_y) - 0.95f) * 20.0f);
    }
    if (final_uv_x < 0.05f || final_uv_y < 0.05f)
    {
        frag_color.a *= mix(0.8f, 0.0f, (0.05f - min(final_uv_x, final_uv_y)) * 20.0f);
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
