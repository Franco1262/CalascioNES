#include "UxROM.h"
#include <iostream>
#include "Cartridge.h"

uint32_t UxROM::cpu_reads(uint16_t address)
{
    uint32_t mapped_addr;
   
    mapped_addr = address & 0x3FFF;

    if(address >= 0xC000 && address <= 0xFFFF)
        mapped_addr = ((n_prg_rom_banks-1) * 0x4000) + mapped_addr;
        
    else if(address >= 0x8000 && address <= 0xBFFF)
        mapped_addr = ((bank_switching & 0xF) * 0x4000) + mapped_addr;

    return mapped_addr; 
}

uint32_t UxROM::ppu_reads(uint16_t address)
{
    return (address & 0x1FFF);
}

void UxROM::cpu_writes(uint16_t address, uint8_t value)
{
    bank_switching = value;
}
