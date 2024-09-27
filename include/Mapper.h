#pragma once
#include <iostream>
#include <cstdint>


enum MIRROR
{
    HORIZONTAL,
    VERTICAL ,
    ONE_SCREEN_LOWER,
    ONE_SCREEN_UPPER,
};

class Mapper
{
    public:
        Mapper(int n_prg_rom_banks, int n_chr_rom_banks) 
        {
            this->n_prg_rom_banks = n_prg_rom_banks;
            this->n_chr_rom_banks = n_chr_rom_banks;
        };
        
        virtual ~Mapper() { };
        virtual uint32_t cpu_reads(uint16_t address) = 0;
        virtual uint32_t ppu_reads(uint16_t address) = 0;
        virtual void cpu_writes(uint16_t address, uint8_t value) = 0;
        virtual MIRROR get_mirroring_mode() = 0;
        virtual void new_instruction() = 0;
    protected:
        int n_prg_rom_banks;
        int n_chr_rom_banks;
};