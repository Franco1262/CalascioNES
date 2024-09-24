#include "CNROM.h"
#include <iostream>

uint32_t CNROM::cpu_reads(uint16_t address)
{
    uint32_t mapped_addr;

    if(n_prg_rom_banks == 1)
        mapped_addr = address  % 0x4000;
    else
        mapped_addr = address % 0x8000;   
    return mapped_addr; 
}

uint32_t CNROM::ppu_reads(uint16_t address)
{
    return ((bank_switching & 0x3) * 0x2000) + address;
}

void CNROM::cpu_writes(uint16_t address, uint8_t value)
{
    bank_switching = value;
}
