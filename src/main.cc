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
#include <filesystem>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>

#include <iostream>

#include "tinyfiledialogs.h"

#include "window.hh"
#include "chip8.hh"

// instruction decode macros
#define OP(ins) ((ins & 0xF000) >> 12)
#define X(ins) ((ins & 0x0F00) >> 8)
#define Y(ins) ((ins & 0x00F0) >> 4)
#define N(ins) (ins & 0x000F)
#define NN(ins) (ins & 0x00FF)
#define NNN(ins) (ins & 0x0FFF)

void instruction_cycle(Chip8&, WindowHandler&);

int main(int argc, char ** argv) {
    tinyfd_messageBox(
        "Chip8c++",
        "Please select a Chip 8 ROM to run",
        "ok",
        "option",
        0
    );
    char const* filter_patterns[1] = {"*.ch8"};
    char const* filepath = tinyfd_openFileDialog(
        "Select ROM",
        "./",
        1,
        filter_patterns,
        "*.ch8",
        0
    );
    if (!filepath) {
        tinyfd_messageBox(
			"Error",
			"No ROM selected",
			"ok",
			"error",
			1
        );
		return 1;
    }


    std::filesystem::path rom{filepath};
    if (!std::filesystem::exists(rom) || rom.extension() != ".ch8") {
        tinyfd_messageBox(
			"Error",
			"Invalid file! Select a valid Chip 8 ROM file. (.ch8)",
			"ok",
			"error",
			1
        );
		return 1;
    }
    

    Chip8 chip8(filepath);
    WindowHandler w{};

    std::chrono::time_point time_next = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds inst_time{1000000000 / chip8.get_timing()};

    while (w.get_run_status()) {
        w.poll_events();
        
        // std::array<bool, 16> keys = w.get_keys();
        // for (bool key : keys) {
        //     std::cout << key << " ";
        // }
        // std::cout << "\n";

        if (std::chrono::high_resolution_clock::now() >= time_next) {
            instruction_cycle(chip8, w);
            time_next += inst_time;
        }
    }
    
    return 0;
}


void instruction_cycle(Chip8& chip8, WindowHandler& w) {
    uint16_t inst = chip8.get_inst();
    switch (OP(inst)) {
    case 0x0:
        switch (NNN(inst)) {
        case 0x0E0:    chip8.disp_clear();                               break;
        case 0x0EE:    chip8.subroutine_return();                        break;
        }
        break;
    case 0x1:    chip8.jump(NNN(inst));                                  break;
    case 0x2:    chip8.subroutine_call(NNN(inst));                       break;
    case 0x3:    chip8.skip_equal_const(X(inst), NN(inst));              break;
    case 0x4:    chip8.skip_not_equal_const(X(inst), NN(inst));          break;
    case 0x5:    chip8.skip_equal(X(inst), Y(inst));                     break;
    case 0x6:    chip8.set_reg_const(X(inst), NN(inst));                 break;
    case 0x7:    chip8.add_reg_const(X(inst), NN(inst));                 break;
    case 0x8:
        switch (N(inst)) {
        case 0x0:    chip8.set_reg(X(inst), Y(inst));                    break;
        case 0x1:    chip8.bitwise_or(X(inst), Y(inst));                 break;
        case 0x2:    chip8.bitwise_and(X(inst), Y(inst));                break;
        case 0x3:    chip8.bitwise_xor(X(inst), Y(inst));                break;
        case 0x4:    chip8.add(X(inst), Y(inst));                        break;
        case 0x5:    chip8.subtract_x_y(X(inst), Y(inst));               break;
        case 0x6:    chip8.bitwise_shift_right(X(inst), Y(inst));        break;
        case 0x7:    chip8.subtract_y_x(X(inst), Y(inst));               break;
        case 0xE:    chip8.bitwise_shift_left(X(inst), Y(inst));         break;
        }
        break;
    case 0x9:    chip8.skip_not_equal(X(inst), Y(inst));                 break;
    case 0xA:    chip8.set_index(NNN(inst));                             break;
    case 0xB:    chip8.jump_offset(NNN(inst));                           break;
    case 0xC:    chip8.gen_rand(X(inst), NN(inst));                      break;
    case 0xD:    chip8.draw(X(inst), Y(inst), N(inst));                  break;
    case 0xE:
        switch (NN(inst)) {
        case 0x9E:
            if (w.keys[chip8.get_var_reg(X(inst))])
                chip8.increment_pc();
            break;
        case 0xA1:
            if (!w.keys[chip8.get_var_reg(X(inst))])
                chip8.increment_pc();
            break;
        }
        break;
    case 0xF:
        switch (NN(inst)) {
        case 0x07:    chip8.get_delay(X(inst));                          break;
        case 0x0A:
            switch (chip8.block_state) {
            case 0:
                chip8.block_state = 1;
                chip8.decrement_pc();
                break;
            case 1:
                if (w.last_key_down >= 0 && w.keys[w.last_key_down]) {
                    chip8.set_reg_const(X(inst), w.last_key_down);
                    std::cout << "REG " << X(inst) << " - " << int(chip8.get_var_reg(X(inst))) << "\n";
                    chip8.block_state = 2;
                }
                chip8.decrement_pc();
                break;
            case 2:
                if (w.last_key_down == -1) {
                    chip8.block_state = 0;
                    std::cout << "(end)REG " << X(inst) << " - " << int(chip8.get_var_reg(X(inst))) << "\n";
                }
                else {
                    chip8.decrement_pc();
                }
                break;
            }
            break;
        case 0x15:    chip8.set_delay(X(inst));                          break;
        case 0x18:    chip8.set_sound(X(inst));                          break;
        case 0x1E:    chip8.add_index(X(inst));                          break;
        case 0x29:    chip8.sprite_index(X(inst));                       break;
        case 0x33:    chip8.bcd(X(inst));                                break;
        case 0x55:    chip8.reg_dump(X(inst));                           break;
        case 0x65:    chip8.reg_load(X(inst));                           break;
        }
        break;
    }

    if (OP(inst) == 0xD || inst == 0x00E0)
        w.draw_pixels(chip8.get_display());

    if (chip8.end_of_mem())
        w.popup("End of memory", "The program counter is pointing past end of the memory.");
}