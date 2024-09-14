#include <iostream>
#include <cstdint>
#include <memory>
#include "Mapper.h"

class PPU;
class CPU;
class Cartridge;

class Bus
{
    public:
        Bus(std::shared_ptr<PPU> ppu,  std::shared_ptr<Cartridge> cart);
        ~Bus();

        uint8_t cpu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value);

        uint8_t ppu_reads(uint16_t address);

        void set_nmi(bool value);
        bool get_nmi();

        MIRROR getMirror();

        void set_input(uint8_t state);
    private:
        std::shared_ptr<PPU> ppu;
        std::shared_ptr<Cartridge> cart;

        bool NMI = false;
        uint8_t controller_state;
        uint8_t shift_register_controller;
        bool strobe;


};