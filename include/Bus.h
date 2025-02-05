#include <iostream>
#include <cstdint>
#include <memory>
#include "Mapper.h"
#include "SDL.h"

class PPU;
class CPU;
class Cartridge;

struct Zapper
{
    bool trigger = 0;
    bool light_sensed = 1;
    int x;
    int y;

};

class Bus
{
    public:
        Bus(std::shared_ptr<PPU> ppu,  std::shared_ptr<Cartridge> cart);
        ~Bus();

        uint8_t cpu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value);

        uint8_t ppu_reads(uint16_t address);
        void ppu_writes(uint16_t address, uint8_t value);

        void set_nmi(bool value);
        bool get_nmi();

        void new_instruction();

        MIRROR getMirror();

        void set_input(uint16_t state);
        bool get_input();
        void set_zapper(bool);
        void update_zapper_coordinates(int x, int y);
        void fire_zapper();
        void set_light_sensed(bool hit);
        void soft_reset();

        void apu_irq();
        
    private:
        std::shared_ptr<PPU> ppu;
        std::shared_ptr<Cartridge> cart;

        bool NMI = false;
        uint16_t controller_state;
        uint16_t shift_register_controller1;
        uint16_t shift_register_controller2;
        bool zapper_connected = false;
        bool handle_input;
        bool strobe;

        Zapper zapper;
};