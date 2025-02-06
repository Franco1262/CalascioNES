#include <iostream>
#include <cstdint>
#include "Mapper.h"
#include "Logger.h"
#include <sstream>

class SxROM : public Mapper
{
    public:
        SxROM(int n_prg_rom_banks, int n_chr_rom_banks);
        ~SxROM() override { };
        uint32_t cpu_reads(uint16_t address);
        uint32_t ppu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value);
        MIRROR get_mirroring_mode();
        void update_state();
        void new_instruction();
    private:
        uint8_t load_register = 0;
        uint8_t control = 0;
        uint8_t chr_bank_0 = 0;
        uint8_t chr_bank_1 = 0;
        uint8_t prg_bank = 0;
        uint8_t shift_register = 0;
        short n_write = 0;
        uint8_t prg_rom_mode = 0;
        bool chr_rom_mode = 0;

        MIRROR mirroring_mode = MIRROR::HORIZONTAL;
        bool written_on_this_instruction = 0;
};