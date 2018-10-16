#include <iostream>
#include <chrono>
#include <thread>
#include "chip8.hpp"
#include <SDL2/SDL.h>

chip8 myChip8;

int main(int /*argc*/ , char const * /*argv*/ [])
{
    std::cout << "Testing\n";
    
    myChip8.initialize();

    for(;;) {
        auto start = std::chrono::system_clock::now();
        
        // Emulate one cycle
        myChip8.emulateCycle();

        auto end = std::chrono::system_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/60) - std::chrono::duration_cast<std::chrono::milliseconds>(end-start));
    }
    return 0;
}
