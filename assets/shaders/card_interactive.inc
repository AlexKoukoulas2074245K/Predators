
const vec4 INTERACTIVE_COLOR = vec4(1.0f, 0.0f, 1.0f, 1.0f);
const float INTERACTIVE_COLOR_DISTANCE_THRESHOLD = 0.4f;

vec4 calculate_interactive_color(vec4 color, int mode)
{
    if (distance(color, INTERACTIVE_COLOR) < INTERACTIVE_COLOR_DISTANCE_THRESHOLD)
    {
        switch(mode)
        {
            case 0: return vec4(1.0f, 1.0f, 1.0f, 1.0f);
            case 1: return vec4(0.0f, 0.8f, 0.0f, 1.0f);
            case 2: return vec4(0.7f, 0.0f, 0.0f, 1.0f);
        }
    }
    return color;
}
