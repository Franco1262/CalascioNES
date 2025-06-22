#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "Cartridge.h"
#include "Bus.h"


class NES
{
    public:
        NES();
        bool load_game(std::string filename);
        void run_frame();
        void change_pause(SDL_AudioDeviceID audio_device);
        void change_timing();
        bool is_game_loaded();
        std::shared_ptr<PPU> get_ppu();
        std::shared_ptr<CPU> get_cpu();
        void reset();
        void reload_game();
        void alternate_zapper();
        bool get_zapper();
        void send_mouse_coordinates(int x, int y);
        void fire_zapper();
        std::string get_log();
        bool get_region();
        std::string get_info();
        bool set_audio_buffer(int16_t *buffer, int BUFFER_SIZE, uint16_t* write_pos);
        int get_write_pos();

    private:
        std::shared_ptr<CPU> cpu;
        std::shared_ptr<PPU> ppu;
        std::shared_ptr<APU> apu;
        std::shared_ptr<Cartridge> cart;
        std::shared_ptr<Bus> bus;
        bool current_frame;
        float ppu_accumulator = 0.0;
        bool region = 0; // 0: NTSC, 1: PAL
        bool pause = false;
        bool game_loaded = false;
        std::string old_game_filename;
        std::string log;
        bool zapper_connected = false;
        double apu_cycle_accumulator = 0;
        double last_sample = 0;
        bool reset_flag = false;
        std::string game_title = "";
        std::string region_info = "NTSC";
        int16_t *audio_buffer;
        int buffer_size;
        uint16_t *write_pos;
};