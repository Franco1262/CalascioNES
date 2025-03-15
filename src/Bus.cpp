#include "Bus.h"
#include "Cartridge.h"
#include "CPU.h"
#include "APU.h"
#include "PPU.h"

Bus::Bus(std::shared_ptr<PPU> ppu,  std::shared_ptr<Cartridge> cart, std::shared_ptr<APU> apu, std::shared_ptr<CPU> cpu)
{
    this->cart = cart;
    this->ppu = ppu;
    this->apu = apu;
    this->cpu = cpu;
}

Bus::~Bus() {}

uint8_t Bus::cpu_reads(uint16_t address)
{
    uint8_t data = 0x00;

    if (address >= 0x2000 && address < 0x4000)
        data = ppu->cpu_reads(address & 0x7);
    
    else if (address >= 0x4000 && address < 0x4018)
    {
        if (address == 0x4016)
        {
            if (strobe)
                data = controller_state & 1;
            else
            {
                // Shift out the button states
                data = shift_register_controller1 & 1;
                shift_register_controller1 >>= 1;
            }
            data |= 0x40;
        }

        else if (address == 0x4017)
        {
            if(zapper_connected)
            {
                data = shift_register_controller2;
                zapper.light_sensed = 1;
            }
                       
            else if (strobe)
                data = (controller_state >> 8) & 1;

            else
            {
                // Shift out the button states
                data = shift_register_controller2 & 1;
                shift_register_controller2 >>= 1;
                if(zapper_connected && data == 0x00)
                {
                    zapper.trigger = 1;
                }
                
            }
            
            data |= 0x40;
        }

        else
            data = apu->cpu_reads(address);
        
    }
    
    else if (address >= 0x4020 && address <= 0xFFFF)
    {
        data = cart->cpu_reads(address);
    }

    return data;
}

void Bus::cpu_writes(uint16_t address, uint8_t value)
{
    if ((address >= 0x2000) && (address < 0x4000))
        ppu->cpu_writes((address & 0x7), value);

    else if ((address >= 0x4000) && (address < 0x4018))
    {
        if (address == 0x4016)
        {
            strobe = value & 1;
            if (strobe)
            {

                const uint8_t *keystate = SDL_GetKeyboardState(NULL);

                uint16_t state = 0x0000;

                // Button mappings for the first controller
                if (keystate[SDL_SCANCODE_KP_8])  state |= 1 << 0;  // A button
                if (keystate[SDL_SCANCODE_KP_7])  state |= 1 << 1;  // B button
                if (keystate[SDL_SCANCODE_KP_4])  state |= 1 << 2;  // Select
                if (keystate[SDL_SCANCODE_KP_5])  state |= 1 << 3;  // Start

                // Up and Down
                if (keystate[SDL_SCANCODE_W])      state |= 1 << 4;  // Up
                if (keystate[SDL_SCANCODE_S])      state |= 1 << 5;  // Down

                // Left and Right
                if (keystate[SDL_SCANCODE_A])      state |= 1 << 6;  // Left
                if (keystate[SDL_SCANCODE_D])      state |= 1 << 7;  // Right

                // Reset state for Up and Down if both are pressed
                if (keystate[SDL_SCANCODE_W] && keystate[SDL_SCANCODE_S]) {
                    state &= ~(1 << 4);  // Clear Up
                    state &= ~(1 << 5);  // Clear Down
                }

                // Reset state for Left and Right if both are pressed
                if (keystate[SDL_SCANCODE_A] && keystate[SDL_SCANCODE_D]) {
                    state &= ~(1 << 6);  // Clear Left
                    state &= ~(1 << 7);  // Clear Right
                }

                if(!zapper_connected)
                {
                    // Second controller's button mappings
                    if (keystate[SDL_SCANCODE_Y])  state |= 1 << 8;  // A button
                    if (keystate[SDL_SCANCODE_T])  state |= 1 << 9;  // B button
                    if (keystate[SDL_SCANCODE_G])  state |= 1 << 10; // Select
                    if (keystate[SDL_SCANCODE_H])  state |= 1 << 11; // Start

                    // Up and Down for the second controller
                    if (keystate[SDL_SCANCODE_UP])    state |= 1 << 12;  // Up
                    if (keystate[SDL_SCANCODE_DOWN])  state |= 1 << 13;  // Down
                    if (keystate[SDL_SCANCODE_LEFT])  state |= 1 << 14;  // Left
                    if (keystate[SDL_SCANCODE_RIGHT]) state |= 1 << 15;  // Right

                    // Reset state for Up and Down on the second controller if both are pressed
                    if (keystate[SDL_SCANCODE_UP] && keystate[SDL_SCANCODE_DOWN]) {
                        state &= ~(1 << 12);  // Clear Up
                        state &= ~(1 << 13);  // Clear Down
                    }

                    // Reset state for Left and Right on the second controller if both are pressed
                    if (keystate[SDL_SCANCODE_LEFT] && keystate[SDL_SCANCODE_RIGHT]) {
                        state &= ~(1 << 14);  // Clear Left
                        state &= ~(1 << 15);  // Clear Right
                    }
                    shift_register_controller2 = (state >> 8) & 0xFF;
                }
      
                // Update shift registers
                shift_register_controller1 = state & 0xFF;
            }    
        }
        else
            apu->cpu_writes(address, value);
    } 

    else if ((address >= 0x4020) && (address <= 0xFFFF))
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

void Bus::set_nmi(bool value)
{
    cpu->set_nmi(value);
}

bool Bus::is_new_instruction()
{
    return cpu->is_new_instruction();
}

void Bus::soft_reset()
{
    NMI = false;
    shift_register_controller1 = shift_register_controller2 = 0x0000;
}

void Bus::set_zapper(bool zapper)
{
    zapper_connected = zapper;
    ppu->set_zapper(zapper);
}

void Bus::update_zapper_coordinates(int x, int y)
{
    zapper.x = x;
    zapper.y = y;

    zapper.trigger = 1;
    shift_register_controller2 &= 0xE6;
    shift_register_controller2 |= (zapper.trigger << 4);
}

void Bus::fire_zapper()
{
    zapper.trigger = 0;
    shift_register_controller2 = (shift_register_controller2 & ~(1 << 4)) | (zapper.trigger << 4);
    ppu->check_target_hit(zapper.x, zapper.y);
}

void Bus::set_light_sensed(bool hit)
{
    zapper.light_sensed = !hit;
    shift_register_controller2 = (shift_register_controller2 & ~(1 << 3)) | (zapper.light_sensed << 3);
}

void Bus::assert_irq(IRQ irq)
{
    IRQ_line |= irq;
}

void Bus::ack_irq(IRQ irq)
{
    IRQ_line = IRQ_line & (~irq);
}

uint8_t Bus::get_irq()
{
    return IRQ_line;
}

void Bus::set_irq_latch(uint8_t value)
{
    ppu->set_irq_latch(value);
}

void Bus::set_irq_enable(bool value)
{
    ppu->set_irq_enable(value);
}

void Bus::set_irq_reload()
{
    ppu->set_irq_reload();
}

void Bus::set_mapper(uint8_t value)
{
    ppu->set_mapper(value);
}

void Bus::set_mirroring_mode(MIRROR value)
{
    ppu->set_mirroring_mode(value);
}