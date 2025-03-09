#pragma once
#include <memory>

const int PRG_ROM_BANK_SIZE_16KB = 0x4000;
const int PRG_ROM_BANK_SIZE_32KB = 0x8000;
const int PRG_ROM_BANK_SIZE_8KB = 0x2000;
const int CHR_ROM_BANK_SIZE_8KB = 0x2000;
const int CHR_ROM_BANK_SIZE_4KB = 0x1000;
const int CHR_ROM_BANK_SIZE_1KB = 0x400;
const int CHR_ROM_BANK_SIZE_2KB = 0x800;

enum MIRROR
{
    HORIZONTAL,
    VERTICAL ,
    ONE_SCREEN_LOWER,
    ONE_SCREEN_UPPER,
    FOUR_SCREEN
};

class Cartridge;
class Mapper
{
    public:
         Mapper(int n_prg_rom_banks, int n_chr_rom_banks, std::shared_ptr<Cartridge> cart)
        {
            this->n_prg_rom_banks = n_prg_rom_banks;
            this->n_chr_rom_banks = n_chr_rom_banks;
            this->cart = cart;
        };
        
        virtual ~Mapper() { };
        virtual uint32_t cpu_reads(uint16_t address) = 0;
        virtual uint32_t ppu_reads(uint16_t address) = 0;
        virtual void cpu_writes(uint16_t address, uint8_t value) = 0;
    protected:
        int n_prg_rom_banks;
        int n_chr_rom_banks;
        std::shared_ptr<Cartridge> cart;
        uint32_t mapped_address = 0;
};