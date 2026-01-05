# CHIP-8 emulator

This is a simple CHIP-8 emulator written in C.

CHIP-8 is an interpreted programming language originally used on early computers and gaming consoles, designed for creating simple games.

This implementation follows the [Cowgod's Chip-8 Technical Reference v1.0](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)

### Requirements

- GCC or any C compiler
- SDL2 library for graphics and input handling
- SDL2_ttf library for font rendering
- Make utility

### Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/yuufw/chip8.git
   cd chip8
   ```

2. Install SDL2 and SDL2_ttf libraries:
   - On Ubuntu/Debian:

     ```bash
     sudo apt-get install libsdl2-dev libsdl2-ttf-dev
     ```

   - On macOS using Homebrew:

     ```bash
     brew install sdl2 sdl2_ttf
     ```

3. Compile the emulator:

   ```bash
   make
   ```

### Usage

To run a CHIP-8 program, use the following command:

```bash
./chip8 path/to/your/program.ch8
```

Replace `path/to/your/program.ch8` with the actual path to your CHIP-8 program file.

### Controls

The CHIP-8 uses the following key mapping for input:

```c
Original CHIP-8 Keypad       Mapped Keyboard Keys
+---+---+---+---+            +---+---+---+---+
| 1 | 2 | 3 | C |            | 1 | 2 | 3 | 4 |
+---+---+---+---+            +---+---+---+---+
| 4 | 5 | 6 | D |            | Q | W | E | R |
+---+---+---+---+    =>      +---+---+---+---+
| 7 | 8 | 9 | E |            | A | S | D | F |
+---+---+---+---+            +---+---+---+---+
| A | 0 | B | F |            | Z | X | C | V |
+---+---+---+---+            +---+---+---+---+  
```
