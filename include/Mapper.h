#pragma once
#include <iostream>
#include <cstdint>


enum MIRROR
{
    HORIZONTAL,
    VERTICAL ,
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
        virtual uint16_t cpu_reads(uint16_t address) = 0;
        virtual uint16_t ppu_reads(uint16_t address) = 0;
        virtual void cpu_writes(uint16_t address, uint8_t value) = 0;
        virtual int get_bank_number() = 0;
    protected:
        int n_prg_rom_banks;
        int n_chr_rom_banks;
};