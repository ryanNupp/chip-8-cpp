#pragma once

#include <cstdint>
#include <array>
#include <filesystem>
#include <stack>
#include <string>

class Chip8 {
private:
    // display  -  64 * 32 pixels
    std::array<uint64_t, 32> display;

    // memory, registers, & counter
    std::array<uint8_t, 4096> memory;
    std::array<uint8_t, 16> var_regs;
    uint16_t index_register;
    uint16_t program_counter;

    // timers
    uint8_t delay_timer;
    uint8_t sound_timer;

    // stack
    std::stack<uint16_t> stack;

    // configuration
    int inst_per_sec;
    bool shift_use_vy;
    bool jump_offset_vx;
    bool store_load_i_inc;

public:
    Chip8(std::filesystem::path);

    // access
    std::array<uint64_t, 32> get_display();
    bool end_of_mem();
    uint16_t get_inst();
    int get_timing();

    // chip 8 configuration
    void config_timing(int);
    void config_shift(bool);
    void config_jump_offset(bool);
    void config_store_load_inc(bool);

/****************/
/*   display    */
/****************/
    
    
    void disp_clear();
    
    // DXYN : Display instruction - draws a sprite to the screen
    // x - register number that holds the X coordinate
    // y - register number that holds the Y coordinate
    // n - number of rows the sprite takes up (1 to 16 rows, represented with 0-15)
    void draw(uint8_t, uint8_t, uint8_t);

/****************/
/*     flow     */
/****************/
    void jump(uint16_t);
    void jump_offset(uint16_t);
    int  subroutine_call(uint16_t);
    int  subroutine_return();

    // cond
    void skip_equal_const(uint8_t, uint8_t);
    void skip_not_equal_const(uint8_t, uint8_t);
    void skip_equal(uint8_t, uint8_t);
    void skip_not_equal(uint8_t, uint8_t);

    // const
    void set_reg_const(uint8_t, uint8_t);
    void add_reg_const(uint8_t, uint8_t);

    // assig
    void set_reg(uint8_t, uint8_t);

    // bitop
    void bitwise_or(uint8_t, uint8_t);
    void bitwise_and(uint8_t, uint8_t);
    void bitwise_xor(uint8_t, uint8_t);
    void bitwise_shift_right(uint8_t, uint8_t);
    void bitwise_shift_left(uint8_t, uint8_t);

    // math
    void add(uint8_t, uint8_t);
    void subtract_x_y(uint8_t, uint8_t);
    void subtract_y_x(uint8_t, uint8_t);

    // mem
    void set_index(uint16_t);
    void add_index(uint8_t);
    void sprite_index(uint8_t);
    void reg_dump(uint8_t);
    void reg_load(uint8_t);

    // rand
    void gen_rand(uint8_t, uint8_t);

    // keyop
    // -
    // -
    // -

    // timer
    void set_delay(uint8_t);
    void get_delay(uint8_t);

    // sound
    void set_sound(uint8_t);

    // bcd
    void bcd(uint8_t);

};