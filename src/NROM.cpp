#include "NROM.h"
#include "Cartridge.h"

uint32_t NROM::cpu_reads(uint16_t address)
{
    mapped_address = (n_prg_rom_banks == 1) ? (address & (PRG_ROM_BANK_SIZE_16KB - 1)) : (address & (PRG_ROM_BANK_SIZE_32KB - 1));
    return mapped_address; 
}

uint32_t NROM::ppu_reads(uint16_t address)
{
    return address;
}