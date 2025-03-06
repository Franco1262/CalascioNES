#include "NROM.h"
#include "Cartridge.h"

uint32_t NROM::cpu_reads(uint16_t address)
{
    uint16_t mapped_addr;

    if(n_prg_rom_banks == 1)
        mapped_addr = address  & 0x3FFF;
    else
        mapped_addr = address & 0x7FFF;   
    return mapped_addr; 
}

uint32_t NROM::ppu_reads(uint16_t address)
{
    return address;
}