#include "UxROM.h"
#include "Cartridge.h"

uint32_t UxROM::cpu_reads(uint16_t address)
{
    if(address >= 0x8000 && address <= 0xBFFF)
        mapped_address = ((bank_number & 0xF) * 0x4000) + (address & (PRG_ROM_BANK_SIZE_16KB-1));

    else if((address >= 0xC000) && (address <= 0xFFFF))
        mapped_address = ((n_prg_rom_banks-1) * PRG_ROM_BANK_SIZE_16KB) + (address & (PRG_ROM_BANK_SIZE_16KB-1));
        
    return mapped_address; 
}

uint32_t UxROM::ppu_reads(uint16_t address)
{
    return (address & (CHR_ROM_BANK_SIZE_8KB-1));
}

void UxROM::cpu_writes(uint16_t address, uint8_t value)
{
    bank_number = value;
}
