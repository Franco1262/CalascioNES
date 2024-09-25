#include "Bus.h"
#include "Cartridge.h"
#include "CPU.h"
#include "PPU.h"

Bus::Bus(std::shared_ptr<PPU> ppu,  std::shared_ptr<Cartridge> cart)
{
    this->cart = cart;
    this->ppu = ppu;
}

Bus::~Bus() {}

uint8_t Bus::cpu_reads(uint16_t address)
{
    uint8_t data = 0x00;
    if( (address >= 0x2000) && (address < 0x4000) )
        data = ppu->cpu_reads(address & 0x7);

    else if( (address >= 0x4000) && (address < 0x4018) )
    {
        if(address == 0x4016)
        {
            if (strobe) 
            {
                // Return the current state of the A button (bit 0)
                shift_register_controller = controller_state & 0xFF;
                data =  controller_state & 1;
            } 
            else 
            {
                // Shift out the button states
                data = shift_register_controller & 1;
                shift_register_controller >>= 1;
                if(shift_register_controller == 0x00)
                    handle_input = false;
            }
        }

        else if(address == 0x4017)
        {
            if (strobe) 
            {
                // Return the current state of the A button (bit 0)
                shift_register_controller = (controller_state >> 8) & 0xFF;
                data =  controller_state & 1;
            } 
            else 
            {
                // Shift out the button states
                data = shift_register_controller & 1;
                shift_register_controller >>= 1;
                if(shift_register_controller == 0x00)
                    handle_input = false;
            }           
        }
    } 
    
    else if( (address >= 0x4020) && (address <= 0xFFFF))
        data = cart->cpu_reads(address);

    return data;
}

void Bus::cpu_writes(uint16_t address, uint8_t value)
{
    if( (address >= 0x2000) && (address < 0x4000) )
        ppu->cpu_writes((address & 0x7), value);

    else if( (address >= 0x4000) && (address < 0x4018) )
    {
        if(address == 0x4016)
        {
            strobe = value & 1;
            if(strobe)
            {
                handle_input = true;
                shift_register_controller = controller_state; 
            }    
        }
    }

    else if( (address >= 0x4020) && (address <= 0xFFFF))
        cart->cpu_writes(address, value);

}

uint8_t Bus::ppu_reads(uint16_t address)
{
    uint8_t data = 0x00;
    
    if(address >= 0x0000 && address < 0x2000)
        data = cart->ppu_reads(address);

    return data;
}

void Bus::ppu_writes(uint16_t address, uint8_t value)
{
    cart->ppu_writes(address, value);
}

MIRROR Bus::getMirror()
{
    return cart->getMirror();
}

void Bus::set_nmi(bool value)
{
    NMI = value;
}

bool Bus::get_nmi()
{
    return NMI;
}

void Bus::set_input(uint16_t state)
{
    controller_state = state; 
    handle_input = false;
}

bool Bus::get_input()
{
    return handle_input;
}