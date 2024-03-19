#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

in vec2 uv_frag;
in vec3 frag_unprojected_pos;
in vec3 frag_pos;
in vec3 normal_interp;

uniform sampler2D tex;
uniform vec3 point_light_position;
uniform float point_light_power;
uniform float custom_alpha;
uniform float light_pos_x;
uniform bool affected_by_light;
uniform bool achievement_unlocked;
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
    
    if (affected_by_light)
    {
        vec4 light_accumulator = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        vec3 view_direction = normalize(vec3(0.0f, 0.0f, -1.0f) /* eye pos */ - frag_pos);
        
        for (int i = 0; i < 1; ++i)
        {
            vec3 normal = normalize(normal_interp);

            vec3 light_direction = normalize(point_light_position - frag_pos);
            vec4 diffuse_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
            vec4 specular_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);

            float diffuse_factor = max(dot(normal, light_direction), 0.0f);
            if (diffuse_factor > 0.0f)
            {
                diffuse_color = vec4(0.6f, 0.6f, 0.6f, 1.0f) /* mat_diffuse */ * diffuse_factor;
                diffuse_color = clamp(diffuse_color, 0.0f, 1.0f);

                vec3 reflected_direction = normalize(reflect(-light_direction, normal));

                specular_color = vec4(0.8f, 0.8f, 0.8f, 1.0f) /* mat_spec */ * pow(max(dot(view_direction, reflected_direction), 0.0f), 1.0f /* shiny */);
                specular_color = clamp(specular_color, 0.0f, 1.0f);
            }
            
            float distance = distance(point_light_position, frag_unprojected_pos);
            float attenuation = point_light_power / (distance * distance);
            
            light_accumulator.rgb += (diffuse_color * attenuation + specular_color * attenuation).rgb;
        }
        
        frag_color = frag_color * vec4(0.5f, 0.5f, 0.5f, 1.0f) /* ambient_light_color */ + light_accumulator;
    }
    
    frag_color.a *= custom_alpha;
}
