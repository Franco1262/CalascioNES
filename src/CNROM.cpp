#include "CNROM.h"
#include "Cartridge.h"

uint32_t CNROM::cpu_reads(uint16_t address)
{
    mapped_address = (n_prg_rom_banks == 1) ? (address & (PRG_ROM_BANK_SIZE_16KB - 1)) : (address & (PRG_ROM_BANK_SIZE_32KB - 1));
    return mapped_address; 
}

uint32_t CNROM::ppu_reads(uint16_t address)
{
    if(address < CHR_ROM_BANK_SIZE_8KB) mapped_address = ((bank_number * CHR_ROM_BANK_SIZE_8KB) + (address & (CHR_ROM_BANK_SIZE_8KB-1)));
    return mapped_address;
}

void CNROM::cpu_writes(uint16_t address, uint8_t value)
{
    //Bank number is selected by the 2 rightmost bits
    bank_number = value & 0x3;
}
