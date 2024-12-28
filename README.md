# NES Emulator in C++

This NES emulator, written in C++, serves as a learning project to explore the inner workings of the Nintendo Entertainment System. It integrates a cycle-accurate MOS 6502 CPU, the Picture Processing Unit (PPU), and will soon feature the Audio Processing Unit (APU) to replicate the core functionality of the NES hardware. The aim of this project is to deepen the understanding of emulation concepts by recreating the essential components of the NES system and executing games in a controlled, software-driven environment.

# Features Implemented

### CPU (MOS 6502)
- [x] Official opcodes
- [x] Cycle accuracy
- [ ] Dummy reads/writes (some implemented)

### PPU
- [x] NTSC support
- [x] PAL support

### APU
- [ ] Not implemented yet (coming soon)

### Controllers
- [x] Keyboard input
- [x] Zapper support

### iNES Format Support

### Mappers
- [x] Mapper 0 (NROM)
- [x] Mapper 2 (UxROM)
- [x] Mapper 1 (MMC1)
- [x] Mapper 3 (CNROM)
- [x] Mapper 7 (AxROM)
- [ ] Mapper 4 (MMC3) (coming soon)

# Visual Examples

![image](https://github.com/user-attachments/assets/0a1519d6-324f-4245-a658-5448ebea2d89)

![image](https://github.com/user-attachments/assets/d78a2bef-d4bf-4c3d-a728-17a562d89aa4)

![image](https://github.com/user-attachments/assets/f555db1d-a293-4058-a9b5-f3efbf6c6f55)

https://github.com/user-attachments/assets/e5b3fd82-0645-4a24-a55c-0e3d55a86c56




# Controller Setup

| Button      | Key  | Description                            |
|-------------|--------------|----------------------------------------|
| A           | 8 (Numpad)           | Jump or primary action button          |
| B           | 7  (Numpad)          | Secondary action button                |
| Start       | 5   (Numpad)         | Start the game or pause                |
| Select      | 4   (Numpad)         | Select item in the menu                |
| D-Pad (Up)  | W            | Move up in the game                    |
| D-Pad (Down)| S            | Move down in the game                  |
| D-Pad (Left)| A            | Move left in the game                  |
| D-Pad (Right)| D           | Move right in the game                 |

## Zapper (Light Gun for the NES)

For games that support the Zapper, simply click on "Settings" and then enable the Zapper. Once enabled, you can use it by left-clicking your mouse. 

# Dependencies

This project relies on the following libraries to build and run correctly:

### [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.10)
A powerful multimedia library used for handling graphics, input, and audio. It provides the foundation for rendering and user input in this emulator.

### [ImGui](https://github.com/ocornut/imgui)
A bloat-free graphical user interface (GUI) library used to create in-game menus and debugging interfaces.

### [nativefiledialog](https://github.com/mlabbe/nativefiledialog)
A lightweight library used for opening file dialogs, providing cross-platform file selection capabilities for loading and saving files.

# CalascioNES Compilation Guide

This guide will walk you through the process of compiling **CalascioNES**.

## Prerequisites

Before compiling the project, ensure you have the following dependencies installed:

1. **MinGW
2. **[SDL2](https://www.libsdl.org/)** (for windowing and graphics rendering)
3. **[ImGui](https://github.com/ocornut/imgui)** (for GUI elements)
4. **[NativeFileDialog](https://github.com/mlabbe/nativefiledialog)** (for file dialog support)

## Step 1: Clone the Repository

Clone the repository and navigate into the project folder:

```
git clone https://github.com/Franco1262/CalascioNES.git 
cd CalascioNES
```

## Step 2: Add dependencies

Clone ImGUI and NativeFileDialog into the current folder. Download SDL2 and move it into this folder

Folder structure should look like this:

```
CalascioNES/
│
├── imgui/                # ImGui library
├── NativeFileDialog/     # NativeFileDialog library
├── SDL2/                 # SDL2 library
├── main.cpp
├── CPU.cpp
├── src/
│   ├── PPU.cpp
│   ├── Cartridge.cpp
│   ├── Bus.cpp
│   ├── NROM.cpp
│   ├── UxROM.cpp
│   ├── CNROM.cpp
│   ├── SxROM.cpp
│   └── AxROM.cpp
└── include/
```

## Step 3: Run this command in the cmd
```
g++ main.cpp src/CPU.cpp src/PPU.cpp src/Cartridge.cpp src/Bus.cpp src/NROM.cpp src/UxROM.cpp src/CNROM.cpp src/SxROM.cpp src/AxROM.cpp imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_sdl2.cpp imgui/backends/imgui_impl_sdlrenderer2.cpp -L./SDL2/x86_64-w64-mingw32/lib -L./nativefiledialog/build/lib/Release/x64 -lmingw32 -lSDL2main -lSDL2 -lnfd -lcomctl32 -lole32 -luuid -lshell32 -O3 -flto -march=native -fomit-frame-pointer -funroll-loops -I./nativefiledialog/src/include -I./include -I./imgui -I./imgui/backends -I./SDL2/x86_64-w64-mingw32/include/SDL2 -Wall -mwindows -o main.exe
```


# Extra Notes

As mentioned in the introduction, this emulator is a personal project designed for learning and experimentation. While it is far from perfect, it can successfully run most games that are compatible with the integrated mappers. A few exceptions may not work due to the need for precise timing in some games, while others may encounter issues due to inaccuracies elsewhere. However, the majority of games work as expected.

This emulator was only tested on Windows




 
