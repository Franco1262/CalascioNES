#include <iostream>
#include <memory>

struct Pulse
{
    uint8_t duty = 0;
    bool envelope_loop = 0;
    bool const_volume = 0;
    uint8_t volume = 0;
    bool sweep_unit_enabled = 0;
    uint8_t period = 0;
    bool negate = 0;
    uint8_t shift = 0;
    uint8_t timer_low = 0;
    uint8_t length_counter_load = 0;
    uint8_t timer_high = 0;
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
        void tick_envelope();
        void tick_length_counter();
    private:
        std::shared_ptr<Bus> bus;
        Pulse pulse1, pulse2;
        uint8_t status_register = 0;
        bool sequence_mode = 0;
        bool inhibit_flag = 0;
        uint8_t sequence_step = 1;  
};