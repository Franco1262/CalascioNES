#include "Cartridge.h"
#include "PPU.h"

const int PRG_ROM_BANK_SIZE = 0x4000;
const int CHR_ROM_BANK_SIZE = 0x2000;

Cartridge::Cartridge() : mapper(nullptr)
{

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
        data = PRG_RAM[mapper->cpu_reads(address)];


    else if(address >= 0x8000 && address <= 0xFFFF)
        data = PRG_ROM[mapper->cpu_reads(address)];
    return data;
}

void Cartridge::cpu_writes(uint16_t address, uint8_t value)
{
    if(address >= 0x6000 && address < 0x8000)
        PRG_RAM[mapper->cpu_reads(address)] = value;

        //Ver esto luego
    else if(address >= 0x8000 && address <= 0xFFFF)
        mapper->cpu_writes(address, value);  
}

MIRROR Cartridge::getMirror()
{
    if (mapper_id == 1 || mapper_id == 7) 
        mirror_mode = mapper->get_mirroring_mode();


    return mirror_mode;
}

void Cartridge::new_instruction()
{
    mapper->new_instruction();
}

bool Cartridge::load_game(const std::string filename, std::string& log)
{
    bool ok = true;
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    if (!file.is_open())
    {
        log =   std::string("Error: Could not open file ") + filename;
        ok = false;
    }

    try
    {
        // Read header
        if (!file.read(reinterpret_cast<char*>(&header), sizeof(header)))
        {
            log =   std::string("Error: Failed to read header in ") + filename;
            ok = false;
        }

        // Check format
        bool iNESFormat = (header.id_string[0] == 'N' && header.id_string[1] == 'E' && header.id_string[2] == 'S' && header.id_string[3] == 0x1A);
        bool NES20Format = (iNESFormat && (header.flag7 & 0x0C) == 0x08);

        // Set mirror mode and ROM bank counts
        mirror_mode = static_cast<MIRROR>(header.flag6 & 0x01);
        n_prg_rom_banks = (header.prg_rom_lsb | ((header.prg_chr_rom_size & 0x0F) << 8)) & 0xFFF;
        n_chr_rom_banks = (header.chr_rom_lsb | ((header.prg_chr_rom_size & 0xF0) << 8)) & 0xFFF;

        // Ignore trainer if present
        if (header.flag6 & 0x4)
            file.ignore(512);

        // Allocate memory for PRG and CHR ROM
        PRG_ROM.resize(n_prg_rom_banks * PRG_ROM_BANK_SIZE);
        uint32_t chr_size = (n_chr_rom_banks & 0xF00) == 0xF00
            ? (1 << (header.prg_chr_rom_size & 0x0F)) * ((header.chr_rom_lsb & 0x03) * 2 + 1)
            : n_chr_rom_banks * CHR_ROM_BANK_SIZE;
        
        if (n_chr_rom_banks > 0)
            CHR_ROM.resize(chr_size);
        else
            CHR_RAM.resize(NES20Format ? (64 << (header.chr_ram_shift & 0x0F)) : 8192);

        PRG_RAM.resize(0x8000);

        // Read PRG-ROM
        if (!file.read(reinterpret_cast<char*>(PRG_ROM.data()), PRG_ROM.size()))
        {
            log =  std::string("Error: Failed to read PRG-ROM in ") + filename;
            ok = false;
        }

        // Read CHR-ROM
        if (n_chr_rom_banks > 0) {
            // Get current position in the file
            std::streampos current_pos = file.tellg();

            // Move to the end to calculate remaining size
            file.seekg(0, std::ios::end);
            std::streamsize remaining_size = file.tellg() - current_pos;

            // Move back to the original position
            file.seekg(current_pos, std::ios::beg);

            // Resize CHR_ROM if necessary
            CHR_ROM.resize(static_cast<size_t>(remaining_size));

            // Read the remaining data
            if (!file.read(reinterpret_cast<char*>(CHR_ROM.data()), remaining_size)) {
                log = std::string("Error: Failed to read CHR-ROM in ") + filename;
                ok = false;
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