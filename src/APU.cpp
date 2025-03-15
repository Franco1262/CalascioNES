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

    triangle_sequence = 
    {
        15, 14, 13, 12, 11, 10,  9,  8,
         7,  6,  5,  4,  3,  2,  1,  0,
         0,  1,  2,  3,  4,  5,  6,  7,
         8,  9, 10, 11, 12, 13, 14, 15
    };

    ntsc_noise_period = 
    {
        4, 8, 16, 32, 64, 96, 128, 160, 
        202, 254, 380, 508, 762, 1016, 2034, 4068
    };

    pal_noise_period = 
    {
        4, 8, 14, 30, 60, 88, 118, 148, 
        188, 236, 354, 472, 708, 944, 1890, 3778
    };

    ntsc_dpcm_period = 
    {
        428, 380, 340, 320, 286, 254, 226, 214, 
        190, 160, 142, 128, 106,  84,  72,  54
    };
    
    pal_dpcm_period = 
    {
        398, 354, 316, 298, 276, 236, 210, 198, 
        176, 148, 132, 118,  98,  78,  66,  50
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
            if ((status_register & 0x01))
                pulse1.length_counter_load = length_counter_lookup_table[((value & 0xF8) >> 3)];

            //Restart envelope
            pulse1.envelope_decay_level_counter = 15;
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
            if ((status_register & 0x02))
                pulse2.length_counter_load = length_counter_lookup_table[((value & 0xF8) >> 3)];
            //Restart envelope
            pulse2.envelope_decay_level_counter = 15;
            //Side effects of writing to this register
            pulse2.sequence_step = 7;
            pulse2.start_flag = true;
            break;
        case 0x4008:
            triangle.linear_counter_load = value & 0x7F;
            triangle.length_counter_halt = value & 0x80;
            break;
        case 0x400A:
            triangle.timer = (triangle.timer & 0xFF00) | value;
            break;
        case 0x400B:
            triangle.timer = (triangle.timer & 0x00FF) | ((value & 0x7) << 8);
            if ((status_register & 0x04))
                triangle.length_counter_load = length_counter_lookup_table[((value & 0xF8) >> 3)];
            triangle.linear_counter_reload = true;
            break;
        case 0x400C:
            noise.volume = value & 0xF;
            noise.const_volume = value & 0x10;
            noise.envelope_loop = value & 0x20;
            break;
        case 0x400E:
            noise.noise_period = value & 0xF;
            if(!region)
                noise.timer = ntsc_noise_period[noise.noise_period];
            else
                noise.timer = pal_noise_period[noise.noise_period];
            noise.loop_noise = value & 0x80;
            break;
        case 0x400F:
            if ((status_register & 0x08))
                noise.length_counter_load = length_counter_lookup_table[((value & 0xF8) >> 3)];
            noise.envelope_decay_level_counter = 15;
            noise.start_flag = true;
            break;

        case 0x4010:
            dmc.IRQ = (value & 0x80) > 0;
            if(!dmc.IRQ)
            {
                dmc.interrupt_flag = false;
                bus->ack_irq(DMC_IRQ);
            }
            dmc.loop = (value & 0x40) > 0;
            dmc.rate_index = value & 0xF;
            dmc.rate = (region > 0) ? pal_dpcm_period[dmc.rate_index] : ntsc_dpcm_period[dmc.rate_index];
            break;
        case 0x4011:
            dmc.direct_load = value & 0x7F;
            dmc.output_level = dmc.direct_load;
            //Check later if im missing something
            break;
        case 0x4012:
            dmc.sample_address = 0xC000 + (value * 64);
            break;
        case 0x4013:
            dmc.sample_length = (value * 16) + 1;
            break;
        //Used for enabling and disabling individual channels
        case 0x4015:
            status_register = value;
            if (!(value & 0x01)) pulse1.length_counter_load = 0;
            if (!(value & 0x02)) pulse2.length_counter_load = 0;
            if (!(value & 0x04)) triangle.length_counter_load = 0;
            if (!(value & 0x8)) noise.length_counter_load = 0;
            //TODO: Fix this, this should reload the sample if bits_remaining are zero, if they are not zero it should wait until they arent
            if (value & 0x10)
            {
                if(dmc.bytes_remaining == 0)
                {
                    //sample restarted
                    dmc.current_address = dmc.sample_address;
                    dmc.bytes_remaining = dmc.sample_length;               
                }
            }

            else
                dmc.bytes_remaining = 0;

            dmc.interrupt_flag = false;
            bus->ack_irq(DMC_IRQ);
            break;
        case 0x4017:
            sequence_mode = (value & 0x80) > 0;
            inhibit_flag = (value & 0x40) > 0;
            if(inhibit_flag)
            {
                frame_interrupt = false;
                bus->ack_irq(Frame_IRQ);
            }

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

uint8_t APU::cpu_reads(uint16_t address)
{
    uint8_t value = 0;
    switch(address)
    {
        case 0x4015:
            value |= pulse1.length_counter_load > 0;
            value |= (pulse2.length_counter_load > 0) << 1;
            value |= (triangle.length_counter_load > 0) << 2;
            value |= (noise.length_counter_load > 0) << 3;
            value |= (dmc.bytes_remaining > 0) << 4;
            value |= frame_interrupt << 6;
            value |= dmc.interrupt_flag << 7;

            frame_interrupt = false;
            bus->ack_irq(Frame_IRQ);
            return value;
        default:
            return 0;
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
        if((apu_cycles_counter == 14914 || apu_cycles_counter == 14914.5 || apu_cycles_counter == 0) && !inhibit_flag && !sequence_mode)
        {
            frame_interrupt = true;
        }

        if(apu_cycles_counter == 3728.5 || apu_cycles_counter == 7456.5 || apu_cycles_counter == 11185.5 ||
            apu_cycles_counter == 14914.5 || apu_cycles_counter == 18640.5)
        {
            tick_frame_counter();  
        }
    }
    else
    {
        if(apu_cycles_counter == 4156.5 || apu_cycles_counter == 8313.5 || apu_cycles_counter == 12469.5 ||
            apu_cycles_counter == 16626.5 || apu_cycles_counter == 20782.5)
        {
            tick_frame_counter();  
        }

        if((apu_cycles_counter == 16626 || apu_cycles_counter == 16626.5 || apu_cycles_counter == 0) && !inhibit_flag && !sequence_mode)
        {
            frame_interrupt = true;
        }
    }

    //Every apu cycle...
    if(ceil(apu_cycles_counter) == apu_cycles_counter)
        tick_timers();

    //On every cpu cycle clock triangle's timer
    tick_triangle_timer();


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
            tick_linear_counter();
        }
    }

    if((dmc.sample_buffer == 0) && (dmc.bytes_remaining > 0))
    {
       // bus->trigger_dmc_dma(dmc.current_address, 0); //0 : reload, 1 : load
        //dmc.sample_buffer = 0xff;
        if(dmc.current_address == 0xFFFF)
            dmc.current_address = 0x8000;
        else
            dmc.current_address++;
        dmc.bytes_remaining--;
        if(dmc.bytes_remaining == 0 && dmc.loop)
        {
            //sample restarted
            dmc.current_address = dmc.sample_address;
            dmc.bytes_remaining = dmc.sample_length;
        }
        else if(dmc.bytes_remaining == 0 && dmc.IRQ)
            dmc.interrupt_flag = true;
    }

    if(dmc.interrupt_flag)
    {
/*         std::cout << "DMC IRQ ASSERTED" << std::endl;
        bus->assert_irq(DMC_IRQ); */
    }
}

void APU::tick_frame_counter()
{
    switch (sequence_step)
    {
        case 0:
            tick_envelope();
            tick_linear_counter();
            sequence_step++;
            break;
        case 1:
            tick_envelope();
            tick_linear_counter();
            tick_length_counter();
            tick_sweep();
            sequence_step++;
            break;
        case 2:
            tick_envelope();
            tick_linear_counter();
            sequence_step++; 
            break;
        case 3:
            if(sequence_mode)
                sequence_step++;
            else
            {
                if(frame_interrupt && !inhibit_flag)
                    bus->assert_irq(Frame_IRQ);
                
      
                tick_envelope();
                tick_length_counter();
                tick_linear_counter();
                tick_sweep();
                sequence_step = 0;
                apu_cycles_counter = 0.0;
            }
            break;
        case 4:
            tick_envelope();
            tick_linear_counter();
            tick_length_counter();
            tick_sweep();
                
            sequence_step = 0;
            apu_cycles_counter = 0.0;
            break;
    }
}

void APU::tick_linear_counter()
{

    if(triangle.linear_counter_reload)
        triangle.linear_counter_divider = triangle.linear_counter_load;
        
    else if(triangle.linear_counter_divider != 0)
        triangle.linear_counter_divider--;
    
    if(!triangle.length_counter_halt)
        triangle.linear_counter_reload = false;
}

void APU::tick_triangle_timer()
{
    if(triangle.divider == 0)
    {
        triangle.divider = triangle.timer;
        if((triangle.length_counter_load > 0) && (triangle.linear_counter_divider > 0) && (triangle.timer > 2) && (triangle.timer < 0x7FE))
            triangle.sequence_step = (triangle.sequence_step + 1) & 31;
    }
    else
        triangle.divider--;   
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

    if(!noise.start_flag)
    {
        if(noise.envelope_divider == 0)
        {
            noise.envelope_divider = noise.volume;
            if(noise.envelope_decay_level_counter != 0)
                noise.envelope_decay_level_counter--;
            else if(noise.envelope_loop)
                    noise.envelope_decay_level_counter = 15;          
        }
        else
            noise.envelope_divider--;
    }
    else
    {
        noise.start_flag = false;
        noise.envelope_decay_level_counter = 15;
        noise.envelope_divider = noise.volume;
    }
}

void APU::tick_length_counter()
{
    if(!pulse1.envelope_loop && (pulse1.length_counter_load != 0))
        pulse1.length_counter_load--;

    if(!pulse2.envelope_loop && (pulse2.length_counter_load != 0))
        pulse2.length_counter_load--;

    if(!triangle.length_counter_halt && (triangle.length_counter_load != 0))
        triangle.length_counter_load--;
    
    if(!noise.envelope_loop && (noise.length_counter_load != 0))
        noise.length_counter_load--;
}

void APU::tick_timers()
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

    if(noise.timer_divider == 0)
    {
        noise.timer_divider = noise.timer;
        noise.feedback = (noise.shift_register & 0x1) ^ ((noise.loop_noise == 0) ? 
                                                        ((noise.shift_register & 0x2) >> 1) : //Bit 1
                                                        ((noise.shift_register & 0x40) >> 6)); //bit 6

        noise.shift_register = noise.shift_register >>  1;
        noise.shift_register = (noise.shift_register & 0x3FFF) | (noise.feedback << 14);
    }
    else
        noise.timer_divider--;

    // DMC Output Unit
    if (dmc.timer_divider == 0)
    {
        dmc.timer_divider = dmc.rate;

        if (!dmc.silence)
        {
            if (dmc.shift_register & 1)
            {
                if (dmc.output_level <= 125)
                    dmc.output_level += 2;
            }
            else
            {
                if (dmc.output_level >= 2)
                    dmc.output_level -= 2;
            }
        }
        
        dmc.shift_register >>= 1;
        dmc.bits_remaining--;
        
        if (dmc.bits_remaining == 0)
        {
            dmc.bits_remaining = 8;
            if (dmc.sample_buffer == 0)
            {
                dmc.silence = true;
            }
            else
            {
                dmc.silence = false;
                dmc.shift_register = dmc.sample_buffer;
                dmc.sample_buffer = 0;
            }
        }
    }
    
    else
        dmc.timer_divider--;
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

double prev_output_hp_90 = 0;
double prev_output_hp_440 = 0;
double prev_output_lp_14000 = 0;

double high_pass_filter(double input, double& prev_output, double cutoff_freq, double sample_rate = 44100.0) 
{
    double alpha = 1.0 / (1.0 + (2.0 * M_PI * cutoff_freq / sample_rate));
    double output = input - prev_output + alpha * prev_output;
    prev_output = output;
    return output;
}

// Function to apply a first-order low-pass filter.
double low_pass_filter(double input, double& prev_output, double cutoff_freq, double sample_rate = 44100.0) 
{
    double alpha = 1.0 / (1.0 + (2.0 * M_PI * cutoff_freq / sample_rate));
    double output = alpha * input + (1.0 - alpha) * prev_output;
    prev_output = output;
    return output;
}

double APU::get_output()
{
    float pulse1_output = 0;
    float pulse2_output = 0;
    float pulse_output = 0;
    float tnd_output = 0;
    float triangle_sample = 0;
    float noise_sample = 0;
    float triangle_output = 0;
    float noise_output = 0;

    //Calculate pulses output
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
    
    if(pulse1_output != 0 || pulse2_output != 0)
        pulse_output = (95.88 / ((8128 / (pulse1_output + pulse2_output)) + 100));

    //Calculate triangle and noise output
    triangle_sample = triangle_sequence[triangle.sequence_step];
    if(!(noise.shift_register & 0x1) && noise.length_counter_load != 0)
    {
        if(noise.const_volume)
            noise_sample = noise.volume;
        else
            noise_sample = noise.envelope_decay_level_counter;      
    } 

    if(triangle_sample != 0)
        triangle_output = triangle_sample / 8227;
        
    if(noise_sample != 0)
        noise_output = noise_sample / 12241;

    if((noise_output != 0) || (triangle_output != 0))
        tnd_output = 159.79 / ((1.0 / (triangle_output + noise_output)) + 100.0);

    
    // Apply the filters:
    double filtered_output = tnd_output + pulse_output;
    filtered_output = high_pass_filter(filtered_output, prev_output_hp_90, 90.0);  // High-pass at 90 Hz
    filtered_output = high_pass_filter(filtered_output, prev_output_hp_440, 440.0); // High-pass at 440 Hz
    filtered_output = low_pass_filter(filtered_output, prev_output_lp_14000, 14000.0); // Low-pass at 14 kHz

    return filtered_output;
}

void APU::connect_bus(std::shared_ptr<Bus> bus)
{
    this->bus = bus;
}

void APU::set_timing(bool value)
{
    region = value;
}

void APU::soft_reset()
{
    // Reset Pulse 1 state
    pulse1.start_flag = false;
    pulse1.envelope_divider = 0;
    pulse1.envelope_decay_level_counter = 0;
    pulse1.volume = 0;
    pulse1.sweep_divider_counter = 0;
    pulse1.reload_flag = false;
    pulse1.negate = 0;
    pulse1.shift = 0;
    pulse1.sweep_unit_enabled = false;
    pulse1.period = 0;
    pulse1.target_period = 0;
    pulse1.duty = 0;
    pulse1.envelope_loop = false;
    pulse1.const_volume = false;
    pulse1.length_counter_load = 0;
    pulse1.timer_divider = 0;
    pulse1.timer = 0;
    pulse1.sequence_step = 7;
    pulse1.sequencer_output = 0;

    // Reset Pulse 2 state (similar to Pulse 1)
    pulse2.start_flag = false;
    pulse2.envelope_divider = 0;
    pulse2.envelope_decay_level_counter = 0;
    pulse2.volume = 0;
    pulse2.sweep_divider_counter = 0;
    pulse2.reload_flag = false;
    pulse2.negate = 0;
    pulse2.shift = 0;
    pulse2.sweep_unit_enabled = false;
    pulse2.period = 0;
    pulse2.target_period = 0;
    pulse2.duty = 0;
    pulse2.envelope_loop = false;
    pulse2.const_volume = false;
    pulse2.length_counter_load = 0;
    pulse2.timer_divider = 0;
    pulse2.timer = 0;
    pulse2.sequence_step = 7;
    pulse2.sequencer_output = 0;

    // Reset Triangle state
    triangle.linear_counter_load = 0;
    triangle.length_counter_halt = false;
    triangle.length_counter_load = 0;
    triangle.linear_counter_reload = false;
    triangle.timer = 0;
    triangle.divider = 0;
    triangle.sequence_step = 0;
    triangle.linear_counter_divider = 0;

    // Reset Noise state
    noise.start_flag = false;
    noise.envelope_divider = 0;
    noise.envelope_decay_level_counter = 0;
    noise.volume = 0;
    noise.const_volume = false;
    noise.envelope_loop = false;
    noise.noise_period = 0;
    noise.loop_noise = false;
    noise.length_counter_load = 0;
    noise.timer = 0;
    noise.timer_divider = 0;
    noise.shift_register = 1;
    noise.feedback = 0;

    // Reset APU control state
    status_register = 0;
    sequence_mode = false;
    inhibit_flag = 0;
    sequence_step = 0;
    delay_write_to_frame_counter = 0;
    reset = false;

    // Reset APU cycle counter
    apu_cycles_counter = 0.0;

    // Reset all lookup tables and sequences
    region = 0;
}
