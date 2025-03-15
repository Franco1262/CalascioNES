#include <iostream>
#include <cstdint>
#include <memory>
#include "Mapper.h"
#include "SDL.h"

struct Zapper
{
    bool trigger = 0;
    bool light_sensed = 1;
    int x;
    int y;

};

enum IRQ
{
    MMC3 = 1,
    Frame_IRQ = 2,
    DMC_IRQ = 4
};


class PPU;
class CPU;
class APU;
class Cartridge;
class Bus : public std::enable_shared_from_this<Bus> 
{
    public:
        Bus(std::shared_ptr<PPU> ppu,  std::shared_ptr<Cartridge> cart, std::shared_ptr<APU> apu, std::shared_ptr<CPU> cpu);
        ~Bus();

        uint8_t cpu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value);

        uint8_t ppu_reads(uint16_t address);
        void ppu_writes(uint16_t address, uint8_t value);

        void set_nmi(bool value);
        bool is_new_instruction();

        void set_input(uint16_t state);
        bool get_input();
        void set_zapper(bool);
        void update_zapper_coordinates(int x, int y);
        void fire_zapper();
        void set_light_sensed(bool hit);
        void soft_reset();
        void assert_irq(IRQ);
        void ack_irq(IRQ);
        uint8_t get_irq();

        std::shared_ptr<Bus> get_shared() 
        {
            return shared_from_this();
        }

        void set_irq_latch(uint8_t value);
        void set_irq_enable(bool);
        void set_irq_reload();
        void set_mapper(uint8_t value);
        void set_mirroring_mode(MIRROR);
        
    private:
        std::shared_ptr<PPU> ppu;
        std::shared_ptr<APU> apu;
        std::shared_ptr<Cartridge> cart;
        std::shared_ptr<CPU> cpu;

        bool NMI = false;
        uint16_t controller_state;
        uint16_t shift_register_controller1;
        uint16_t shift_register_controller2;
        bool zapper_connected = false;
        bool handle_input;
        bool strobe;
        //xxxx xDFM
        //M = MMC3, F = Frame interrupt, D = DMC IRQ
        uint8_t IRQ_line = 0;
        Zapper zapper;
};