#include <iostream>
#include <cstdint>
#include "Mapper.h"

class UxROM : public Mapper
{
    public:
        UxROM(int n_prg_rom_banks, int n_chr_rom_banks) : Mapper(n_prg_rom_banks, n_chr_rom_banks) {}
        ~UxROM() override { };
        uint16_t cpu_reads(uint16_t address);
        uint16_t ppu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value);
        int get_bank_number();
    private:
        uint8_t bank_switching;
};