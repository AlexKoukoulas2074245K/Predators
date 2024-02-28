#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

in float frag_lifetime;
in vec2 uv_frag;
in vec3 frag_unprojected_pos;
out vec4 frag_color;

uniform float custom_alpha;
uniform sampler2D tex;

void main()
{
    // Calculate final uvs
    float final_uv_x = uv_frag.x;
    float final_uv_y = 1.0 - uv_frag.y;
    
    // Get texture color
    frag_color = texture(tex, vec2(final_uv_x/3.0f + 0.33333f, final_uv_y/3.0f + 0.05f));
    
    if (frag_color.a < 0.1) discard;
    
    frag_color.a = clamp(frag_lifetime, 0.0f, 1.0f);
    
    frag_color.a *= custom_alpha;
}
