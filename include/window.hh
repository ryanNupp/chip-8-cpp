#include <array>

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

class WindowHandler {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    bool is_running;

public:
    WindowHandler();
    ~WindowHandler();

    void draw_pixels(std::array<uint64_t, 32>);
    void poll_events();
    void rom_over_popup();
    bool get_run_status();
};