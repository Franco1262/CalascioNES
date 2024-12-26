## A NES emualtor written in C++

This NES emulator, written in C++, is designed as a learning project to explore the inner workings of the Nintendo Entertainment System. It integrates a cycle-accurate MOS 6502 CPU, the Picture Processing Unit (PPU), and will soon the Audio Processing Unit (APU) to replicate the core functionality of the NES hardware. The goal of this project is to provide a better understanding of emulation concepts by recreating the essential components of the NES system and executing games in a controlled, software-driven environment.

## What is implemented?

* CPU (MOS 6502)
  - [x] Official opcodes
  - [x] Cycle accuracy
  - [ ] Dummy reads/writes (Some of them are implemented).
* PPU
  - [x] NTSC
  - [x] PAL
* APU (Still not implemented. Soon will be)
* Controllers
   - [x] Keyboard inpute
   - [x] Zapper
* iNES format
* Mappers
  - [x] Mapper 0 (NROM)
  - [x] Mapper 2 (UxROM) 
  - [x] Mapper 1 (MMC1)
  - [x] Mapper 3 (CNROM) 
  - [x] Mapper 7 (AxROM)
  - [ ] Mapper 4 (MMC3) Soon

## Visual examples

![image](https://github.com/user-attachments/assets/0a1519d6-324f-4245-a658-5448ebea2d89)

![image](https://github.com/user-attachments/assets/d78a2bef-d4bf-4c3d-a728-17a562d89aa4)

![image](https://github.com/user-attachments/assets/f555db1d-a293-4058-a9b5-f3efbf6c6f55)

## Controller setup

| Button      | Key   | Description                            |
|-------------|-------|----------------------------------------|
| A           | 8 (Numpad)     | Jump or primary action button          |
| B           | 7 (Numpad)    | Secondary action button                |
| Start       | 5 (Numpad)    | Start the game or pause                |
| Select      | 4 (Numpad)    | Select item in the menu                |
| D-Pad (Up)  | W     | Move up in the game                    |
| D-Pad (Down)| S     | Move down in the game                  |
| D-Pad (Left)| A     | Move left in the game                  |
| D-Pad (Right)| D    | Move right in the game                 |

### Zapper (Light gun for the nes)

For games implementing the Zapper just click on settings and then click on Zapper. Now the zapper is enabled, to use it just left click on your mouse

## Compiling (TODO)

## Extra notes

As explained in the introduction, this emulator is a personal project for learning and having fun, is far from perfect although you can play with this emulator almost all the games included in the integrated mappers, there are a few exceptions that don't work because these games require a fairly precise timing to work, other games might not work due to inaccuracies elsewhere but as said these are a few exceptions MOST games work.



 
