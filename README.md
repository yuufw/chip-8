# CHIP-8 emulator

This is a simple CHIP-8 emulator written in C.

CHIP-8 is an interpreted programming language originally used on early computers and gaming consoles, designed for creating simple games.

This implementation follows the [Cowgod's Chip-8 Technical Reference v1.0](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)

---

### Requirements

- GCC or any C compiler
- SDL2 library for graphics and input handling
- SDL2_ttf library for font rendering
- Make utility

---

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

   - On Fedora

     ```bash
     sudo dnf install -y gcc make pkgconf-pkg-config \
     SDL2-devel SDL2_ttf-devel
     ```

   - On Arch Linux / Manjaro

     ```bash
     sudo pacman -S --needed base-devel pkgconf sdl2 sdl2_ttf
     ```

   - On openSUSE (Tumbleweed / Leap)

     ```bash
     sudo zypper install -y gcc make pkg-config \
     SDL2-devel SDL2_ttf-devel
     ```

   - On Alpine Linux

     ```bash
     sudo apk add --no-cache build-base pkgconf \
     sdl2-dev sdl2_ttf-dev
     ```
   - On macOS using Homebrew:

     ```bash
     brew install sdl2 sdl2_ttf
     ```

3. Compile the emulator:

**Targets**

- `make sdl`  
  Build the SDL frontend (default).

- `make headless`  
  Build without SDL (useful for CI / tests).

- `make debug`  
  SDL build with debug symbols and no optimizations (GDB-friendly).

- `make release`  
  SDL build optimized for speed (`-Ofast`, optional LTO).

- `make release_size`  
  SDL build optimized for size (`-Os`, optional LTO).

**Clean / format**

- `make clean`
  Remove build outputs.

- `make format`
  Run clang-format on src/*.c and inc/*.h.

---

### Usage

SDL binaries are placed under `./bin/`, following the profiles:

- `bin/chip8 (sdl)`

- `bin/chip8-debug`

- `bin/chip8-release`

- `bin/chip8-release-size`

To run a CHIP-8 program, use the following command:

```bash
./bin/chip8-<profile>
```

Replace `profile` based on the profile you selected in the make command.

When the program opens, two windows will appear on the screen: one with the emulator's register debugging and another with a menu where you can select one of the pre-installed ROMs, which are located in `./roms/`, or load your own by typing the path.

---

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

---
