
const vec4 WEIGHT_INTERACTIVE_COLOR = vec4(1.0f, 0.0f, 1.0f, 1.0f);
const vec4 DAMAGE_INTERACTIVE_COLOR = vec4(1.0f, 1.0f, 0.0f, 1.0f);
const float INTERACTIVE_COLOR_DISTANCE_THRESHOLD = 0.4f;

vec4 calculate_interactive_color(vec4 color, int weight_mode, int damage_mode)
{
    if (distance(color, WEIGHT_INTERACTIVE_COLOR) < INTERACTIVE_COLOR_DISTANCE_THRESHOLD)
    {
        switch(weight_mode)
        {
            case 0: return vec4(1.0f, 1.0f, 1.0f, 1.0f);
            case 1: return vec4(0.0f, 0.8f, 0.0f, 1.0f);
            case 2: return vec4(0.7f, 0.0f, 0.0f, 1.0f);
        }
    }
    
    if (distance(color, DAMAGE_INTERACTIVE_COLOR) < INTERACTIVE_COLOR_DISTANCE_THRESHOLD)
    {
        switch(damage_mode)
        {
            case 0: return vec4(1.0f, 1.0f, 1.0f, 1.0f);
            case 1: return vec4(0.0f, 0.8f, 0.0f, 1.0f);
            case 2: return vec4(0.7f, 0.0f, 0.0f, 1.0f);
        }
    }
    
    return color;
}
