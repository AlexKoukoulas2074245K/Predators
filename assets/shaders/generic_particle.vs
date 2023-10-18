precision mediump float;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 position;
layout(location = 3) in float lifetime;
layout(location = 4) in float size;

uniform mat4 view;
uniform mat4 proj;

out float frag_lifetime;
out vec2 uv_frag;

mat4 rotationMatrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}


void main()
{
    uv_frag = uv;
    vec3 frag_unprojected_pos = vec3
    (
        (vertex_position.x * size) + position.x,
        (vertex_position.y * size) + position.y,
        (vertex_position.z * size) + position.z
    );

    frag_lifetime = lifetime;
    gl_Position = proj * view * vec4(frag_unprojected_pos, 1.0f);
}
