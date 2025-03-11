#include <iostream>
#include <memory>
#include <vector>

struct Pulse
{
    //Envelope unit
    bool start_flag = false;
    uint8_t envelope_divider = 0;
    uint8_t envelope_decay_level_counter = 0;
    uint8_t volume = 0; //envelope

    //Sweep unit
    uint8_t sweep_divider_counter = 0;
    bool reload_flag = false;
    bool negate = 0;
    uint8_t shift = 0;
    bool sweep_unit_enabled = 0;
    uint8_t period = 0;
    uint16_t target_period = 0; // calculated by the sweep unit

    uint8_t duty = 0;
    bool envelope_loop = 0;
    bool const_volume = 0;


    uint8_t length_counter_load = 0;
    uint16_t timer_divider = 0;
    uint16_t timer = 0;
    uint8_t sequence_step = 7;
    bool sequencer_output = 0;
};

struct Triangle
{
    uint8_t linear_counter_load = 0;
    bool length_counter_halt = false;
    uint8_t length_counter_load = 0;
    bool linear_counter_reload = false;
    uint16_t timer = 0;
    uint16_t divider = 0;
    uint8_t sequence_step = 0;
    uint8_t linear_counter_divider = 0;
};

struct Noise
{
    //Envelope unit
    bool start_flag = false;
    uint8_t envelope_divider = 0;
    uint8_t envelope_decay_level_counter = 0;
    uint8_t volume = 0; //envelope
    bool const_volume = false;

    bool envelope_loop = false;
    uint8_t noise_period = 0;
    bool loop_noise = false;
    uint8_t length_counter_load = 0;
    uint16_t timer = 0;
    uint16_t timer_divider = 0;
    uint16_t shift_register = 1;
    uint16_t feedback = 0;
};

struct DMC
{
    bool IRQ = false;
    bool loop = false;
    uint8_t rate_index = 0;
    uint16_t rate = 0;
    uint8_t output_level = 0;
    uint8_t direct_load = 0;
    uint16_t sample_address = 0;
    uint16_t sample_length = 0;
    uint16_t timer = 0;
    uint16_t timer_divider = 0;

    //Memory reader
    uint8_t sample_buffer = 0;
    uint16_t bytes_remaining = 0;
    uint16_t current_address = 0;

    //Output unit
    uint8_t shift_register = 0;
    uint8_t bits_remaining = 0;
    bool silence = false;


};

class Bus;
class APU
{
    public:
        APU();
        ~APU();
        void cpu_writes(uint16_t address, uint8_t value);
        uint8_t cpu_reads(uint16_t address);
        void connect_bus(std::shared_ptr<Bus> bus);
        void tick();
        void set_timing(bool value);
        void soft_reset();
        double get_output();

    private:
        void tick_envelope();
        void tick_length_counter();
        void tick_frame_counter();
        void tick_timers();
        void tick_sweep();
        void calculate_target_period_pulse(Pulse &pulse, int npulse);
        void tick_triangle_timer();
        void tick_linear_counter();

        std::vector<uint8_t> sequence_lookup_table;
        std::vector<uint8_t> length_counter_lookup_table;
        std::vector<uint8_t> triangle_sequence;
        std::vector<uint16_t> ntsc_noise_period;
        std::vector<uint16_t> pal_noise_period;
        std::vector<uint16_t> ntsc_dpcm_period;
        std::vector<uint16_t> pal_dpcm_period;

        float apu_cycles_counter = 0.0;
        std::shared_ptr<Bus> bus;
        Pulse pulse1, pulse2;
        Triangle triangle;
        Noise noise;
        DMC dmc;
        uint8_t status_register = 0;
        bool sequence_mode = 0;
        bool inhibit_flag = 0;
        uint8_t sequence_step = 0;
        bool region = 0; //0 NTSC |  1 PAL

        uint8_t delay_write_to_frame_counter = 0.0;
        bool reset = false;
};