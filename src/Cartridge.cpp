#include "Cartridge.h"
#include "PPU.h"

const int PRG_ROM_BANK_SIZE = 0x4000;
const int CHR_ROM_BANK_SIZE = 0x2000;

Cartridge::Cartridge(const char *filename) : mapper(nullptr)
{
    std::ifstream file;
    file.open(filename, std::ios::binary | std::ios::in);
    if (file.is_open())
    {
        if(!file.read(reinterpret_cast<char*> (&header), sizeof(header)))
            throw std::runtime_error("There was a problem reading the header" + (std::string)filename);

        //Reading the mirror mode of the cartridge
        mirror_mode = static_cast<MIRROR>(header.flag6 & 0x01);
        //Implement multiplier notation later
        n_prg_rom_banks = (header.prg_rom_lsb | ((header.prg_chr_rom_size & 0x0F) << 8)) & 0xFFF;
        n_chr_rom_banks = (header.chr_rom_lsb | ((header.prg_chr_rom_size & 0xF0) << 8)) & 0xFFF;

        if(header.flag6 & 0x4)
            std::cout << "Trainer";
        
        //Reserving memory for the PRG-ROM 
        PRG_ROM.reserve(n_prg_rom_banks * PRG_ROM_BANK_SIZE);
        PRG_ROM.resize(n_prg_rom_banks * PRG_ROM_BANK_SIZE);
        //Reserving memory for the CHR-ROM
        CHR_ROM.reserve(n_chr_rom_banks * CHR_ROM_BANK_SIZE);
        CHR_ROM.resize(n_chr_rom_banks * CHR_ROM_BANK_SIZE);

        //Reading PRG-ROM
        if(!file.read(reinterpret_cast<char*> (PRG_ROM.data()), n_prg_rom_banks * 0x4000))
            throw std::runtime_error("There was a problem reading PRG-ROM");
        //Reading CHR-ROM  
        if(!file.read(reinterpret_cast<char*> (CHR_ROM.data()), n_chr_rom_banks * 0x2000))
            throw std::runtime_error("There was a problem reading CHR-ROM");

        //Calculating mapper_id
        mapper_id = ((header.flag6 & 0xF0) | ((header.flag7 & 0xF0) << 4) | ((header.mapper) << 8)) & 0xFFF;

        //Selecting the mapper
        switch(mapper_id)
        {
            case 0: { mapper = std::make_unique<NROM>(n_prg_rom_banks, n_chr_rom_banks); break; }
            
            default: {throw std::runtime_error("Unsupported mapper"); break;}
        }
    }

    else
        throw std::runtime_error("There was an error opening the file");
} 


Cartridge::~Cartridge() { }

uint8_t Cartridge::ppu_reads(uint16_t address)
{
    return CHR_ROM[mapper->ppu_reads(address)];
}

uint8_t Cartridge::cpu_reads(uint16_t address)
{
    return PRG_ROM[mapper->cpu_reads(address)];
}

MIRROR Cartridge::getMirror()
{
    return mirror_mode;
}