#pragma once
#include "Mapper.h"

class Cartridge;
class NROM : public Mapper
{
    public:
        NROM(int n_prg_rom_banks, int n_chr_rom_banks, std::shared_ptr<Cartridge> cart) : Mapper(n_prg_rom_banks, n_chr_rom_banks, cart) {};
        ~NROM() override { };
        uint32_t cpu_reads(uint16_t address);
        uint32_t ppu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value) { };
};