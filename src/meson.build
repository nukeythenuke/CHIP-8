project('CHIP-8', 'cpp')
sdldep = dependency('sdl2')
executable('debug', 'main.cpp', 'chip8.cpp', cpp_args: '-Og', dependencies : sdldep)
executable('release', 'main.cpp', 'chip8.cpp', cpp_args: '-O3', dependencies : sdldep, build_by_default: false)