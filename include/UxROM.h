#pragma once
#include <iostream>
#include <cstdint>
#include "Mapper.h"

class Cartridge;
class UxROM : public Mapper
{
    public:
        UxROM(int n_prg_rom_banks, int n_chr_rom_banks, std::shared_ptr<Cartridge> cart) : Mapper(n_prg_rom_banks, n_chr_rom_banks, cart) {};
        ~UxROM() override { };
        uint32_t cpu_reads(uint16_t address);
        uint32_t ppu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value);
        MIRROR get_mirroring_mode() {return MIRROR::HORIZONTAL;};
    private:
        uint8_t bank_switching = 0;
};