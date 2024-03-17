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
uniform float loading_progress;
uniform float glow_threshold;
uniform float time;
uniform bool affected_by_light;
out vec4 frag_color;

#include "perlin_noise.inc"

void main()
{
    float final_uv_x = uv_frag.x * 0.999f;
    float final_uv_y = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));

    if (frag_color.a < 0.1) discard;
    if (frag_color.r < 0.99 && frag_color.g < 0.99 && frag_color.b < 0.99) discard;
    
    float perlin_noise = perlin(312.0f, time/10.0f, 5.0f);
    float ridged_noise = 1.0 - abs(perlin_noise);
    ridged_noise = pow(ridged_noise, 15.0f);
    frag_color = vec4(ridged_noise * 0.2, ridged_noise * 0.3, ridged_noise * 0.56, 1.0f);
    
    float x_cutoff = (loading_progress - 0.5f)/2.0f;
    if (frag_unprojected_pos.x > x_cutoff - glow_threshold)
    {
        frag_color.a *= (x_cutoff - min(x_cutoff, frag_unprojected_pos.x))/glow_threshold;
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
