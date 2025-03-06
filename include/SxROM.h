#pragma once
#include <iostream>
#include <cstdint>
#include "Mapper.h"
#include "Logger.h"
#include <sstream>

class Cartridge;
class SxROM : public Mapper
{
    public:
        SxROM(int n_prg_rom_banks, int n_chr_rom_banks, std::shared_ptr<Cartridge> cart);
        ~SxROM() override { };
        uint32_t cpu_reads(uint16_t address);
        uint32_t ppu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value);
        MIRROR get_mirroring_mode();
        void update_state();
    private:
        uint8_t load_register;
        uint8_t control ;
        uint8_t chr_bank_0;
        uint8_t chr_bank_1;
        uint8_t prg_bank;
        uint8_t shift_register;
        short n_write;
        uint8_t prg_rom_mode;
        bool chr_rom_mode;

        MIRROR mirroring_mode;
        bool written_on_this_instruction;
};