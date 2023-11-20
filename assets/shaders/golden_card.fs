precision mediump float;

in vec2 uv_frag;
in vec3 frag_unprojected_pos;

uniform sampler2D tex;
uniform sampler2D distortion_tex;
uniform sampler2D distortion_mask_tex;
uniform vec4 ambient_light_color;
uniform vec4 point_light_colors[32];
uniform vec3 point_light_positions[32];
uniform float point_light_powers[32];
uniform float custom_alpha;
uniform float time;
uniform float res;
uniform float light_power;
uniform float diffuse_intensity;
uniform float spec_intensity;
uniform float distance_threshold;
uniform float distortion_mag;
uniform bool affected_by_light;
uniform bool show_specular;
uniform bool show_diffuse;
uniform int active_light_count;
uniform int weight_interactive_mode;
uniform int damage_interactive_mode;
out vec4 frag_color;

#include "card_interactive.inc"

float gray(vec4 col) {
    return (col.r + col.g + col.b) / 3.0f;
}

vec4 calculate_golden_card_color(vec4 color, float time, vec2 uv, sampler2D main_texture, sampler2D distortion_tex)
{
    float grayScale = gray(color);
    
    vec2 distortion_scroll = vec2(time, time);
    vec2 distortion_color = (texture(distortion_tex, uv + distortion_scroll).rg - 0.5f) * 2.0f;
    vec2 distorted_uvs = uv + distortion_color * distortion_mag * (texture(distortion_mask_tex, uv)).r;
                                                                    
    float dx = gray(texture(main_texture, distorted_uvs + vec2(1.0f/res, 0.0f))) - grayScale;
    float dy = gray(texture(main_texture, distorted_uvs + vec2(0.0f, 1.0f/res))) - grayScale;
    vec3 normal = normalize(vec3(-dx, -dy, 0.4f));
    
    vec3 lightPos = vec3(vec2(sin(time * 30.0f)/1.5f, 0.5f /*cos(time * 30.0f)/1.5f*/), 0.4f);
    //vec3 lightPos = vec3(0.5f, 0.5f, 0.4f);
    float fft = 0.1f;
    vec3 lightDir = lightPos - vec3(distorted_uvs, 0.4f);
    float lightDist = length(lightDir);
    vec3 lighting = vec3(0.0f);
    
    if (lightDist <= distance_threshold)
    {
        float diffIntensity = diffuse_intensity * fft;
        float specIntensity = spec_intensity * fft;
        float diffuse = dot(normal, lightDir) * diffIntensity;
        float specular = pow(dot(normal, normalize(lightDir + vec3(0.0f, 0.0f, 1.0f))), light_power) * specIntensity;
        vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
        float lightRadius = 0.9 + fft;
        lighting = lightColor * ((show_diffuse ? diffuse : 0.0f) + (show_specular ? specular : 0.0f)) * max(0.0f, lightRadius - lightDist) / lightDist;
    }
    //return vec4(normal, 1.0f);
    return vec4(texture(main_texture, distorted_uvs).rgb + lighting, 1.0f);
}

void main()
{
    float final_uv_x = uv_frag.x;
    float final_uv_y = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));

    if (frag_color.a < 0.1) discard;
    
    frag_color = calculate_golden_card_color(frag_color, time, vec2(final_uv_x, final_uv_y), tex, distortion_tex);
    frag_color = calculate_interactive_color(frag_color, weight_interactive_mode, damage_interactive_mode, time);
    
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
