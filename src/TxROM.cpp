#include "TxROM.h"
#include "Cartridge.h"

TxROM::TxROM(int n_prg_rom_banks, int n_chr_rom_banks, std::shared_ptr<Cartridge> cart) : Mapper(n_prg_rom_banks * 2, n_chr_rom_banks, cart)
{
    select_bank = 0;
    R0 = 0;
    R1 = 0;
    R2 = 0;
    R3 = 0;
    R4 = 0;
    R5 = 0;
    R6 = 0;
    R7 = 0;
    prg_rom_bank_mode = 0;
    chr_inversion = 0;
    mirroring_mode = MIRROR::HORIZONTAL;
    irq_latch = 0;
    irq_reload = 0;
    irq_counter = 0;
}

void TxROM::cpu_writes(uint16_t address, uint8_t value)
{
    if(address >= 0x8000 && address <= 0x9FFF)
    {
        if(!(address & 0x1))
        {
            select_bank = value & 0x7;
            prg_rom_bank_mode = value & 0x40;
            chr_inversion = value & 0x80;
        }
        else
        {
            switch(select_bank)
            {
                case 0: R0 = (value & 0xFE); break; // Ignore LSB
                case 1: R1 = (value & 0xFE); break; // Ignore LSB
                case 2: R2 = value; break;
                case 3: R3 = value; break;
                case 4: R4 = value; break;
                case 5: R5 = value; break;
                case 6: R6 = (value & 0x3F) & (n_prg_rom_banks-1); break; // Ignore top 2 bits
                case 7: R7 = (value & 0x3F) & (n_prg_rom_banks-1); break; // Ignore top 2 bits
            }
        }
    }

    if(address >= 0xA000 && address <= 0xBFFF)
    {
        //On ODD adresses doesnt do anything on MMC3
        if(!(address & 0x1))
        {
            mirroring_mode = (value & 0x1) ? MIRROR::HORIZONTAL : MIRROR::VERTICAL;
            cart->set_mirroring_mode(mirroring_mode);
        }             
    }

    if(address >= 0xC000 && address <= 0xDFFF)
    {
        if(address & 0x1)
            cart->set_irq_reload();
        else
            cart->set_irq_latch(value);

    }

    if(address >= 0xE000 && address <= 0xFFFF)
    {
        if(address & 0x1)
            cart->set_irq_enable(1);
        else
            cart->set_irq_enable(0);
    }
}

uint32_t TxROM::cpu_reads(uint16_t address)
{
    if(address >= 0x6000 && address < 0x8000) 
        mapped_address = address & (PRG_ROM_BANK_SIZE_8KB-1);

    else if(address >= 0x8000 && address <= 0xFFFF)
    {
        if(prg_rom_bank_mode)
        {
            if(address >= 0x8000 && address <= 0x9FFF)
                mapped_address = ((n_prg_rom_banks-2) * PRG_ROM_BANK_SIZE_8KB) + (address & (PRG_ROM_BANK_SIZE_8KB-1));
            if(address >= 0xA000 && address <= 0xBFFF)
                mapped_address = (R7 * PRG_ROM_BANK_SIZE_8KB) + (address & (PRG_ROM_BANK_SIZE_8KB-1));
            if(address >= 0xC000 && address <= 0xDFFF)
                mapped_address = (R6 * PRG_ROM_BANK_SIZE_8KB) + (address & (PRG_ROM_BANK_SIZE_8KB-1));
            if(address >= 0xE000 && address <= 0xFFFF)
                mapped_address = ((n_prg_rom_banks-1) * PRG_ROM_BANK_SIZE_8KB) + (address & (PRG_ROM_BANK_SIZE_8KB-1));        
        }
    
        else
        {
            if((address >= 0x8000) && (address <= 0x9FFF))
                mapped_address = (R6 * PRG_ROM_BANK_SIZE_8KB) + (address & (PRG_ROM_BANK_SIZE_8KB-1));
            if((address >= 0xA000) && (address <= 0xBFFF))
                mapped_address = (R7 * PRG_ROM_BANK_SIZE_8KB) + (address & (PRG_ROM_BANK_SIZE_8KB-1));
            if((address >= 0xC000) && (address <= 0xDFFF))
                mapped_address = ((n_prg_rom_banks-2) * PRG_ROM_BANK_SIZE_8KB) + (address & (PRG_ROM_BANK_SIZE_8KB-1));
            if((address >= 0xE000) && (address <= 0xFFFF))
                mapped_address = ((n_prg_rom_banks-1) * PRG_ROM_BANK_SIZE_8KB) + (address & (PRG_ROM_BANK_SIZE_8KB-1));       
        }
    }

    return mapped_address;
}

uint32_t TxROM::ppu_reads(uint16_t address)
{
    uint32_t mapped_address = 0; 

    if(chr_inversion)
    {
        if(address >= 0x0000 && address <= 0x03FF)
            mapped_address = (R2 * CHR_ROM_BANK_SIZE_1KB) + (address & (CHR_ROM_BANK_SIZE_1KB-1));
        if(address >= 0x0400 && address <= 0x07FF)
            mapped_address = (R3 * CHR_ROM_BANK_SIZE_1KB) + (address & (CHR_ROM_BANK_SIZE_1KB-1));
        if(address >= 0x0800 && address <= 0x0BFF)
            mapped_address = (R4 * CHR_ROM_BANK_SIZE_1KB) + (address & (CHR_ROM_BANK_SIZE_1KB-1));
        if(address >= 0x0C00 && address <= 0x0FFF)
            mapped_address = (R5 * CHR_ROM_BANK_SIZE_1KB) + (address & (CHR_ROM_BANK_SIZE_1KB-1));
        if(address >= 0x1000 && address <= 0x17FF)
            mapped_address = (R0 * CHR_ROM_BANK_SIZE_1KB) + (address & (CHR_ROM_BANK_SIZE_2KB-1));
        if(address >= 0x1800 && address <= 0x1FFF)
            mapped_address = (R1 * CHR_ROM_BANK_SIZE_1KB) + (address & (CHR_ROM_BANK_SIZE_2KB-1));
    }

    else
    {
        if(address >= 0x0000 && address <= 0x07FF)
            mapped_address = (R0 * CHR_ROM_BANK_SIZE_1KB) + (address & (CHR_ROM_BANK_SIZE_2KB-1));
        if(address >= 0x0800 && address <= 0x0FFF)
            mapped_address = (R1 * CHR_ROM_BANK_SIZE_1KB) + (address & (CHR_ROM_BANK_SIZE_2KB-1));
        if(address >= 0x1000 && address <= 0x13FF)
            mapped_address = (R2 * CHR_ROM_BANK_SIZE_1KB) + (address & (CHR_ROM_BANK_SIZE_1KB-1));
        if(address >= 0x1400 && address <= 0x17FF)
            mapped_address = (R3 * CHR_ROM_BANK_SIZE_1KB) + (address & (CHR_ROM_BANK_SIZE_1KB-1));
        if(address >= 0x1800 && address <= 0x1BFF)
            mapped_address = (R4 * CHR_ROM_BANK_SIZE_1KB) + (address & (CHR_ROM_BANK_SIZE_1KB-1));
        if(address >= 0x1C00 && address <= 0x1FFF)
            mapped_address = (R5 * CHR_ROM_BANK_SIZE_1KB) + (address & (CHR_ROM_BANK_SIZE_1KB-1));     
    }

    return mapped_address;
}
