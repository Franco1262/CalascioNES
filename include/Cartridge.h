#include <iostream>
#include <cstdint>
#include <fstream>
#include <vector>
#include <memory>
#include "NROM.h"
#include "UxROM.h"

class PPU;
class Cartridge
{
    public:
        Cartridge(const char* filename);
        ~Cartridge();
        uint8_t ppu_reads(uint16_t address);
        uint8_t cpu_reads(uint16_t address);
        void ppu_writes(uint16_t address, uint8_t value);
        MIRROR getMirror();
        void cpu_writes(uint16_t address, uint8_t value);
       
    private:
        std::vector<uint8_t> CHR_ROM; 
        std::vector<uint8_t> PRG_ROM;
        std::vector<uint8_t> CHR_RAM;
        std::unique_ptr<Mapper> mapper;

        struct Header
        {
            uint8_t id_string[4];
            uint8_t prg_rom_lsb = 0x00;
            uint8_t chr_rom_lsb = 0x00;
            uint8_t flag6 = 0x00;
            uint8_t flag7 = 0x00;
            uint8_t mapper = 0x00;
            uint8_t prg_chr_rom_size = 0x00;
            uint8_t prg_ram_size = 0x00;
            uint8_t chr_ram_shift = 0x00;
            uint8_t misc[4]; //Unused for now
        } header;

        uint16_t n_prg_rom_banks = 1;
        uint16_t n_chr_rom_banks = 1;
        uint16_t mapper_id;
        MIRROR mirror_mode;
};