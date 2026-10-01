#include <SDL.h>
#include <stdlib.h>

#include "lsd/lsd_main.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    const lsd_config_t *lsd_config = lsd_default_config();
    const int rc = lsd_main(lsd_config);

    SDL_Quit();
    return rc;
}
