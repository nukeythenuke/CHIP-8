#include "chip8.hpp"
#include <iostream>
#include <random>
#include <algorithm>

void chip8::initialize() {
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;
}

void chip8::emulateCycle() {
    // Update Timers
    if (delay_timer > 0) { --delay_timer; }
    if (sound_timer > 0) { if (--sound_timer == 0) {std::cout << "BEEP!\n";} }
    
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Decode and execute opcode
    switch(opcode & 0xF000) {
        case 0x0000:
            switch(opcode & 0x000F) {
                case 0x0000: // Clears the screen
                    std::fill(gfx.begin(), gfx.end(), 0);
                    drawFlag = true;
                    pc += 2;
                    break;
                
                case 0x000E: // Returns from a subroutine
                    pc = stack[--sp];
                    pc += 2;
                    break;
                
                default:
                    std::cout << "Undefined opcode: " << std::hex << std::showbase << opcode << '\n';
                    break;
            }
            break;
        
        case 0x1000: // 1NNN - Jump to address NNN
            pc = opcode & 0x0FFF;
            break;

        case 0x2000: // 2NNN - Call subroutine at NNN
            stack[sp++] = pc;
            pc = opcode & 0x0FFF;
            break;
        
        case 0x3000: // 3XNN - Skips the next instruction if VX == NN
            if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;

        case 0x4000: // 4XNN - Skips the next instruction if VX != NN
            if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        
        case 0x5000: // 5XY0 - Skips the next instruction if VX == VY
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        
        case 0x6000: // 6XNN - Sets VX to NN
            V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
            pc += 2;
            break;

        case 0x7000: // 7XNN - Adds NN to VX
            V[(opcode & 0x0F00) >> 8] += V[opcode & 0x00FF];
            pc += 2;
            break;
        
        case 0x8000: // 8XY_
            switch(opcode & 0x000F) {
                case 0x0000: // 8XY0 - Sets VX to the value of VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                
                case 0x0001: // 8XY1 - Sets VX to VX or VY (Bitwise OR operation)
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0002: // 8XY2 - Sets VX to VX and VY (Bitwise AND operation)
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0003: // 8XY3 - Sets VX to VX xor VY
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0004: // 8XY4 - Adds VY to VX. if(result > 255) {VF = 1} else {VF = 0}
                    if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00) >> 8] += V[opcode & 0x00F0 >> 4];
                    pc += 2;
                    break;
                
                case 0x0005: // 8XY5 - VY is subtracted from VX. VF is set to 0 when there is a borrow, and 1 when there isn't
                    if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                
                case 0x0006: // 8XY6 - Stores the least significant bit of VX in VF and then shifts VX to the right by 1
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                    break;
                
                case 0x0007: // 8XY7 - Sets VX to VY minux VX. VF is set to 0 when there is a borrow, and 1 when there isn't
                    if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) {
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                
                case 0x000E: // 8XYE - Stores the most significant bit of VX in VF and then shifts VX to the left by 1
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;
                
                default:
                    std::cout << "Undefined opcode " << std::hex << std::showbase << opcode << '\n';
            }
            break;
        
        case 0x9000: // 9XY0 - Skips the next instruction if VX != VY
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        
        case 0xA000: // ANNN - Sets I to address NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        
        case 0xB000: // BNNN - Jumps to the address NNN + V0
            pc = V[0x0] + (opcode & 0x0FFF);
            break;
        
        case 0xC000: // CXNN - Sets VX to the result of a bitwise and operation on a random number and NN
            V[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
            pc += 2;
            break;

        case 0xD000: // DXYN - Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
            {
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short width = 0x8;
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;
            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = memory[I + yline];
                for (int xline = 0; xline < width; xline++) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        if (gfx[(x + xline + ((y + yline) * 64))] == 1) {
                            V[0xF] = 1;
                        }
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }
            drawFlag = true;
            pc += 2;
            }
            break;
        
        case 0xE000: // EX__
            switch (opcode & 0x00FF) {
                case 0x009E: // EX9E - Skips next instruction if key VX is pressed
                    if (key[V[(opcode & 0x0F00) >> 8]]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                
                case 0x00A1: // EXA1 - Skips next instruction if key VX is not pressed
                    if (!key[V[(opcode & 0x0F00) >> 8]]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                
                default:
                    std::cout << "Undefined opcode: " << std::hex << std::showbase << opcode << '\n';
            }
            break;

        case 0xF000: // 0xFX__
            switch(opcode & 0x00FF) {
                case 0x007: // FX07 - Sets VX to the value of delay timer
                    V[(opcode & 0x0F00) >> 8] = delay_timer;
                    pc += 2;
                    break;
                
                case 0x000A: // FX0A - A key press is awaited, then stored in VX
                    {
                    bool key_pressed = false;
                    //V[(opcode & 0x0F00) >> 8] = std::find_if_not(key.begin(), key.end(), [&key_pressed](auto i){ return key_pressed = i != 0; }) - key.begin();
                    std::for_each(key.begin(), key.end(), [opcode = opcode, &key_pressed, &V = V](auto i) {
                        if(i != 0) {
                            V[(opcode & 0x0F00) >> 8] = i;
                            key_pressed = true;
                        }
                    });
                    if(!key_pressed) {
                        return;
                    }
                    pc += 2;
                    }
                    break;
                
                case 0x0015: // FX15 - Sets the delay timer to VX
                    delay_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                
                case 0x0018: // FX18 - Sets the sound timer to VX
                    sound_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                
                case 0x001E: // FX1E - Adds VX to I. VF = 1 when overflow, 0 when not
                    if (0xFFF - I < V[(opcode & 0x0F00) >> 8]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                
                case 0x0029: // FX29 - Sets I to the location of the sprite for the character in VX
                    I = V[(opcode & 0x0F00) >> 8] * 0x5;
                    pc += 2;
                    break;

                case 0x0033: // FX33 - Stores VX in I, I+1 and I+2
                    memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
                    pc += 2;
                    break;
                
                case 0x0055: // FX55 - Store V0 to VX in memory starting at address I
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
                        memory[I + i] = V[i];
                    }
                    break;
                
                case 0x0065: // FX55 - Fill V0 to VX from memory starting at address I
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
                        V[i] = memory[I + i];
                    }
                    break;
                
                default:
                    std::cout << "Undefined opcode: " << std::hex << std::showbase << opcode << '\n';
            }
            break;

        default:
            std::cout << "Undefined opcode: " << std::hex << std::showbase << opcode << '\n';
    }
}