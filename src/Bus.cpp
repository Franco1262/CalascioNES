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
                data =  controller_state & 1;
                
            else 
            {
                // Shift out the button states
                data = shift_register_controller1 & 1;
                shift_register_controller1 >>= 1;
            }
        }

        else if(address == 0x4017)
        {
            if (strobe)        
                data =  (controller_state >> 8) & 1;
            
            else 
            {
                // Shift out the button states
                data = shift_register_controller2 & 1;
                shift_register_controller2 >>= 1;
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

                const uint8_t *keystate = SDL_GetKeyboardState(NULL);

                uint16_t state = 0x0000;
                
                if (keystate[SDL_SCANCODE_KP_8])  state |= 1 << 0;  // A button
                if (keystate[SDL_SCANCODE_KP_7])  state |= 1 << 1;  // B button
                if (keystate[SDL_SCANCODE_KP_4])  state |= 1 << 2;  // Select
                if (keystate[SDL_SCANCODE_KP_5])  state |= 1 << 3;  // Start
                if (keystate[SDL_SCANCODE_W])    state |= 1 << 4;  // Up
                if (keystate[SDL_SCANCODE_S])  state |= 1 << 5;  // Down
                if (keystate[SDL_SCANCODE_A])  state |= 1 << 6;  // Left
                if (keystate[SDL_SCANCODE_D]) state |= 1 << 7;  // Right

                if (keystate[SDL_SCANCODE_S])  state |= 1 << 8;  // A button
                if (keystate[SDL_SCANCODE_A])  state |= 1 << 9;  // B button
                if (keystate[SDL_SCANCODE_W])  state |= 1 << 10;  // Select
                if (keystate[SDL_SCANCODE_E])  state |= 1 << 11;  // Start
                if (keystate[SDL_SCANCODE_UP])    state |= 1 << 12;  // Up
                if (keystate[SDL_SCANCODE_DOWN])  state |= 1 << 13;  // Down
                if (keystate[SDL_SCANCODE_LEFT])  state |= 1 << 14;  // Left
                if (keystate[SDL_SCANCODE_RIGHT]) state |= 1 << 15;  // Right

                shift_register_controller1 = state & 0xFF;
                shift_register_controller2 = (state >> 8) & 0xFF;
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

void Bus::new_instruction()
{
    cart->new_instruction();  
}