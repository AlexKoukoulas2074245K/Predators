precision mediump float;

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
uniform int interactive_mode;
out vec4 frag_color;

const vec4 INTERACTIVE_COLOR = vec4(1.0f, 0.0f, 1.0f, 1.0f);
const float INTERACTIVE_COLOR_DISTANCE_THRESHOLD = 0.4f;

void main()
{
    float final_uv_x = uv_frag.x;
    float final_uv_y = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));

    if (frag_color.a < 0.1) discard;
    
    if (distance(frag_color, INTERACTIVE_COLOR) < INTERACTIVE_COLOR_DISTANCE_THRESHOLD)
    {
        switch(interactive_mode)
        {
            case 0: frag_color = vec4(1.0f, 1.0f, 1.0f, 1.0f); break;
            case 1: frag_color = vec4(0.7f, 0.0f, 0.0f, 1.0f); break;
            case 2: frag_color = vec4(0.0f, 0.8f, 0.0f, 1.0f); break;
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
