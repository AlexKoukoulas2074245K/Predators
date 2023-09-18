#include <iostream>
#include <SDL.h>

int main() {
    assert(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));
    
    assert(SDL_CreateWindow(
                "BaseProject",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                300, // Original = 160
                300, // Original = 144
                SDL_WINDOW_SHOWN));
    
    SDL_Event event;
    bool isRunning = true;
    while (isRunning)
    {
        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }
        }
    }
    
    return 0;
}
