// Standard Library Headers
#include <filesystem>
#include "NES.h"

// Clock Rates
const double MASTER_CLOCK_NTSC = 236250000.0 / 11.0;
const double MASTER_CLOCK_PAL = 26601712.5;
const double CPU_CLOCK_NTSC = MASTER_CLOCK_NTSC / 12.0;
const double CPU_CLOCK_PAL = MASTER_CLOCK_PAL / 16.0;
const double SAMPLE_RATE = 44100.0;

// APU Ratios
const double apu_ratio_NTSC = CPU_CLOCK_NTSC / SAMPLE_RATE;
const double apu_ratio_PAL = CPU_CLOCK_PAL / SAMPLE_RATE;

NES::NES()
{
    cpu = std::make_shared<CPU>();
    ppu = std::make_shared<PPU>();
    cart = std::make_shared<Cartridge>();
    apu = std::make_shared<APU>();
    bus = std::make_shared<Bus>(ppu, cart, apu, cpu);

    cpu->connect_bus(bus);
    cart->connect_bus(bus);
    ppu->connect_bus(bus);
    apu->connect_bus(bus);
}

bool NES::load_game(std::string filename)
{
    reset_flag = true;
    std::string extension = std::filesystem::path(filename).extension().string();
    if(extension == ".nes" || extension == ".NES")
    {
        old_game_filename = filename;           
        reset();
        if(cart->load_game(filename, log))
            game_loaded = true; 
        if(std::filesystem::is_regular_file(log))
        {
            log = std::filesystem::path(log).stem().string();
            if(log.length() > 50)
                log = log.substr(0, 50) + "...";
            game_title = log;
        }
    }

    else
        log = std::string("File does not have .nes extension");
    reset_flag = false;
    return game_loaded;
}

void NES::run_frame()
{
    current_frame = ppu->get_frame();

    while (current_frame == ppu->get_frame() && !pause && !reset_flag) 
    {          
        cpu->tick();
        apu->tick();

        apu_cycle_accumulator += 1;
        double apu_ratio = region ? apu_ratio_PAL : apu_ratio_NTSC;
        if (apu_cycle_accumulator >= apu_ratio)
        {
            double alpha = apu_cycle_accumulator - apu_ratio;
            double previous_sample = last_sample;
            double current_sample = apu->get_output();
            
            // Linear interpolation to fill holes in audio
            double interpolated_sample = (previous_sample * (1.0 - alpha)) +( current_sample * alpha);
            {
                audio_buffer[*write_pos] = interpolated_sample * 32767;
                *write_pos = (*write_pos+1) & (buffer_size - 1);
            }

            last_sample = current_sample;
            apu_cycle_accumulator -= apu_ratio;
        }

        //Depending on the region, after every cpu tick, the ppu will tick either 3 or 3.2 times
        if (!region)  // NTSC
        {
            ppu->tick();
            ppu->tick();
            ppu->tick();
        } 

        else // PAL
        {
            ppu_accumulator += 3.2;
            while (ppu_accumulator >= 1.0)
            {
                ppu->tick();
                ppu_accumulator -= 1.0;
            }
        }     
    }
}

void NES::change_pause(SDL_AudioDeviceID audio_device)
{
    pause = !pause;
    SDL_PauseAudioDevice(audio_device, pause);
}

void NES::change_timing()
{
    region = !region;
    region_info = (region) ? "PAL" : "NTSC";
    ppu->set_ppu_timing(region);
    apu->set_timing(region);                       
}

bool NES::is_game_loaded()
{
    return game_loaded;
}

std::shared_ptr<PPU> NES::get_ppu()
{
    return ppu;
}

std::shared_ptr<CPU> NES::get_cpu()
{
    return cpu;
}

void NES::reset()
{
    cpu->soft_reset();
    ppu->soft_reset();
    cart->soft_reset();
    bus->soft_reset();
    apu->soft_reset();
    game_loaded = false;
    ppu_accumulator = 0;
    pause = false;
    log = "";
    region = 0;
    region_info = "NTSC";
    
}

void NES::reload_game()
{
    reset_flag = true;
    reset();
    load_game(old_game_filename);
    reset_flag = false;
}

void NES::alternate_zapper()
{
    zapper_connected = !zapper_connected;
    bus->set_zapper(zapper_connected);
}

bool NES::get_zapper()
{
    return zapper_connected;
}

void NES::send_mouse_coordinates(int x, int y)
{
    bus->update_zapper_coordinates(x, y);
}

void NES::fire_zapper()
{
    bus->fire_zapper();
}

std::string NES::get_log()
{
    return log;
}

bool NES::get_region()
{
    return region;
}

std::string NES::get_info()
{
    std::string info = " | Game: " + game_title + " | Active region: " + region_info;
    return info;
}

bool NES::set_audio_buffer(int16_t *buffer, int BUFFER_SIZE, uint16_t *WRITE_POS)
{
    audio_buffer = buffer;
    buffer_size = BUFFER_SIZE;
    write_pos = WRITE_POS;
    return true;
}