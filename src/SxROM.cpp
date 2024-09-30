#include "SxROM.h"
#include <iostream>

SxROM::SxROM(int n_prg_rom_banks, int n_chr_rom_banks)  : Mapper(n_prg_rom_banks, n_chr_rom_banks) 
{
    n_write = 0;
    shift_register = 0x00;
    prg_rom_mode = 3;
    written_on_this_instruction = false;
}

uint32_t SxROM::cpu_reads(uint16_t address)
{
    uint32_t mapped_addr;

    if(address >= 0x6000 && address < 0x8000)
    {
        if(!chr_rom_mode)
            mapped_addr =  address & 0x1FFF;
        else
        {
            if(address >= 0x6000 && address < 0x7000)
                mapped_addr = (((chr_bank_0 & 0xC) >> 2) * 0x1000) + (address & 0x0FFF);
            
            else
                mapped_addr = (((chr_bank_1 & 0xC) >> 2) * 0x1000) + (address & 0x0FFF);
        }
    }

    else
    {
        if(prg_rom_mode < 2)
        {
            if(address >= 0x8000 && address <= 0xFFFF)
                mapped_addr = (((prg_bank & 0xE) >> 1) * 0x4000) + (address & 0x7FFF);
        }

        else if(prg_rom_mode == 2)
        {
            if(address >= 0x8000 && address < 0xC000)
                mapped_addr = address & 0x3FFF;  // Map to the lower PRG-ROM bank
            else if(address >= 0xC000 && address <= 0xFFFF)
                mapped_addr = ((prg_bank) * 0x4000) + (address & 0x3FFF);
        }

        else if(prg_rom_mode == 3)
        {
            if(address >= 0x8000 && address < 0xC000)
                mapped_addr = ((prg_bank) * 0x4000) + (address & 0x3FFF);
            else if(address >= 0xC000 && address <= 0xFFFF)
                mapped_addr = ((n_prg_rom_banks- 1) * 0x4000) + (address & 0x3FFF);
        }
    }

    return mapped_addr; 
}

uint32_t SxROM::ppu_reads(uint16_t address)
{
    uint32_t mapped_addr = 0;

    if(n_chr_rom_banks != 0)
    {
        if(chr_rom_mode)
        {
            if(address >= 0x0000 && address < 0x1000)      
                mapped_addr = ((chr_bank_0 & 0x1F) * 0x1000) + (address & 0x0FFF);
            else if(address >= 0x1000 && address < 0x2000)
                mapped_addr = ((chr_bank_1 & 0x1F) * 0x1000) + (address & 0x0FFF);           
        }
        else
        {
            if(address >= 0x0000 && address < 0x2000)
                mapped_addr = ( ((chr_bank_0 & 0x1E) >> 1) * 0x2000) + (address & 0x1FFF);
        }
    }

    else
    {
        if(chr_rom_mode)
        {
            if(address >= 0x0000 && address < 0x1000)      
                mapped_addr = ((chr_bank_0 & 0x1) * 0x1000) + (address & 0x0FFF);
            else if(address >= 0x1000 && address < 0x2000)
                mapped_addr = ((chr_bank_1 & 0x1) * 0x1000) + (address & 0x0FFF);             
        }
        else
        {
            if(address >= 0x0000 && address < 0x2000)
                mapped_addr = address & 0x1FFF;
        }
    }
    return mapped_addr;
}

void SxROM::cpu_writes(uint16_t address, uint8_t value)
{

    if (value & 0x80)
    {
        shift_register = 0x10; // Reset to 0x10
        n_write = 0;
        control |= 0xC;
        update_state();
    }  

    else
    {
        if(!written_on_this_instruction)
        {
            load_register = value;
            shift_register = (shift_register >> 1) | ((load_register & 1) << 4);
            n_write++;
            
            if (n_write == 5)
            {

                if(address >= 0x8000 && address < 0xA000)
                {
                    control = shift_register & 0x1F; 
                    update_state();
                }              
                else if(address >= 0xA000 && address < 0xC000)
                    chr_bank_0 = shift_register & 0x1F;
                else if(address >= 0xC000 && address < 0xE000)
                    chr_bank_1 = shift_register & 0x1F;
                else if(address >= 0xE000 && address <= 0xFFFF)
                    prg_bank = shift_register & 0x0F;      
                n_write = 0; // Reset write count
                shift_register = 0x10; // Reset the shift register
            }
        } 
    }   

    written_on_this_instruction = true;   
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
}

MIRROR SxROM::get_mirroring_mode()
{
    return mirroring_mode;
}

void SxROM::new_instruction()
{
    written_on_this_instruction = false;
}