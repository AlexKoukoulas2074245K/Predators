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
uniform float time;
uniform float time_speed;
uniform float color_factor_r;
uniform float color_factor_g;
uniform float color_factor_b;
uniform float perturbation_factor;
uniform float noise_0_factor;
uniform float noise_1_factor;
uniform float noise_2_factor;
uniform float noise_3_factor;
uniform float noise_4_factor;
uniform float noise_5_factor;
uniform bool affected_by_light;
uniform int active_light_count;
out vec4 frag_color;

void main()
{
    float final_uv_x = uv_frag.x;
    float final_uv_y = 1.0f - uv_frag.y;

    vec2 n0Uv = vec2(final_uv_x * 1.4f  + 0.01f,  final_uv_y        + time * time_speed * 0.69f);
    vec2 n1Uv = vec2(final_uv_x * 0.5f  - 0.033f, final_uv_y * 2.0f + time * time_speed * 0.12f);
    vec2 n2Uv = vec2(final_uv_x * 0.94f + 0.02f,  final_uv_y * 3.0f + time * time_speed * 0.61f);
    
    float n0A = (texture(tex, n0Uv).a - 0.5f) * 2.0f;
    float n1A = (texture(tex, n1Uv).a - 0.5f) * 2.0f;
    float n2A = (texture(tex, n2Uv).a - 0.5f) * 2.0f;
    
    float noiseA = clamp(n0A * noise_0_factor + n1A * noise_1_factor + n2A * noise_2_factor, -1.0, 1.0);
    
    vec2 n0UvB = vec2(final_uv_x * 0.7f - 0.01f,   final_uv_y * 1.0f + time * time_speed * 0.27f);
    vec2 n1UvB = vec2(final_uv_x * 0.45f + 0.033f, final_uv_y * 1.9f + time * time_speed * 0.61f);
    vec2 n2UvB = vec2(final_uv_x * 0.8f - 0.02f,   final_uv_y * 2.5f + time * time_speed * 0.51f);
    
    float n0B = (texture(tex, n0UvB).a - 0.5f) * 2.0f;
    float n1B = (texture(tex, n1UvB).a - 0.5f) * 2.0f;
    float n2B = (texture(tex, n2UvB).a - 0.5f) * 2.0f;
    float noiseB = clamp(n0B * noise_3_factor + n1B * noise_4_factor + n2B * noise_5_factor, -1.0f, 1.0f);
    
    vec2 final_noise = vec2(noiseA, noiseB);
    float perturb = (1.0f - final_uv_y) * 0.35f * perturbation_factor + 0.02f;
    final_noise = (final_noise * perturb) + vec2(final_uv_x, final_uv_y) - 0.02f;
    
    vec4 color = texture(tex, final_noise);

    color = vec4(color.r * 2.0f * color_factor_r, color.g * 0.9f * color_factor_g, (color.g/color.r) * 0.2f * color_factor_b, 1.0f);
    final_noise = clamp(final_noise, 0.05f, 1.0f);
    color.a = texture(tex, final_noise).b * 2.0f;
    color.a = color.w * texture(tex, vec2(final_uv_x, final_uv_y)).b;
    
    frag_color = color;
    
    //frag_color = vec4(1.0f, 0.0f, 0.0f, texture(tex, vec2(final_uv_x, final_uv_y)).r);
    
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
