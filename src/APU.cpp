#include "APU.h"
#include "Bus.h"
#include <cmath>


APU::APU()
{
    sequence_lookup_table = {0b01000000, 0b01100000, 0b01111000, 0b10011111};
    length_counter_lookup_table = 
    {
        10, 254, 20, 2, 40, 4, 80, 6,
        160, 8, 60, 10, 14, 12, 26, 14,
        12, 16, 24, 18, 48, 20, 96, 22,
        192, 24, 72, 26, 16, 28, 32, 30
    };
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
$4003 / $4007 |	LLLL LTTT |	Length counter load (L), timer_divider high (T)
________________________________________________________________________________________________________________________
*/

void APU::cpu_writes(uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0x4000:
            pulse1.volume = value & 0xF;
            pulse1.const_volume = (value & 0x10) > 0;
            pulse1.envelope_loop = (value & 0x20) > 0;
            pulse1.duty = (value & 0xC0) >> 6;           
            break;
        case 0x4001:
            pulse1.shift = value & 0x7;
            pulse1.negate = (value & 0x8) > 0;
            pulse1.period = (value & 0x70) >> 4;
            pulse1.sweep_unit_enabled = (value & 0x80) > 0;

            //Side effects of writing to this register
            pulse1.reload_flag = true;
            break;
        case 0x4002:
            pulse1.timer = (pulse1.timer & 0xFF00) | value;
            break;
        case 0x4003:
            pulse1.timer = (pulse1.timer & 0x00FF) | ((value & 0x7) << 8);
            pulse1.length_counter_load = length_counter_lookup_table[((value & 0xF8) >> 3)];

            //Restart envelope
            pulse1.envelope_decay_level_counter = 15;
            pulse1.envelope_divider = pulse1.volume;
            //Side effects of writing to this register
            pulse1.sequence_step = 7;
            pulse1.start_flag = true;
            break;
        case 0x4004:
            pulse2.volume = value & 0xF;
            pulse2.const_volume = (value & 0x10) > 0;
            pulse2.envelope_loop = (value & 0x20) > 0;
            pulse2.duty = (value & 0xC0) >> 6;          
            break;
        case 0x4005:
            pulse2.shift = value & 0x7;
            pulse2.negate = (value & 0x8) > 0;
            pulse2.period = (value & 0x70) >> 4;
            pulse2.sweep_unit_enabled = (value & 0x80) > 0;

            //Side effects of writing to this register
            pulse2.reload_flag = true;
            break;
        case 0x4006:
            pulse2.timer = (pulse2.timer & 0xFF00) | value;
            break;
        case 0x4007:
            pulse2.timer = (pulse2.timer & 0x00FF) | ((value & 0x7) << 8);
            pulse2.length_counter_load = length_counter_lookup_table[((value & 0xF8) >> 3)];
            //Restart envelope
            pulse2.envelope_decay_level_counter = 15;
            pulse2.envelope_divider = pulse2.volume;
            //Side effects of writing to this register
            pulse2.sequence_step = 7;
            pulse2.start_flag = true;
            break;
        //Used for enabling and disabling individual channels
        case 0x4015:
            status_register = value;
            if (!(value & 0x01)) pulse1.length_counter_load = 0;
            if (!(value & 0x02)) pulse2.length_counter_load = 0;
            break;
        case 0x4017:
            sequence_mode = (value & 0x80) > 0;
            inhibit_flag = (value & 0x40) > 0;
            if(ceil(apu_cycles_counter) == apu_cycles_counter)
                delay_write_to_frame_counter = 3;
            else
                delay_write_to_frame_counter = 4;
            reset = true;
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
        //Every apu cycle...
        if(ceil(apu_cycles_counter) == apu_cycles_counter)
            tick_pulse_timer();
            
        if(apu_cycles_counter == 3728.5 || apu_cycles_counter == 7456.5 || apu_cycles_counter == 11185.5 ||
            apu_cycles_counter == 14914.5 || apu_cycles_counter == 18640.5)
        {
            tick_frame_counter();  
        }
    }
    delay_write_to_frame_counter--;
    if(delay_write_to_frame_counter == 0 && reset)
    {
        sequence_step = 0.0;
        apu_cycles_counter = 0.0;
        reset = false;
        if(sequence_mode)
        {
            tick_envelope();
            tick_sweep();
            tick_length_counter();
            //tick_linear_counter();
        }
    }
}

void APU::tick_frame_counter()
{
    switch (sequence_step)
    {
        case 0:
            tick_envelope();
            //tick_linear_counter();
            sequence_step++;
            break;
        case 1:
            tick_envelope();
            //tick_linear_counter();
            tick_length_counter();
            tick_sweep();
            sequence_step++;
            break;
        case 2:
            tick_envelope();
            //tick_linear_counter();
            sequence_step++; 
            break;
        case 3:
            if(sequence_mode)
                sequence_step++;
            else
            {
                tick_envelope();
                tick_length_counter();
                //tick_linear_counter();
                tick_sweep();
                if(!inhibit_flag)
                    bus->apu_irq();
                sequence_step = 0;
                apu_cycles_counter = 0.0;
            }
            break;
        case 4:
            tick_envelope();
            //tick_linear_counter();
            tick_length_counter();
            tick_sweep();
            sequence_step = 0;
            apu_cycles_counter = 0.0;
            break;
    }
}

void APU::tick_envelope()
{
    if(!pulse1.start_flag)
    {
        if(pulse1.envelope_divider == 0)
        {
            pulse1.envelope_divider = pulse1.volume;
            if(pulse1.envelope_decay_level_counter != 0)
                pulse1.envelope_decay_level_counter--;
            else if(pulse1.envelope_loop)
                pulse1.envelope_decay_level_counter = 15;          
        }
        else
            pulse1.envelope_divider--;
    }
    else
    {
        pulse1.start_flag = false;
        pulse1.envelope_decay_level_counter = 15;
        pulse1.envelope_divider = pulse1.volume;
    }

    if(!pulse2.start_flag)
    {
        if(pulse2.envelope_divider == 0)
        {
            pulse2.envelope_divider = pulse2.volume;
            if(pulse2.envelope_decay_level_counter != 0)
                pulse2.envelope_decay_level_counter--;
            else if(pulse2.envelope_loop)
                    pulse2.envelope_decay_level_counter = 15;          
        }
        else
            pulse2.envelope_divider--;
    }
    else
    {
        pulse2.start_flag = false;
        pulse2.envelope_decay_level_counter = 15;
        pulse2.envelope_divider = pulse2.volume;
    } 
}

void APU::tick_length_counter()
{
    if(!pulse1.envelope_loop && (pulse1.length_counter_load != 0))
        pulse1.length_counter_load--;

    if(!pulse2.envelope_loop && (pulse2.length_counter_load != 0))
        pulse2.length_counter_load--;
}

void APU::tick_pulse_timer()
{
    if(pulse1.timer_divider == 0)
    {
        pulse1.timer_divider = pulse1.timer;
        pulse1.sequencer_output = (sequence_lookup_table[pulse1.duty] >> pulse1.sequence_step) & 0x1;
        if(pulse1.sequence_step == 0)
            pulse1.sequence_step = 7;
        else
            pulse1.sequence_step--;
    }
    else
        pulse1.timer_divider--;
    
    //if pulse2 enabled
    if(pulse2.timer_divider == 0)
    {
        pulse2.timer_divider = pulse2.timer;
        pulse2.sequencer_output = (sequence_lookup_table[pulse2.duty] >> pulse2.sequence_step) & 0x1;
        if(pulse2.sequence_step == 0)
            pulse2.sequence_step = 7;
        else
            pulse2.sequence_step--;
    }
    else
        pulse2.timer_divider--;
}

//the pulse number is passed as parameter because the behaviour when change amount is negated differs from one another
void APU::calculate_target_period_pulse(Pulse &pulse, int npulse)
{
    int16_t change_amount = pulse.timer >> pulse.shift;
    int16_t aux_target_period = 0;

    if(pulse.negate)
    {
        if(npulse == 1)
            change_amount = (change_amount * -1) - 1;
        else if(npulse == 2)
            change_amount = change_amount * -1;
    }

    aux_target_period = pulse.timer + change_amount;
    if(aux_target_period < 0)
        aux_target_period = 0;

    pulse.target_period = aux_target_period;
}

void APU::tick_sweep()
{
    //Sweep unit is countinuosly calculating the target period even if disabled
    calculate_target_period_pulse(pulse1, 1);
    calculate_target_period_pulse(pulse2, 2);
    //Pulse 1
    if(pulse1.sweep_unit_enabled && (pulse1.shift > 0))
    {
        //if sweep unit is not muting the channel
        if((pulse1.target_period <= 0x7FF) && ((pulse1.sweep_divider_counter == 0)) && (pulse1.timer >= 8))
            pulse1.timer = pulse1.target_period; 
            
        if(pulse1.sweep_divider_counter == 0 || pulse1.reload_flag)
        {
            pulse1.sweep_divider_counter = pulse1.period;
            pulse1.reload_flag = false;
        }
        else
            pulse1.sweep_divider_counter--;
    }

    //Pulse 2
    if(pulse2.sweep_unit_enabled && (pulse2.shift > 0))
    {
        //if sweep unit is not muting the channel
        if((pulse2.sweep_divider_counter == 0) && (pulse2.target_period <= 0x7FF) && (pulse2.timer >= 8))
            pulse2.timer = pulse2.target_period;

        if(pulse2.sweep_divider_counter == 0 || pulse2.reload_flag)
        {
            pulse2.sweep_divider_counter = pulse2.period;
            pulse2.reload_flag = false;
        }
        else
            pulse2.sweep_divider_counter--;
    }
}

double APU::get_output()
{
    double pulse1_output = 0;
    double pulse2_output = 0;
    double pulse_output = 0;

    if(pulse1.sequencer_output == 1 && pulse1.target_period <= 0x7FF && pulse1.timer >= 8 && pulse1.length_counter_load > 0 && (status_register & 0x1))
    {
        if(pulse1.const_volume)
            pulse1_output = pulse1.volume;
        else
            pulse1_output = pulse1.envelope_decay_level_counter;
    }
    if(pulse2.sequencer_output == 1 && pulse2.target_period <= 0x7FF && pulse2.timer >= 8 && pulse2.length_counter_load > 0 && (status_register & 0x2))
    {
        if(pulse2.const_volume)
            pulse2_output = pulse2.volume;
        else
            pulse2_output = pulse2.envelope_decay_level_counter;
    }
    
    if(pulse1_output == 0 && pulse2_output == 0)
        pulse_output = 0;
    else
        pulse_output = (95.88 / ((8128 / (pulse1_output + pulse2_output)) + 100));

    return  pulse_output;
}

void APU::connect_bus(std::shared_ptr<Bus> bus)
{
    this->bus = bus;
}

void APU::set_timing(bool value)
{
    region = value;
}