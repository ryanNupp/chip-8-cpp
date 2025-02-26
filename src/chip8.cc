#include <algorithm>
#include <array>
#include <fstream>
#include <cstdint>
//#include <ctime>

#include "chip8.hh"

Chip8::Chip8(std::filesystem::path rom_file) {
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
    std::ifstream rom(rom_file, std::ios::binary);
    rom.read(reinterpret_cast<char*>(&memory[0x200]), memory.size()-0x200);
}

//////////////////////////////////////////////////
//                    Access                    //
//////////////////////////////////////////////////

std::array<uint64_t, 32> Chip8::get_display() {
    return display;
}

bool Chip8::end_of_mem() {
    return (program_counter > 0xFFF);
}

uint16_t Chip8::get_inst() {
    uint8_t first_byte = memory[program_counter++];
    uint8_t last_byte  = memory[program_counter++];
    return (first_byte << 8) | last_byte;
}


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
    std::fill(display.begin(), display.end(), 0);
}

// DXYN : Display instruction - draws a sprite to the screen
// x - register number that holds the X coordinate
// y - register number that holds the Y coordinate
// n - number of rows the sprite takes up (1 to 16 rows, represented with 0-15)
void Chip8::draw(uint8_t x, uint8_t y, uint8_t n) {
    size_t x_coord = var_regs[x] & 63;
    size_t y_coord = var_regs[y] & 31;
    uint64_t sprite_row;
    uint64_t collision_test;

    // initialize flag reg VF to 0
    var_regs[15] = 0;

    for (size_t i=0; i<n; i++) {
        sprite_row = memory[index_register+i];
        int shift = 56 - x_coord;
        if (shift >= 0) {
            sprite_row <<= shift;
        } else {
            sprite_row >>= shift * -1;
        }
        
        // apply changes, flag VF=1 if collision
        collision_test = display[y_coord+i] | sprite_row;
        display[y_coord+i] ^= sprite_row;
        if (display[y_coord+i] != collision_test) {
            var_regs[15] = 1;
        }
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
