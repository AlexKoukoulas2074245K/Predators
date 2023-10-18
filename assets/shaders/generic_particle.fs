precision mediump float;

in float frag_lifetime;
in vec2 uv_frag;

out vec4 frag_color;

uniform sampler2D tex;

void main()
{
    // Calculate final uvs
    float finalUvX = uv_frag.x;
    float finalUvY = 1.00 - uv_frag.y;

    // Get texture color
    frag_color = texture(tex, vec2(finalUvX, finalUvY));
    frag_color.r = frag_color.r * min(1.0f, frag_lifetime);
    frag_color.g = frag_color.r * min(1.0f, frag_lifetime);
    frag_color.b = frag_color.r * min(1.0f, frag_lifetime);
    frag_color.a = frag_color.r * min(1.0f, frag_lifetime);
    
    if (frag_color.a < 0.1) discard;
}
