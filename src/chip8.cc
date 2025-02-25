#include <algorithm>
#include <array>
#include <fstream>
#include <cstdint>
#include <ctime>

#include "chip8.hh"

Chip8::Chip8(std::string rom_filepath) {
    // initialize display
    display.resize(disp_w * disp_h);

    uint8_t font[] = {
        0x60, 0xB0, 0xD0, 0x90, 0x60,   // 0
        0x20, 0x60, 0x20, 0x20, 0x70,   // 1
        0x60, 0x90, 0x20, 0x40, 0xF0,   // 2
        0xE0, 0x10, 0x60, 0x10, 0xE0,   // 3
        0x20, 0x60, 0xA0, 0xF0, 0x20,   // 4
        0xF0, 0x80, 0xE0, 0x10, 0xE0,   // 5
        0x60, 0x80, 0xE0, 0x90, 0x60,   // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
        0x60, 0x90, 0x60, 0x90, 0x60,   // 8
        0x60, 0x90, 0x70, 0x10, 0x60,   // 9
        0x60, 0x90, 0xF0, 0x90, 0x90,   // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
        0x70, 0x80, 0x80, 0x80, 0x70,   // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
        0xF0, 0x80, 0xE0, 0x80, 0xF0,   // E
        0xF0, 0x80, 0xE0, 0x80, 0x80    // F
    };

    // initialize font in memory
    for (int i=0; i<80; i++) {
        memory[i+0x50] = font[i];
    }

    // initialize program counter to 0x200
    program_counter = 0x200;

    // config defaults
    inst_per_sec = 700;

    // srand
    srand(time(NULL));

    // read & load rom to memory (starting @ address 0x200)
    std::ifstream rom(rom_filepath, std::ios::binary);
    rom.read(reinterpret_cast<char*>(memory.data()+0x200), 3584);
}

Chip8::~Chip8() {}

//////////////////////////////////////////////////
//                Configurations                //
//////////////////////////////////////////////////

void Chip8::config_timing(int num) {
    inst_per_sec = num;
}

void Chip8::config_shift(bool set) {
    shift_use_vy = set;
}

void Chip8::config_jump_offset(bool set) {
    jump_offset_vx = set;
}

void Chip8::config_store_load_inc(bool set) {
    store_load_i_inc = set;
}

//////////////////////////////////////////////////
//                   Display                    //
//////////////////////////////////////////////////

void Chip8::disp_clear() {
    std::fill(display.begin(), display.end(), 0x000000FF);
}

void Chip8::draw(uint8_t x, uint8_t y, uint8_t n) {
    int x_pos = var_regs[x] & 63;
    int y_pos = var_regs[y] & 31;

    int mem_addr = index_register;
    while (y_pos < y_pos + n && y_pos < 32) {
        int curr_bit = 0b1000'0000;
        while (x_pos < x_pos + 8 && x_pos < 64) {
            int display_index = (x & 63) + (y & 31) * 64;
            if ((mem_addr & curr_bit) > 0) {
                if (display[display_index] == 0xFFFFFFFF) {
                    display[display_index] = 0x000000FF;
                }
                else {
                    display[display_index] = 0xFFFFFFFF;
                }
            }
            curr_bit >>= 1;
            x_pos++;
        }
        mem_addr++;
        y_pos++;
    }

    
}

//////////////////////////////////////////////////
//                     Flow                     //
//////////////////////////////////////////////////

void Chip8::jump(uint16_t n) {
    program_counter = n;
}

//////////////////////////////////////////////////
//                     Cond                     //
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//                    Const                     //
//////////////////////////////////////////////////

// 6XNN : Register set constant
// x - register to set
// n - value to set the register to
void Chip8::set_const(uint8_t x, uint8_t n) {
    var_regs[x] = n;
}

// 7XNN : Register add constant
// x - register to add to
// n - value to add to the register
void Chip8::add_const(uint8_t x, uint8_t n) {
    var_regs[x] += n;
}

//////////////////////////////////////////////////
//                    Assig                     //
//////////////////////////////////////////////////

void Chip8::set(uint8_t x, uint8_t y) {
    var_regs[x] = var_regs[y];
}

//////////////////////////////////////////////////
//                    Bitop                     //
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//                    Math                      //
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//                     Mem                      //
//////////////////////////////////////////////////

// ANNN : Set index register
// n - value index register will be set to
void Chip8::set_index(uint16_t n) {
    index_register = n;
}

//////////////////////////////////////////////////
//                    Rand                      //
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//                    KeyOp                     //
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//                    Timer                     //
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//                    Sound                     //
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//                    BCD                       //
//////////////////////////////////////////////////
