#pragma once
#include <iostream>
#include <cstdint>
#include "Mapper.h"
#include "Logger.h"
#include <string>

class Cartridge;
class TxROM : public Mapper
{
    public:
        TxROM(int n_prg_rom_banks, int n_chr_rom_banks, std::shared_ptr<Cartridge> cart);
        ~TxROM() override { };
        uint32_t cpu_reads(uint16_t address);
        uint32_t ppu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value);
        MIRROR get_mirroring_mode();
    private:
        uint8_t select_bank;
        uint8_t R0;
        uint8_t R1;
        uint8_t R2;
        uint8_t R3;
        uint8_t R4;
        uint8_t R5;
        uint8_t R6;
        uint8_t R7;
        bool prg_rom_bank_mode;
        bool chr_inversion;
        MIRROR mirroring_mode;
        uint8_t irq_latch;
        uint8_t irq_counter = 0;
        bool irq_enable;
        bool irq_reload;
        Logger logger;
};