#include <iostream>
#include <filesystem>
#include <semaphore>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>

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
        "dialog",
        "option",
        0
    );

    char const* filter_patterns[1] = {"*.ch8"};

    char* filepath = tinyfd_openFileDialog(
        "Select ROM",
        "./",
        1,
        filter_patterns,
        "Chip 8 ROM files",
        0
    );

    Chip8 chip8(filepath);
    WindowHandler w{};    


    std::chrono::time_point time_next = std::chrono::high_resolution_clock::now();
    //std::chrono::nanoseconds inst_time{1000000000 / 700};

    while (w.get_run_status()) {
        w.poll_events();

        if (std::chrono::high_resolution_clock::now() >= time_next) {
            instruction_cycle(chip8, w);
            time_next += std::chrono::nanoseconds{1000000000 / 700};
        }
    }
    
    return 0;
}


void instruction_cycle(Chip8& chip8, WindowHandler& w) {
    uint16_t inst = chip8.get_inst();
    switch (OP(inst)) {
    case 0x0:
        switch (NNN(inst)) {
        case 0x0E0:    chip8.disp_clear();                         break;
        case 0x0EE:    chip8.subroutine_return();                  break;
        }
        break;
    case 0x1:    chip8.jump(NNN(inst));                            break;
    case 0x2:    chip8.subroutine_call(NNN(inst));                 break;
    case 0x3:    chip8.skip_equal_const(X(inst), NN(inst));        break;
    case 0x4:    chip8.skip_not_equal_const(X(inst), NN(inst));    break;
    case 0x5:    chip8.skip_equal(X(inst), Y(inst));               break;
    case 0x6:    chip8.set_reg_const(X(inst), NN(inst));           break;
    case 0x7:    chip8.add_reg_const(X(inst), NN(inst));           break;
    case 0x8:
        switch (N(inst)) {
        case 0x0:    chip8.set_reg(X(inst), Y(inst));              break;
        case 0x1:    chip8.bitwise_or(X(inst), Y(inst));           break;
        case 0x2:    chip8.bitwise_and(X(inst), Y(inst));          break;
        case 0x3:    chip8.bitwise_xor(X(inst), Y(inst));          break;
        case 0x4:    chip8.add(X(inst), Y(inst));                  break;
        case 0x5:    chip8.subtract_x_y(X(inst), Y(inst));         break;
        case 0x6:    chip8.bitwise_shift_right(X(inst), Y(inst));  break;
        case 0x7:    chip8.subtract_y_x(X(inst), Y(inst));         break;
        case 0xE:    chip8.bitwise_shift_left(X(inst), Y(inst));   break;
        }
        break;
    case 0x9:    chip8.skip_not_equal(X(inst), Y(inst));           break;
    case 0xA:    chip8.set_index(NNN(inst));                       break;
    case 0xB:    chip8.jump_offset(NNN(inst));                     break;
    case 0xC:    chip8.gen_rand(X(inst), NN(inst));                break;
    case 0xD:    chip8.draw(X(inst), Y(inst), N(inst));            break;
    case 0xE:
        switch (NN(inst)) {
        case 0x9E:  /* skip if key pressed == Vx (lowest four bits) */  break;
        case 0xA1:  /* skip if key pressed != Vx (lowest four bits) */  break;
        }
        break;
    case 0xF:
        switch (NN(inst)) {
        case 0x07:    chip8.get_delay(X(inst));                    break;
        case 0x0A:    /* key op get key */   break;
        case 0x15:    chip8.set_delay(X(inst));                    break;
        case 0x18:    chip8.set_sound(X(inst));                    break;
        case 0x1E:    chip8.add_index(X(inst));                    break;
        case 0x29:    chip8.sprite_index(X(inst));                 break;
        case 0x33:    chip8.bcd(X(inst));                          break;
        case 0x55:    chip8.reg_dump(X(inst));                     break;
        case 0x65:    chip8.reg_load(X(inst));                     break;
        }
        break;
    }

    if (OP(inst) == 0xD || inst == 0x00E0)
        w.draw_pixels(chip8.get_display());

    if (chip8.end_of_mem())
        w.popup("End of memory", "The program counter is pointing past end of the memory.");
}