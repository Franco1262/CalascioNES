#include "APU.h"
#include "Bus.h"


APU::APU()
{

}
APU::~APU()
{

}

/*
Pulse ($4000–$4007)
________________________________________________________________________________________________________________________
$4000 / $4004 |	DDLC VVVV |	Duty (D), envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)
$4001 / $4005 |	EPPP NSSS |	Sweep unit: enabled (E), period (P), negate (N), shift (S)
$4002 / $4006 |	TTTT TTTT |	Timer low (T)
$4003 / $4007 |	LLLL LTTT |	Length counter load (L), timer high (T)
________________________________________________________________________________________________________________________
*/

void APU::cpu_writes(uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 4000:
            pulse1.volume = value & 0xF;
            pulse1.const_volume = value & 0x10;
            pulse1.envelope_loop = value & 0x20;
            pulse1.duty = value & 0xC0;
            break;
        case 4001:
            pulse1.shift = value & 0x7;
            pulse1.negate = value & 0x8;
            pulse1.period = value & 0x70;
            pulse1.sweep_unit_enabled = value & 0x80;
            break;
        case 4002:
            pulse1.timer_low = value;
            break;
        case 4003:
            pulse1.timer_high = value & 0x7;
            pulse1.length_counter_load = value & 0xF8;
            break;
        case 4004:
            pulse2.volume = value & 0xF;
            pulse2.const_volume = value & 0x10;
            pulse2.envelope_loop = value & 0x20;
            pulse2.duty = value & 0xC0;
            break;
        case 4005:
            pulse2.shift = value & 0x7;
            pulse2.negate = value & 0x8;
            pulse2.period = value & 0x70;
            pulse2.sweep_unit_enabled = value & 0x80;
            break;
        case 4006:
            pulse2.timer_low = value;
            break;
        case 4007:
            pulse2.timer_high = value & 0x7;
            pulse2.length_counter_load = value & 0xF8;
            break;
        //Used for enabling and disabling individual channels
        case 4015:
            status_register = value;
            break;
        case 4017:
            sequence_mode = value & 0x80;
            inhibit_flag = value & 0x40;
            break;
        default:
            break;
    }
}


/*
mode 0:    mode 1:       function
---------  -----------  -----------------------------
- - - f    - - - - -    IRQ (if bit 6 is clear)
- l - l    - l - - l    Length counter and sweep
e e e e    e e e - e    Envelope and linear counter

either 4 or 5 steps depending on bit 6 of frame counter ($4017)
*/

void APU::tick()
{
    apu_cycles_counter += 0.5;
    if(!region) // NTSC mode
    {
        if(apu_cycles_counter == 3728.5 || apu_cycles_counter == 7456.5 || apu_cycles_counter == 11185.5 ||
            apu_cycles_counter == 14914.5 || apu_cycles_counter == 18640.5)
        {
            tick_frame_counter();  
        }
    }

}

void APU::tick_frame_counter()
{
    switch (sequence_step)
    {
        case 1:
            tick_envelope();
            //tick_linear_counter();
            sequence_step++;
            break;
        case 2:
            tick_envelope();
            //tick_linear_counter();
            tick_length_counter();
            sequence_step++;
            break;
        case 3:
            tick_envelope();
            //tick_linear_counter();
            sequence_step++; 
            break;
        case 4:
            if(sequence_mode)
                sequence_step++;
            else
            {
                tick_envelope();
                //tick_linear_counter();
                sequence_step = 0;
                apu_cycles_counter = 0.0;
            }
            break;
        case 5:
            tick_envelope();
            //tick_linear_counter();
            tick_length_counter();
            if(!inhibit_flag)
                bus->apu_irq();
            sequence_step = 0;
            apu_cycles_counter = 0.0;
            break;
    }
}

void APU::tick_envelope()
{

}
void APU::tick_length_counter()
{

}

void APU::connect_bus(std::shared_ptr<Bus> bus)
{
    this->bus = bus;
}

void APU::set_timing(bool value)
{
    region = value;
}