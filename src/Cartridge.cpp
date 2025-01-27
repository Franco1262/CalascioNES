#include "Cartridge.h"
#include "PPU.h"

const int PRG_ROM_BANK_SIZE = 0x4000;
const int CHR_ROM_BANK_SIZE = 0x2000;

Cartridge::Cartridge() : mapper(nullptr) 
{ 
    CHR_RAM.resize(0x2000);
    PRG_RAM.resize(0x8000);
} 

Cartridge::~Cartridge() { }

uint8_t Cartridge::ppu_reads(uint16_t address)
{
    uint8_t data;
    if(n_chr_rom_banks == 0)
        data = CHR_RAM[mapper->ppu_reads(address)];
       
    else
        data = CHR_ROM[mapper->ppu_reads(address)];
          
    return data;
}

void Cartridge::ppu_writes(uint16_t address, uint8_t value)
{
    if(n_chr_rom_banks == 0)
        CHR_RAM[mapper->ppu_reads(address)] = value;
}

uint8_t Cartridge::cpu_reads(uint16_t address)
{
    uint8_t data;

    if(address >= 0x6000 && address < 0x8000)
        data = PRG_RAM[mapper->cpu_reads(address) & 0x7FFF];
    
    else if(address >= 0x8000 && address <= 0xFFFF)
        data = PRG_ROM[mapper->cpu_reads(address)];
    return data;
}

void Cartridge::cpu_writes(uint16_t address, uint8_t value)
{
    if(address >= 0x6000 && address < 0x8000)
        PRG_RAM[mapper->cpu_reads(address) & 0x7FFF] = value;

    else if(address >= 0x8000 && address <= 0xFFFF)
        mapper->cpu_writes(address, value);  
}

MIRROR Cartridge::getMirror()
{
    if (mapper_id == 1 || mapper_id == 7) 
        mirror_mode = mapper->get_mirroring_mode();

    return mirror_mode;
}

void Cartridge::new_instruction() { mapper->new_instruction(); }

bool Cartridge::load_game(const std::string filename, std::string& log)
{
    bool ok = true;
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    if (!file.is_open())
    {
        log = std::string("Error: Could not open file ") + filename;
        ok = false;
        return ok;
    }

    try
    {
        // Read header
        if (!file.read(reinterpret_cast<char*>(&header), sizeof(header)))
        {
            log =   std::string("Error: Failed to read header in ") + filename;
            ok = false;
            return ok;
        }

        // Check format
        bool iNESFormat=false;
        if (header.id_string[0]=='N' && header.id_string[1]=='E' && header.id_string[2]=='S' && header.id_string[3]==0x1A)
            iNESFormat=true;

        bool NES20Format=false;
        if (iNESFormat==true && (header.flag7 & 0x0C)==0x08)
            NES20Format=true;

        if(NES20Format)
        {
            log =   std::string("Error: NES2.0 format not supported") + filename;
            ok = false;
            return ok;            
        }

        // Set mirror mode and ROM bank counts
        mirror_mode = static_cast<MIRROR>(header.flag6 & 0x01);
        n_prg_rom_banks = header.prg_rom_lsb;
        n_chr_rom_banks = header.chr_rom_lsb;

        //Allocate memory
        PRG_ROM.resize(n_prg_rom_banks * PRG_ROM_BANK_SIZE);
        CHR_ROM.resize(n_chr_rom_banks * CHR_ROM_BANK_SIZE);

        //Ignore trainer if present
        if (header.flag6 & 0x4)
        {
            log = std::string("Error: Games with trainer are not supported");
            ok = false;
            return ok;
        }

        // Read PRG-ROM
        if (!file.read(reinterpret_cast<char*>(PRG_ROM.data()), PRG_ROM.size()))
        {
            log =  std::string("Error: Failed to read PRG-ROM in ") + filename;
            ok = false;
            return ok;
        }

        // Read CHR-ROM
        if (n_chr_rom_banks > 0) 
        {
            std::streampos current_pos = file.tellg();
            file.seekg(0, std::ios::end);
            std::streamsize remaining_size = file.tellg() - current_pos;
            file.seekg(current_pos, std::ios::beg);
            CHR_ROM.resize(static_cast<size_t>(remaining_size));

            // Read the remaining data
            if (!file.read(reinterpret_cast<char*>(CHR_ROM.data()), remaining_size)) 
            {
                log = std::string("Error: Failed to read CHR-ROM in ") + filename;
                ok = false;
                return ok;
            }
        }

        // Calculate mapper ID and initialize mapper
        mapper_id = (((header.flag6 & 0xF0) >> 4) | (header.flag7 & 0xF0) | ((header.mapper & 0x0F) << 8));

        switch (mapper_id)
        {
            case 0: mapper = std::make_unique<NROM>(n_prg_rom_banks, n_chr_rom_banks); break;
            case 2: mapper = std::make_unique<UxROM>(n_prg_rom_banks, n_chr_rom_banks); break;
            case 3: mapper = std::make_unique<CNROM>(n_prg_rom_banks, n_chr_rom_banks); break;
            case 1: mapper = std::make_unique<SxROM>(n_prg_rom_banks, n_chr_rom_banks); break;
            case 7: mapper = std::make_unique<AxROM>(n_prg_rom_banks, n_chr_rom_banks); break;
            default:
                log = std::string("Error: Unsupported mapper ID ") + std::to_string((int)mapper_id);
                ok =  false;
                return ok;
        }

    }
    catch (const std::exception& e)
    {
        log = std::string("Exception: ") +  std::string(e.what());
        ok =  false;
    }

    if(ok)
        log = filename;

    return ok;
}


void Cartridge::soft_reset()
{
    CHR_RAM.clear();
    PRG_RAM.clear();
    CHR_ROM.clear();
    PRG_ROM.clear();
    mapper = nullptr;
    header = Header{};
}