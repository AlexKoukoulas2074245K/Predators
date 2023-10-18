precision mediump float;

in vec2 uv_frag;
in vec3 frag_unprojected_pos;

uniform sampler2D tex;
uniform vec4 ambient_light_color;
uniform vec4 point_light_colors[32];
uniform vec3 point_light_positions[32];
uniform float point_light_powers[32];
uniform float time;
uniform float time_speed;
uniform float perlin_resolution_x;
uniform float perlin_resolution_y;
uniform float perlin_clarity;
uniform bool affected_by_light;
uniform int active_light_count;
out vec4 frag_color;

vec2 randomGradient(vec2 p)
{
  p = p + 0.02;
  float x = dot(p, vec2(123.4, 234.5));
  float y = dot(p, vec2(234.5, 345.6));
  vec2 gradient = vec2(x, y);
  gradient = sin(gradient);
  gradient = gradient * 43758.5453;

  // part 4.5 - update noise function with time
  gradient = sin(gradient + time * time_speed);
  return gradient;

  // gradient = sin(gradient);
  // return gradient;
}

vec2 cubic(vec2 p)
{
  return p * p * (3.0 - p * 2.0);
}

vec2 quintic(vec2 p)
{
  return p * p * p * (10.0 + p * (-15.0 + p * 6.0));
}

void main()
{
    // part 0 - basic shader setup
    vec2 res = vec2(perlin_resolution_x, perlin_resolution_y);
    vec2 uv = gl_FragCoord.xy/res;
    
    // uncomment for final final demo
    uv = gl_FragCoord.xy/res.y;
    
    vec3 black = vec3(0.0);
    vec3 white = vec3(1.0);
    vec3 color = black;
    
    // part 1 - set up a grid of cells
    uv = uv * 4.0;
    vec2 gridId = floor(uv);
    vec2 gridUv = fract(uv);
    color = vec3(gridId, 0.0);
    color = vec3(gridUv, 0.0);
    
    // part 2.1 - start by finding the coords of grid corners
    vec2 bl = gridId + vec2(0.0, 0.0);
    vec2 br = gridId + vec2(1.0, 0.0);
    vec2 tl = gridId + vec2(0.0, 1.0);
    vec2 tr = gridId + vec2(1.0, 1.0);
    
    // part 2.2 - find random gradient for each grid corner
    vec2 gradBl = randomGradient(bl);
    vec2 gradBr = randomGradient(br);
    vec2 gradTl = randomGradient(tl);
    vec2 gradTr = randomGradient(tr);
    
    // part 3.2 - find distance from current pixel to each grid corner
    vec2 distFromPixelToBl = gridUv - vec2(0.0, 0.0);
    vec2 distFromPixelToBr = gridUv - vec2(1.0, 0.0);
    vec2 distFromPixelToTl = gridUv - vec2(0.0, 1.0);
    vec2 distFromPixelToTr = gridUv - vec2(1.0, 1.0);
    
    // part 4.1 - calculate the dot products of gradients + distances
    float dotBl = dot(gradBl, distFromPixelToBl);
    float dotBr = dot(gradBr, distFromPixelToBr);
    float dotTl = dot(gradTl, distFromPixelToTl);
    float dotTr = dot(gradTr, distFromPixelToTr);
    
    // part 4.4 - smooth out gridUvs
    // gridUv = smoothstep(0.0, 1.0, gridUv);
    // gridUv = cubic(gridUv);
    gridUv = quintic(gridUv);
    
    float b = mix(dotBl, dotBr, gridUv.x);
    float t = mix(dotTl, dotTr, gridUv.x);
    float perlin = mix(b, t, gridUv.y);
    
    // part 5.1 - billow noise
    // float billow = abs(perlin);
    // color = vec3(billow);

    // part 5.2 - ridged noise
     //float ridgedNoise = 1.0 - abs(perlin);
     //ridgedNoise = ridgedNoise * ridgedNoise;
    // color = vec3(ridgedNoise);
    
    float distanceFromCenter = 1.0f - distance(uv_frag, vec2(0.5f, 0.5f));
    distanceFromCenter -= 0.5f;
    
    color = vec3(perlin + perlin_clarity);
    frag_color = vec4(0.0f, color.g, 0.0f, (perlin + perlin_clarity) * distanceFromCenter);
    
    
    
    
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
