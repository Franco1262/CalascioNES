#include <iostream>
#include <cstdint>
#include <fstream>
#include <vector>
#include <memory>
#include "NROM.h"
#include "UxROM.h"
#include "CNROM.h"
#include "SxROM.h"
#include "AxROM.h"
#include "TxROM.h"

class Bus;
class Cartridge : public std::enable_shared_from_this<Cartridge>
{
    public:
        Cartridge();
        ~Cartridge();
        uint8_t ppu_reads(uint16_t address);
        uint8_t cpu_reads(uint16_t address);
        void ppu_writes(uint16_t address, uint8_t value);
        void cpu_writes(uint16_t address, uint8_t value);
        bool is_new_instruction();
        bool load_game(std::string filename, std::string& log);
        void soft_reset();
        MIRROR getMirror();
        void connect_bus(std::shared_ptr<Bus> bus);

        std::shared_ptr<Cartridge> get_shared() 
        {
            return shared_from_this();
        }

        void set_irq_latch(uint8_t value);
        void set_irq_enable(bool);
        void set_irq_reload();
        uint8_t get_mapper();
       
    private:
        std::vector<uint8_t> CHR_ROM; 
        std::vector<uint8_t> PRG_ROM;
        std::vector<uint8_t> CHR_RAM;
        std::vector<uint8_t> PRG_RAM;
        std::unique_ptr<Mapper> mapper;
        std::shared_ptr<Bus> bus;

        struct Header
        {
            uint8_t id_string[4] = {0};
            uint8_t prg_rom_lsb = 0x00;
            uint8_t chr_rom_lsb = 0x00;
            uint8_t flag6 = 0x00;
            uint8_t flag7 = 0x00;
            uint8_t mapper = 0x00;
            uint8_t prg_chr_rom_size = 0x00;
            uint8_t prg_ram_shift = 0x00;
            uint8_t chr_ram_shift = 0x00;
            uint8_t ppu_type = 0;
            uint8_t misc[3] = {0}; //Unused for now
        } header;

        uint16_t n_prg_rom_banks = 0;
        uint16_t n_chr_rom_banks = 0;
        uint16_t mapper_id;
        MIRROR mirror_mode;
};