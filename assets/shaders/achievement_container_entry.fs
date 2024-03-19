#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

in vec2 uv_frag;
in vec3 frag_unprojected_pos;

uniform sampler2D tex;
uniform sampler2D golden_flakes_mask_tex;
uniform sampler2D dormant_mask_tex;
uniform vec4 ambient_light_color;
uniform vec4 point_light_colors[32];
uniform vec3 point_light_positions[32];
uniform float point_light_powers[32];
uniform float custom_alpha;
uniform float light_pos_x;
uniform float time;
uniform float cutoff_min_y;
uniform float cutoff_max_y;
uniform bool achievement_unlocked;
uniform bool affected_by_light;
uniform int active_light_count;
out vec4 frag_color;


float gray(vec4 col)
{
    return (col.r + col.g + col.b) / 3.0f;
}

void main()
{
    float final_uv_x = uv_frag.x * 0.999f;
    float final_uv_y = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));

    if (frag_color.a < 0.1) discard;
    
    float gray_scale = gray(frag_color);
    
    
    if (achievement_unlocked)
    {
        float dx = gray(texture(tex, vec2(final_uv_x, final_uv_y) + vec2(1.0f/360.0f, 0.0f))) - gray_scale;
        float dy = gray(texture(tex, vec2(final_uv_x, final_uv_y) + vec2(0.0f, 1.0f/360.0f))) - gray_scale;
        vec3 normal = normalize(vec3(-dx, -dy, 0.4f));
        
        vec3 light_pos = vec3(light_pos_x, 0.0f, 0.4f);

        float fft = 0.1f;
        vec3 light_dir = light_pos - vec3(frag_unprojected_pos.xy, 0.4f);
        float light_dist = abs(light_pos.x - frag_unprojected_pos.x);
        vec3 lighting = vec3(0.0f);
        
        if (light_dist <= 0.445f)
        {
            float diff_intensity = 0.3f * fft;
            float spec_intensity = 0.5f * fft;
            float diffuse = dot(normal, light_dir) * diff_intensity;
            float specular = pow(dot(normal, normalize(light_dir + vec3(0.0f, 0.0f, 1.0f))), 64.0f) * spec_intensity;
            vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
            float light_radius = 0.9f + fft;
            lighting = light_color * (diffuse + specular) * max(0.0f, light_radius - light_dist) / light_dist;
            frag_color.rgb = vec3(frag_color.rgb + lighting * 1.121f);
        }
    }
    else
    {
        frag_color.rgb = vec3(gray_scale);
    }
    
    const float CUTOFF_VALUE = 0.02f;
    if (frag_unprojected_pos.y < cutoff_min_y + CUTOFF_VALUE)
    {
        frag_color.a *= (max(cutoff_min_y, frag_unprojected_pos.y) - cutoff_min_y)/CUTOFF_VALUE;
    }
    if (frag_unprojected_pos.y > cutoff_max_y - CUTOFF_VALUE)
    {
        frag_color.a *= (cutoff_max_y - min(cutoff_max_y, frag_unprojected_pos.y))/CUTOFF_VALUE;
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
