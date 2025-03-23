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
#pragma once

#include <array>
#include <string>
#include <filesystem>

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

class WindowHandler {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    bool is_running;

public:
    std::array<bool, 16> keys;
    int last_key_down;

    WindowHandler();
    ~WindowHandler();

    void open_file();
    void draw_pixels(std::array<uint64_t, 32>);
    void poll_events();
    bool key_is_pressed(uint8_t);
    int  get_curr_key();
    void popup(std::string, std::string);
    bool get_run_status();
};