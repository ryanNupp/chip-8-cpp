#include <iostream>
#include <filesystem>
#include <unistd.h>

#include "window.hh"
#include "chip8.hh"

// instruction decode macros
#define OP(ins) ((ins & 0xF000) >> 12)
#define X(ins) ((ins & 0x0F00) >> 8)
#define Y(ins) ((ins & 0x00F0) >> 4)
#define N(ins) (ins & 0x000F)
#define NN(ins) (ins & 0x00FF)
#define NNN(ins) (ins & 0x0FFF)

int main(int argc, char ** argv) {
    // check if file arg is present
    if (argc != 2) {
        std::cout << "Usage: chip8emu /path/to/rom\n";
        return 1;
    }

    // open rom
    std::filesystem::path rom_file{argv[1]};
    if (!std::filesystem::exists(rom_file)) {
        printf("ERROR: Incorrect file path\n");
        return 1;
    }

    Chip8 chip8(rom_file);
    WindowHandler w{};

    // chip8.disp_clear();
    // chip8.set_const(4, 2);
    // chip8.set_const(5, 2);
    // chip8.set_index(0x50);
    // chip8.draw(4, 5, 5);
    // w.draw_pixels(chip8.get_display());

    // chip8.jump(0x50);
    // std::cout << chip8.get_curr_inst() << "\n";

    while (w.get_run_status()) {
        w.poll_events();

        // instruction cycle
        uint16_t inst = chip8.get_inst();
        switch (OP(inst)) {
        case 0x0:
            chip8.disp_clear();
            w.draw_pixels(chip8.get_display());
            break;
        case 0x1:    chip8.jump(NNN(inst));                 break;
        case 0x2:    break;
        case 0x3:    break;
        case 0x4:    break;
        case 0x5:    break;
        case 0x6:    chip8.set_const(X(inst), NN(inst));    break;
        case 0x7:    chip8.add_const(X(inst), NN(inst));    break;
        case 0x8:    break;
        case 0x9:    break;
        case 0xA:    break;
        case 0xB:    break;
        case 0xC:    break;
        case 0xD:
            chip8.draw(X(inst), Y(inst), N(inst));
            w.draw_pixels(chip8.get_display());
            break;
        case 0xE:    break;
        case 0xF:    break;
        }
        
        if (chip8.end_of_mem())
            w.rom_over_popup();

        // wait and stuff
        sleep(1);
    }
    
    return 0;
}