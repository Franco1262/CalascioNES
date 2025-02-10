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
    uint8_t sequence_step = 0;
    bool sequencer_output = 0;
};

class Bus;
class APU
{
    public:
        APU();
        ~APU();
        void cpu_writes(uint16_t address, uint8_t value);
        void connect_bus(std::shared_ptr<Bus> bus);
        void tick();
        void set_timing(bool value);
        double get_output();

    private:
        void tick_envelope();
        void tick_length_counter();
        void tick_frame_counter();
        void tick_pulse_timer();
        void tick_sweep();
        void calculate_target_period_pulse(Pulse &pulse, int npulse);

        std::vector<uint8_t> sequence_lookup_table;
        std::vector<uint8_t> length_counter_lookup_table;
        
        float apu_cycles_counter = 0.0;
        std::shared_ptr<Bus> bus;
        Pulse pulse1, pulse2;
        uint8_t status_register = 0;
        bool sequence_mode = 0;
        bool inhibit_flag = 0;
        uint8_t sequence_step = 1;
        bool region = 0; //0 NTSC |  1 PAL

        uint8_t delay_write_to_frame_counter = 0.0;
        bool reset = false;

};