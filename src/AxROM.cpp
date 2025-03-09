#include "AxROM.h"
#include "Cartridge.h"

uint32_t AxROM::cpu_reads(uint16_t address)
{
    mapped_address = (bank_number * PRG_ROM_BANK_SIZE_32KB) + (address & (PRG_ROM_BANK_SIZE_32KB-1));
    return mapped_address; 
}

uint32_t AxROM::ppu_reads(uint16_t address)
{
    return (address & (CHR_ROM_BANK_SIZE_8KB-1));
}

void AxROM::cpu_writes(uint16_t address, uint8_t value)
{
    bank_number = value & 0x7;
    mirroring_mode = ((value & 0x10) > 0) ? MIRROR::ONE_SCREEN_UPPER : MIRROR::ONE_SCREEN_LOWER;
    cart->set_mirroring_mode(mirroring_mode);
}