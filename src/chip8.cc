#include <algorithm>
#include <array>
#include <fstream>
#include <cstdint>
//#include <ctime>
#include <iostream>

#include "chip8.hh"

Chip8::Chip8(std::filesystem::path rom_file) {
    // zero out all memory first
    std::fill(memory.begin(), memory.end(), 0);

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

int Chip8::get_timing() {
    return inst_per_sec;
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

void Chip8::jump_offset(uint16_t n) {
    int x = 0;
    if (jump_offset_vx) {
        x = (n & 0xF00) >> 8;
    }
    program_counter = n + var_regs[x];
}

int Chip8::subroutine_call(uint16_t n) {
    if (stack.size() > 15) {
        return 1;   // stack overflow
    }

    stack.push(program_counter);
    jump(n);
    return 0;
}

int Chip8::subroutine_return() {
    if (stack.empty()) {
        return 1;
    }

    jump(stack.top());
    stack.pop();
    return 0;
}

//////////////////////////////////////////////////
//                     Cond                     //
//////////////////////////////////////////////////

void Chip8::skip_equal_const(uint8_t x, uint8_t n) {
    if (var_regs[x] == n)
        program_counter += 2;
}

void Chip8::skip_not_equal_const(uint8_t x, uint8_t n) {
    if (var_regs[x] != n)
        program_counter += 2;
}

void Chip8::skip_equal(uint8_t x, uint8_t y) {
    if (var_regs[x] == var_regs[y])
        program_counter += 2;
}

void Chip8::skip_not_equal(uint8_t x, uint8_t y) {
    if (var_regs[x] != var_regs[y])
        program_counter += 2;
}

//////////////////////////////////////////////////
//                    Const                     //
//////////////////////////////////////////////////

// 6XNN : Register set constant
// x - register to set
// n - value to set the register to
void Chip8::set_reg_const(uint8_t x, uint8_t n) {
    var_regs[x] = n;
}

// 7XNN : Register add constant
// x - register to add to
// n - value to add to the register
void Chip8::add_reg_const(uint8_t x, uint8_t n) {
    var_regs[x] += n;
}

//////////////////////////////////////////////////
//                    Assig                     //
//////////////////////////////////////////////////

void Chip8::set_reg(uint8_t x, uint8_t y) {
    var_regs[x] = var_regs[y];
}

//////////////////////////////////////////////////
//                    Bitop                     //
//////////////////////////////////////////////////

// 8XY1
void Chip8::bitwise_or(uint8_t x, uint8_t y) {
    var_regs[x] |= var_regs[y];
}

// 8XY2
void Chip8::bitwise_and(uint8_t x, uint8_t y) {
    var_regs[x] &= var_regs[y];
}

// 8XY3
void Chip8::bitwise_xor(uint8_t x, uint8_t y) {
    var_regs[x] ^= var_regs[y];
}

// 8XY6
void Chip8::bitwise_shift_right(uint8_t x, uint8_t y) {
    if (shift_use_vy)
        var_regs[x] = var_regs[y];

    uint8_t flag = var_regs[x] & 0x01;
    var_regs[x] >>= 1;
    var_regs[0xF] = flag;
}

// 8XYE
void Chip8::bitwise_shift_left(uint8_t x, uint8_t y) {
    if (shift_use_vy)
        var_regs[x] = var_regs[y];

    uint8_t flag = (var_regs[x] & 0x80) >> 7;
    var_regs[x] <<= 1;
    var_regs[0xF] = flag;
}

//////////////////////////////////////////////////
//                    Math                      //
//////////////////////////////////////////////////

// 8XY4
void Chip8::add(uint8_t x, uint8_t y) {
    int test1 = var_regs[x];
    int test2 = var_regs[y];

    var_regs[x] += var_regs[y];

    // mark VF=1 if overflow occurred
    int flag = 0;
    if (var_regs[x] != test1 + test2)
        flag = 1;

    var_regs[0xF] = flag;
}

// 8XY5
void Chip8::subtract_x_y(uint8_t x, uint8_t y) {
    int test1 = var_regs[x];
    int test2 = var_regs[y];
    
    var_regs[x] -= var_regs[y];

    // mark VF=1 if no underflow occured
    int flag = 0;
    if (var_regs[x] == test1 - test2)
        flag = 1;

    var_regs[0xF] = flag;
}

// 8XY7
void Chip8::subtract_y_x(uint8_t x, uint8_t y) {
    int test1 = var_regs[x];
    int test2 = var_regs[y];
    
    var_regs[x] = var_regs[y] - var_regs[x];

    // mark VF=1 if no underflow occured
    int flag = 0;
    if (var_regs[x] == test2 - test1)
        flag = 1;

    var_regs[0xF] = flag;
}

//////////////////////////////////////////////////
//                     Mem                      //
//////////////////////////////////////////////////

// ANNN : Set index register
// n - value index register will be set to
void Chip8::set_index(uint16_t n) {
    index_register = n;
}

// FX1E : Add Vx to I - VF not affected
void Chip8::add_index(uint8_t x) {
    index_register += var_regs[x];
}

// FX29 : Set index register to memory location for sprite character that represents value in Vx
void Chip8::sprite_index(uint8_t x) {
    index_register = 0x050 + (var_regs[x] * 5);
}

// FX55 : register dump V0-Vx into memory, starting at location I
void Chip8::reg_dump(uint8_t x) {
    for (int i=0; i<=x; i++) {
        memory[index_register+i] = var_regs[i];
    }

    if (store_load_i_inc) {
        index_register += x + 1;
    }
}

// FX65 : register load V0-Vx from memory, starting at location I
void Chip8::reg_load(uint8_t x) {
    for (int i=0; i<=x; i++) {
        var_regs[i] = memory[index_register+i];
    }

    if (store_load_i_inc) {
        index_register += x + 1;
    }
}

//////////////////////////////////////////////////
//                    Rand                      //
//////////////////////////////////////////////////

// CXNN : Random
void Chip8::gen_rand(uint8_t x, uint8_t n) {
    var_regs[x] = rand() & n;
}

//////////////////////////////////////////////////
//                    KeyOp                     //
//////////////////////////////////////////////////

// EX9E : Skip if key currently pressed == Vx

// EXA1 : Skip if key currently pressed != Vx

// FX0A : Await input, grab & store key pressed

//////////////////////////////////////////////////
//                    Timer                     //
//////////////////////////////////////////////////

// FX15 : Set delay timer
void Chip8::set_delay(uint8_t x) {
    delay_timer = var_regs[x];
}

// FX07 : Get delay timer
void Chip8::get_delay(uint8_t x) {
    var_regs[x] = delay_timer;
}

//////////////////////////////////////////////////
//                    Sound                     //
//////////////////////////////////////////////////

// FX18 : Set sound timer
void Chip8::set_sound(uint8_t x) {
    sound_timer = var_regs[x];
}

//////////////////////////////////////////////////
//                    BCD                       //
//////////////////////////////////////////////////

// FX33 : Binary-coded decimal conversion
void Chip8::bcd(uint8_t x) {
    memory[index_register]   = var_regs[x] / 100;
    memory[index_register+1] = (var_regs[x] % 100) / 10;
    memory[index_register+2] = var_regs[x] % 10;
}