#include "AxROM.h"

uint32_t AxROM::cpu_reads(uint16_t address)
{
    uint32_t mapped_addr = 0;
   
    mapped_addr = (bank_switching * 0x8000) + (address & 0x7FFF);

    return mapped_addr; 
}

uint32_t AxROM::ppu_reads(uint16_t address)
{
    return address & 0x1FFF;
}

void AxROM::cpu_writes(uint16_t address, uint8_t value)
{
    bank_switching = value & 0x7;
    mirroring_mode = ((value & 0x10) > 0) ? MIRROR::ONE_SCREEN_UPPER : MIRROR::ONE_SCREEN_LOWER;
}

MIRROR AxROM::get_mirroring_mode()
{
    return mirroring_mode;
}