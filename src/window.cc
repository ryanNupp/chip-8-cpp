/*
CREDITS:

This software makes use of
- SDL3
- tinyfiledialogs

------------------------------------------------------------------------------
zlib License

(C) 2025 Ryan Nuppenau

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#include "window.hh"

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_messagebox.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_dialog.h"
#include <csignal>

WindowHandler::WindowHandler() {
    SDL_Init(SDL_INIT_VIDEO);

    is_running = true;

    window = SDL_CreateWindow(
        "Chip8",
        1024,
        512,
        0
    );
    renderer = SDL_CreateRenderer(
        window,
        NULL
    );
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        64,
        32
    );

    // texture scale mode (nearest pixel mode, don't blur pixels)
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    SDL_SetRenderTarget(renderer, texture);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, NULL);
}

WindowHandler::~WindowHandler() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void WindowHandler::draw_pixels(std::array<uint64_t, 32> chip8_display) {
    std::array<uint32_t, 2048> display_texture;
    std::array<uint32_t, 2048>::iterator display_pixel = display_texture.begin();

    for (uint64_t row : chip8_display) {
        for (uint64_t mask = 0x8000000000000000; mask > 0; mask >>= 1) {
            // check current display pixel, convert to appropriate texture pixel to match
            *(display_pixel++) = ((row & mask) > 0) ? 0xFFFFFFFF : 0x000000FF;
        }
    }

    SDL_UpdateTexture(
        texture,
        NULL,
        display_texture.data(),
        64 * sizeof(uint32_t)
    );
    SDL_RenderTexture(
        renderer,
        texture,
        NULL,
        NULL);
    SDL_RenderPresent(renderer);
}

void WindowHandler::poll_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            is_running = false;
            break;
        }
    }
}

void WindowHandler::popup(std::string title, std::string message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), message.c_str(), window);
    is_running = false;
}

bool WindowHandler::get_run_status() {
    return is_running;
}

void WindowHandler::open_file() {
    SDL_ShowOpenFileDialog(
        [](void* userdata, const char* const* filelist, int filter) {
            userdata = &filelist;
        },
        NULL,
        window,
        NULL,
        0,
        NULL,
        false
    );
}