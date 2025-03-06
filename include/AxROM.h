#pragma once
#include <iostream>
#include <cstdint>
#include "Mapper.h"

class Cartridge;
class AxROM : public Mapper
{
    public:
        AxROM(int n_prg_rom_banks, int n_chr_rom_banks, std::shared_ptr<Cartridge> cart) : Mapper(n_prg_rom_banks, n_chr_rom_banks, cart) {};
        ~AxROM() override { };
        uint32_t cpu_reads(uint16_t address);
        uint32_t ppu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value);
        MIRROR get_mirroring_mode();
    private:
        uint8_t bank_switching = 0;
        MIRROR mirroring_mode = MIRROR::HORIZONTAL;
};