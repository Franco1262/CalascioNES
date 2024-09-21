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

        bool iNESFormat=false;
        if (header.id_string[0]=='N' && header.id_string[1]=='E' && header.id_string[2]=='S' && header.id_string[3]==0x1A)
                iNESFormat=true;


        bool NES20Format=false;
        if (iNESFormat==true && (header.flag7 & 0x0C) ==0x08)
                NES20Format=true; 

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
        if(n_chr_rom_banks != 0)
        {
            CHR_ROM.reserve(n_chr_rom_banks * CHR_ROM_BANK_SIZE);
            CHR_ROM.resize(n_chr_rom_banks * CHR_ROM_BANK_SIZE);
        }
        else
        {   
            int chr_ram_size;
            if(NES20Format)
                chr_ram_size = 64 << (header.chr_ram_shift & 0x0F);
            else
                chr_ram_size = 8192;
            
            CHR_RAM.reserve(chr_ram_size);
            CHR_RAM.resize(chr_ram_size);          
        }

        //Reading PRG-ROM
        if(!file.read(reinterpret_cast<char*> (PRG_ROM.data()), n_prg_rom_banks * 0x4000))
            throw std::runtime_error("There was a problem reading PRG-ROM");
        //Reading CHR-ROM  
        if(!file.read(reinterpret_cast<char*> (CHR_ROM.data()), n_chr_rom_banks * 0x2000))
            throw std::runtime_error("There was a problem reading CHR-ROM");

        //Calculating mapper_id
        mapper_id = (((header.flag6 & 0xF0) >> 4) | (header.flag7 & 0xF0) | ((header.mapper & 0x0F) << 8));
        //Selecting the mapper
        switch(mapper_id)
        {
            case 0: { mapper = std::make_unique<NROM>(n_prg_rom_banks, n_chr_rom_banks); break; }
            case 2: { mapper = std::make_unique<UxROM>(n_prg_rom_banks, n_chr_rom_banks); break; }
            
            default: {throw std::runtime_error("Unsupported mapper"); break;}
        }
    }

    else
        throw std::runtime_error("There was an error opening the file");
} 


Cartridge::~Cartridge() { }

uint8_t Cartridge::ppu_reads(uint16_t address)
{
    uint8_t data;
    if(n_chr_rom_banks != 0)
        data = CHR_ROM[mapper->ppu_reads(address)];
    else
        data = CHR_RAM[mapper->ppu_reads(address)];
    return data;
}

void Cartridge::ppu_writes(uint16_t address, uint8_t value)
{
    CHR_RAM[address] = value;
}

uint8_t Cartridge::cpu_reads(uint16_t address)
{
    uint8_t data;

    switch (mapper_id)
    {
    case 0:
        data = PRG_ROM[mapper->cpu_reads(address)];
        break;
    case 2:
        if(address >= 0xC000 && address <= 0xFFFF)
        {
            data = PRG_ROM[((n_prg_rom_banks-1)*0x4000) +  mapper->cpu_reads(address)];
        }
        else if(address >= 0x8000 && address <= 0xBFFF)
        {
            data = PRG_ROM[(mapper->get_bank_number() * 0x4000) + mapper->cpu_reads(address)];
        }
        break;

    }
   
    return data;
}

void Cartridge::cpu_writes(uint16_t address, uint8_t value)
{
    if(address >= 0x8000 && address <= 0xFFFF)
    {
        mapper->cpu_writes(address, value);
    }
}

MIRROR Cartridge::getMirror()
{
    return mirror_mode;
}