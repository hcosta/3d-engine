#include <SDL2/SDL.h>

int main(int argc, char *args[])
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Quit();
    printf("SDL loaded!");
    return 0;
}

// #include <stdio.h>

// int main()
// {
//     printf("Hello, world!");
//     return 0;
// }