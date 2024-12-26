<div align="left">
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
 
