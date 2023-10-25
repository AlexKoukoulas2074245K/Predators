precision mediump float;

in vec2 uv_frag;
in vec3 frag_unprojected_pos;

uniform sampler2D tex;
uniform vec4 ambient_light_color;
uniform vec4 point_light_colors[32];
uniform vec3 point_light_positions[32];
uniform float point_light_powers[32];
uniform float weight_value;
uniform bool affected_by_light;
uniform int active_light_count;
out vec4 frag_color;

void main()
{
    float final_uv_x = uv_frag.x;
    float final_uv_y = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));

    if (frag_color.a < 0.1) discard;
    
    if (frag_color.r > 0.99f && frag_color.g > 0.99f && frag_color.b > 0.99f && frag_color.a > 0.99f)
    {
        float dynamicWeightColor = min(10.0f, (11.0f - weight_value))/10.0f;
        frag_color = vec4(1.0f, dynamicWeightColor, dynamicWeightColor, 1.0f);
    }
    
#if defined(IOS)
    float temp = frag_color.r;
    frag_color.r = frag_color.b;
    frag_color.b = temp;
#endif
    
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
