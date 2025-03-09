#include "SxROM.h"
#include "Cartridge.h"

SxROM::SxROM(int n_prg_rom_banks, int n_chr_rom_banks, std::shared_ptr<Cartridge> cart) : Mapper(n_prg_rom_banks, n_chr_rom_banks, cart)
{
    n_write = 0;
    shift_register = 0x10;
    prg_rom_mode = 3;
    chr_rom_mode = 0;
    chr_bank_0 = 0;
    chr_bank_1 = 0;
    prg_bank = 0;
    control = 0;
    mirroring_mode = MIRROR::HORIZONTAL;
}

uint32_t SxROM::cpu_reads(uint16_t address)
{
    uint32_t mapped_addr;

    //PRG RAM
    if(address >= 0x6000 && address < 0x8000)
        mapped_addr = address & (PRG_ROM_BANK_SIZE_8KB-1);
    
    //PRG ROM
    else if(address >= 0x8000 && address <= 0xFFFF)
    {
        switch(prg_rom_mode)
        {
            case 0:
            case 1:
                mapped_addr = (((prg_bank & 0x1E)) * PRG_ROM_BANK_SIZE_16KB) + (address & (PRG_ROM_BANK_SIZE_32KB-1));;
                break; 
            case 2:
                if(address < 0xC000) 
                    mapped_addr = address & (PRG_ROM_BANK_SIZE_16KB-1);
                else 
                    mapped_addr = ((prg_bank & 0xF) * PRG_ROM_BANK_SIZE_16KB) + (address & (PRG_ROM_BANK_SIZE_16KB-1));
                break;

            case 3:
                if(address < 0xC000) 
                    mapped_addr = ((prg_bank & 0xF) * PRG_ROM_BANK_SIZE_16KB) + (address & (PRG_ROM_BANK_SIZE_16KB-1));
                else 
                    mapped_addr = ((n_prg_rom_banks - 1) * PRG_ROM_BANK_SIZE_16KB) + (address & (PRG_ROM_BANK_SIZE_16KB-1));
                break;
        }     
    }

    return mapped_addr; 
}

uint32_t SxROM::ppu_reads(uint16_t address)
{
    uint32_t mapped_addr = 0;

    //CHR ROM
    if(n_chr_rom_banks != 0)
    {
        if(chr_rom_mode)
        {
            if(address >= 0x0000 && address < 0x1000)
                mapped_addr = (chr_bank_0 * CHR_ROM_BANK_SIZE_4KB) + (address & (CHR_ROM_BANK_SIZE_4KB-1));

            else if(address >= 0x1000 && address < 0x2000)
                mapped_addr = (chr_bank_1 * CHR_ROM_BANK_SIZE_4KB) + (address & (CHR_ROM_BANK_SIZE_4KB-1));         
        }
        else
        {
            if(address >= 0x0000 && address < 0x2000)
                mapped_addr = ( ((chr_bank_0 & 0xFE)) * CHR_ROM_BANK_SIZE_4KB) + (address & (CHR_ROM_BANK_SIZE_8KB-1));
        }
    }
    //CHR RAM
    else
    {
        if(chr_rom_mode)
        {
            if(address >= 0x0000 && address < 0x1000)
                mapped_addr = ((chr_bank_0 & 0x1) * CHR_ROM_BANK_SIZE_4KB) + (address & (CHR_ROM_BANK_SIZE_4KB-1));

            else if(address >= 0x1000 && address < 0x2000)
                mapped_addr = ((chr_bank_1 & 0x1) * CHR_ROM_BANK_SIZE_4KB) + (address & (CHR_ROM_BANK_SIZE_4KB-1));             
        }
        else
            mapped_addr = address & (CHR_ROM_BANK_SIZE_8KB-1);
    }
    
    return mapped_addr;
}

void SxROM::cpu_writes(uint16_t address, uint8_t value)
{
    if (value & 0x80)
    {
        shift_register = 0x10; // Reset to 0x00
        n_write = 0;
        control |= 0xC;
        update_state();
    }  

    else
    {
        if(cart->is_new_instruction()) //Function useful for ignoring sucessive writes inside the same instruction
        {
            shift_register = (shift_register >> 1) | ((value & 1) << 4);
            n_write++;
            address = (address >> 13) & 0x3;
            if (n_write == 5)
            {
                switch(address)
                {
                    case 0:
                        control = shift_register & 0x1F; 
                        update_state();
                        break;
                    case 1:
                        chr_bank_0 = shift_register & 0x1F;
                        break;
                    case 2:
                        chr_bank_1 = shift_register & 0x1F;
                        break;
                    case 3:
                        prg_bank = ((shift_register & 0x0F) & (n_prg_rom_banks-1));
                        break;                  
                }  
                n_write = 0; // Reset write count
                shift_register = 0x10; // Reset the shift register
            }
        }
    }
}


void SxROM::update_state()
{
    prg_rom_mode = (control & 0xC) >> 2;
    chr_rom_mode = (control & 0x10) > 0;
    uint8_t mirror = control & 0x3;

    switch(mirror)
    {
        case 0:
            mirroring_mode = MIRROR::ONE_SCREEN_LOWER;
            break;
        case 1:
            mirroring_mode = MIRROR::ONE_SCREEN_UPPER;
            break;
        case 2:
            mirroring_mode = MIRROR::VERTICAL;
            break;
        case 3:
            mirroring_mode = MIRROR::HORIZONTAL;
            break;
    }
    cart->set_mirroring_mode(mirroring_mode);
}