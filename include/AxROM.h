#include <iostream>
#include <cstdint>
#include "Mapper.h"

class AxROM : public Mapper
{
    public:
        AxROM(int n_prg_rom_banks, int n_chr_rom_banks) : Mapper(n_prg_rom_banks, n_chr_rom_banks) {}
        ~AxROM() override { };
        uint32_t cpu_reads(uint16_t address);
        uint32_t ppu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value);
        MIRROR get_mirroring_mode();
        void new_instruction() {};
    private:
        uint8_t bank_switching;
        MIRROR mirroring_mode;
};