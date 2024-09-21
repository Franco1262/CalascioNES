#include "UxROM.h"
#include <iostream>

uint16_t UxROM::cpu_reads(uint16_t address)
{
    uint16_t mapped_addr;
   
    mapped_addr = address & 0x3FFF;

    return mapped_addr; 
}

uint16_t UxROM::ppu_reads(uint16_t address)
{
    return address;
}

void UxROM::cpu_writes(uint16_t address, uint8_t value)
{
    bank_switching = value;
}

int UxROM::get_bank_number()
{
    return bank_switching;
}