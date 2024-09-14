#include "NROM.h"

uint16_t NROM::cpu_reads(uint16_t address)
{
    uint16_t mapped_addr;

    if(n_prg_rom_banks == 1)
        mapped_addr = address  % 0x4000;
    else
        mapped_addr = address % 0x8000;   
    return mapped_addr; 
}

uint16_t NROM::ppu_reads(uint16_t address)
{
    return address;
}