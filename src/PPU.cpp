#include <iostream>
#include <cstdint>
#include "PPU.h"
#include "Bus.h"

#define IS_PPUMASK_SET(mask) (((mask) & 0x10) || ((mask) & 0x8))

PPU::PPU() : w(false) ,cycles(0), scanline(0)
{
    odd = false;
    frame = false;
    screen = std::vector<uint32_t>( 256*240 );
    scanline_buffer = std::vector<uint8_t>( 256 );
    pattern_buffer = std::vector<uint32_t>( 128*128 );
    nametable_buffer = std::vector<uint32_t>( 256 * 240 );
    sprite_buffer = std::vector<uint32_t>(64 * 64);
    n = 0;
    m = 0;
    oam_writes = true;
    secondary_oam_pos = 0;
    sprites_found = 0;
    j = 0;
    i = 0;
    palette = 0x00;
}

PPU::~PPU() {}


uint8_t PPU::cpu_reads(uint16_t address)
{
    uint8_t data;
    switch(address)
    {
        case 0: {data = PPUCTRL; break; }
        case 1: {data = PPUMASK; break; }
        case 2: 
        {

            data = PPUSTATUS;
            w = 0;
            PPUSTATUS &= 0x7F;
            vblank = false;
            break; 
        }
        case 3: {data = OAMADDR; break; }
        case 4: 
        {
            data = OAM[OAMADDR]; 
            break; 
        }
        case 5: {data = PPUSCROLL; break; }
        case 6: {data = PPUADDR; break; }
        case 7: 
        {

            data = ppudata_read_buffer;
            ppudata_read_buffer = read(v);
            increment_v_ppudata();
            break; 
        }
    }

    return data;
}

void PPU::cpu_writes(uint16_t address, uint8_t value)
{
    switch(address) 
    {
        case 0: 
        {
            PPUCTRL = value;
            t = (t & ~(0x3 << 10)) | ((PPUCTRL & 0x3) << 10);
            NMI_output = (PPUCTRL & 0x80);
            break; 
        }
        case 1: 
        {
            PPUMASK = value;
            break; 
        }
        case 2: {PPUSTATUS = value; break; }
        case 3: {OAMADDR = value; break; }
        case 4: 
        {
            OAMDATA = value;
            OAM[OAMADDR] = OAMDATA;
            OAMADDR++;
            break; 
            }
        case 5: 
        {
            PPUSCROLL = value;
            if(!w)
            {
                t = (t & ~(0xF8 >> 3)) | ((PPUSCROLL & 0xF8) >> 3);
                fine_x = PPUSCROLL & 0x07;
                w = 1;
            }
            else
            {
                t = (t & ~(0x73E0)) | ((PPUSCROLL & 0x7) << 12) | (((PPUSCROLL & 0xF8) >> 3) << 5);
                w = 0;            
            }
            break; 
        }

        case 6: 
        {
            PPUADDR = value;
            if(!w)
            {
                t = (t & ~(0x3F << 8)) | ((PPUADDR & 0x3F) << 8);
                t &= 0x3FFF;
                w = 1;
            }
            else
            {
                t = (t & 0xFF00) | PPUADDR;
                v = t;
                w = 0;
            }
            break; 
        }
        case 7: 
        {
            PPUDATA = value;
            write(v, PPUDATA);
            increment_v_ppudata();
            break; 
        }
    }    
}

uint8_t PPU::read(uint16_t address)
{
    uint8_t data;


    if((address >= 0x0000) && (address < 0x2000))
    {
        data = bus->ppu_reads(address);
    }

    else if((address >= 0x2000) && (address < 0x3000))
    {
        if(address >= 0x3000)
            address -= 0x1000;
        if(bus->getMirror() == MIRROR::HORIZONTAL)
        {
            if((address >= 0x2000) && (address < 0x2800))
                data = nametable[address & 0x3FF]; 
            
            else if((address >= 0x2800) && (address < 0x3000))          
                data = nametable[0x400 + (address & 0x3FF)];                  
        }
        
         if(bus->getMirror() == MIRROR::VERTICAL)
        {
            if( (address >= 0x2000) && (address < 0x2400))
                data = nametable[address & 0x3FF];
            else if((address >= 0x2800) && (address < 0x2C00))
                data = nametable[address & 0x3FF];
            else if((address >= 0x2400) && (address < 0x2800))
                data = nametable[0x400 + (address & 0x3FF)];
            else if (address >= 0x2C00 && address <= 0x2FFF)
                data = nametable[0x400 + (address & 0x3FF)];       
        } 
    } 

    if( (address >= 0x3F00) && (address <= 0x3FFF) )
    {
        if((address & 0x3) == 0x00)
            data = frame_palette[0x00];       
        else 
            data = frame_palette[address & 0x1F];
    }

    return data;
}

void PPU::write(uint16_t address, uint8_t value)
{
    if((address >= 0x2000) && (address < 0x3EFF))
    {
        if(address >= 0x3000)
            address -= 0x1000;
        if(bus->getMirror() == MIRROR::HORIZONTAL)
        {
            if((address >= 0x2000) && (address < 0x2800))            
                nametable[address & 0x3FF] = value;            

            else if((address >= 0x2800) && (address < 0x3000))           
                nametable[0x400 + (address & 0x3FF)] = value;                   
        }
        
         if(bus->getMirror() == MIRROR::VERTICAL)
        {
            if((address >= 0x2000) && (address < 0x2400))
                nametable[address & 0x3FF] = value;
            else if((address) >= 0x2800 && (address < 0x2C00))
                nametable[address & 0x3FF] = value;
            else if((address >= 0x2400) && (address < 0x2800))
                nametable[0x400 + (address & 0x3FF)] = value;
            else if (address >= 0x2C00 && address <= 0x2FFF)
                nametable[0x400 + (address & 0x3FF)] = value;
        } 
    }  

    if( (address >= 0x3F00) && (address <= 0x3FFF) )
    {
        if((address & 0x3) == 0x00)
            frame_palette[address & 0xF] = value;
        else 
            frame_palette[address & 0x1F] = value;
    }
}

void PPU::connect_bus(std::shared_ptr<Bus> bus)
{
    this->bus = bus;
}


std::vector<uint32_t>& PPU::get_screen()
{
    return screen;
}

void PPU::reset()
{
    PPUADDR = PPUCTRL = PPUDATA = PPUMASK = PPUSCROLL = PPUSTATUS = 0x00;
}

uint32_t PPU::get_palette_color(uint8_t palette_x, uint8_t pixel)
{
    uint32_t data;

    data = read(0x3F00 + ((pixel) | (palette_x << 2)));

    return data;
}


std::vector<uint32_t> PPU::get_pattern_table(int m)
{
    uint32_t palette_pt[4] = {0x000000FF, 0xFFFFFFFF, 0x00FFFFFF, 0x0000FFFF};
    for(int y = 0; y < 16; y++)
    {
        for(int x = 0; x < 16; x++)
        {
            for(int i = 0; i < 8; i++)
            {
                uint8_t LSB = bus->ppu_reads(m*0x1000 + y*256 + x * 16 + i);
                uint8_t MSB = bus->ppu_reads(m*0x1000 + y*256 + x * 16 + i + 8);
                for(int j = 0; j < 8; j++)
                {
                    uint8_t pixel = ((LSB & 0x80) >> 7) | (((MSB & 0x80) >> 6));
                    LSB <<= 1;
                    MSB <<= 1;
                    pattern_buffer[(((y * 8) + i) * 128) + ((x * 8) + j)] = palette_pt[pixel];
                }
            }
        }
    }

    return pattern_buffer;
}

std::vector<uint32_t> PPU::get_sprite()
{
    uint8_t tile_id;
    uint8_t palette_sprite;
    for(int y = 0; y < 8; y++)
    {
        for(int x = 0; x < 8; x++)
        {
            tile_id = OAM[ (((y*8) + x) * 4) + 1 ];
            palette_sprite = OAM[ (((y*8) + x) * 4) + 2 ] & 0x3;
            for(int i = 0; i < 8; i++)
            {                                 //Pattern table                  //Tile id                     //Fine y
                uint8_t LSB = bus->ppu_reads(((PPUCTRL & 0x8) > 0) * 0x1000 + (tile_id * 16) + i);
                uint8_t MSB = bus->ppu_reads(((PPUCTRL & 0x8) > 0) * 0x1000 + (tile_id * 16) + i + 8); // +8 bit plane
                for(int j = 0; j < 8; j++)
                {
                    uint8_t pixel = ((LSB & 0x80) >> 7) | (((MSB & 0x80) >> 6));
                    LSB <<= 1;
                    MSB <<= 1;
                    sprite_buffer[(((y * 8) + i) * 64) + ((x * 8) + j)] = system_palette[get_palette_color(palette_sprite, pixel + 0x10)];
                }
            }
        }
    } 

    return sprite_buffer;                  
} 


std::vector<uint32_t> PPU::get_nametable(int m)
{
    uint8_t nametable_id;
    for(int i = 0; i < 30; i++)
    {
        for(int j = 0; j < 32 ; j++)
        {
            nametable_id = nametable[(m*0x400) + i*32 + j];
            uint8_t attribute2 = read(0x23C0 + (bus->getMirror() > 0 ? 0x400 : 0x800)*m + j/4 + (i/4)*8);
            uint8_t x = ((j & 0x3) & 0x2) >> 1;
            uint8_t y = ((i & 0x3) & 0x2) >> 1;


            for(int k = 0; k < 8; k++)
            {
                uint8_t LSB = bus->ppu_reads(0x1000 * ((PPUCTRL & 0x10) > 0) + (nametable_id * 16) + k);
                uint8_t MSB = bus->ppu_reads(0x1000 * ((PPUCTRL & 0x10) > 0) + (nametable_id * 16) + k + 8);
                uint8_t paleta = ((attribute2 >> ((x * 2) + (y * 4))) & 0x01) > 0 ? 0xFF : 0x00;
                uint8_t paleta0 = ((attribute2 >> ((x * 2) + (y * 4) + 1)) & 0x01) > 0 ? 0xFF : 0x00;

                for(int z = 0; z < 8; z++)
                {
                    uint8_t pixel3 = ((LSB & 0x80) >> 7) | (((MSB & 0x80) >> 6));
                    uint8_t paleta1 = ((paleta & 0x80) >> 7) | (((paleta0 & 0x80) >> 6));
                    nametable_buffer[(((i*8) + k) * 256 ) + ((j * 8) + z)] = system_palette[get_palette_color(paleta1, pixel3)];
                    LSB <<= 1;
                    MSB <<= 1;
                    paleta <<= 1;
                    paleta0 <<= 1;
                }
            }           
        }
    }
    return nametable_buffer;
}

void PPU::shift_bits()
{
    bg_shift_register1 <<= 1;
    bg_shift_register <<= 1;
    palette_bit_0 <<=1;
    palette_bit_1 <<=1;
}

void inline PPU::increment_hori_v()
{
    if ((v & 0x001F) == 31) // if coarse X == 31
    {
        v &= ~0x001F;          // coarse X = 0
        v ^= 0x0400;           // switch horizontal nametable
    }
    else
        v += 1;                // increment coarse X
}

void inline PPU::increment_vert_v()
{
    if ((v & 0x7000) != 0x7000)        // if fine Y < 7
        v += 0x1000;                    // increment fine Y
    else
    {
        v &= ~0x7000;                     // fine Y = 0
        int y = (v & 0x03E0) >> 5;        // let y = coarse Y
        if (y == 29)
        {
            y = 0;                          // coarse Y = 0
            v ^= 0x0800;                    // switch vertical nametable
        }
        else if (y == 31)
            y = 0;                          // coarse Y = 0, nametable not switched
        else
            y += 1;                         // increment coarse Y
        v = (v & ~0x03E0) | (y << 5);     // put coarse Y back into v    
    }
}

void inline PPU::increment_v_ppudata()
{
    if(((scanline < 240) || (scanline == 261)) && IS_PPUMASK_SET(PPUMASK))
    {
        increment_hori_v();
        increment_vert_v();
    }

    else
    {
        if((PPUCTRL & 0x04) >> 2)
            v += 32;
        else
            v++;
    }
}

unsigned long PPU::get_frame()
{
    return frame;
}


void PPU::sprite_evaluation()
{
    //Cycle 65 resetting useful variables
    if(cycles == 65)
    {
        n = 0;
        m = 0;
        overflow_n = false;
        oam_writes = true;
        sprites_found = 0;
        secondary_oam_pos = 0;
        in_range = false;
    }

    //in odd cycles only read
    if(cycles & 0x1)
    {
        oam_data = OAM[(4*n) + m];
    }


    //Even cycle
    else
    {
        //if less than 8 sprites were found and n hasn't overflowed...
        if(oam_writes && !overflow_n)
        {
            y_coord = oam_data;
            secondary_oam[secondary_oam_pos] = oam_data;
            //If is in the range copy the remaining 3 bytes into secondary oam
            uint8_t sprite_row = scanline - y_coord;
            if ((sprite_row < 8) && (sprite_row >= 0) && y_coord < 0xEF)
            {
                secondary_oam[secondary_oam_pos + 1] = OAM[(4*n) + 1];
                secondary_oam[secondary_oam_pos + 2] = OAM[(4*n) + 2];
                secondary_oam[secondary_oam_pos + 3] = OAM[(4*n) + 3];
                sprites_found++;
                secondary_oam_pos += 4;
            }
            //increase n and secondary_oam_pos
            n++;
            if(n == 64)
            {
                n = 0;
                overflow_n = true;
            }

            //if there have already been found 8 sprites stop writes
            if (sprites_found == 8)
                oam_writes = false;
        }

        else if(overflow_n)
        {
            n++;
        }
        
        //Secondary OAM full, read from secondary oam
        else if(!oam_writes)
        {
            oam_data = secondary_oam[secondary_oam_pos + m];

            if(m == 0)
            {
                uint8_t sprite_row = scanline - OAM[(4*n) + m];
                if( sprite_row < 8 && (sprite_row) >= 0 && (OAM[(4*n) + m] < 0xEF))
                {
                    PPUSTATUS |= 0x20;
                    in_range = true;
                }
            }

            else if(in_range)
            {
                oam_data = OAM[(4*n) + m];
                if(m == 4)
                {
                    m = 0;
                    n++;
                    in_range = false;
                }
            }

            else
            {
                n++;
                if(n == 64)
                {
                    n = 0;
                    overflow_n = true;
                }
            }
            m++;
        }
    }
}

inline void PPU::load_shifters()
{
    bg_shift_register = ((bg_shift_register & 0xFF00) | bg_lsb);
    bg_shift_register1 = ((bg_shift_register1 & 0xFF00) | bg_msb);

    uint8_t x = v & 0x1F;
    uint8_t y = (v >> 5) & 0x1F;

    coarse_x_bit1 = (x >> 1) & 0x1;
    coarse_y_bit1 = (y >> 1) & 0x1;

    palette_bit_0 = (palette_bit_0 & 0xFF00) | (((attribute >> ((coarse_x_bit1 * 2) + (coarse_y_bit1 * 4))) & 0x01) > 0 ? 0xFF : 0x00);
    palette_bit_1 = (palette_bit_1 & 0xFF00) | (((attribute >> ((coarse_x_bit1 * 2) + (coarse_y_bit1 * 4) + 1)) & 0x01) > 0 ? 0xFF : 0x00);
}

 
void PPU::draw_sprite_pixel()
{
    uint8_t pixel;

    for(int i = 0; i < 8; i++)
    {
        uint8_t tile_id = scanline_sprite_buffer[(i * 6) + 1];
        uint8_t attribute_sprite = scanline_sprite_buffer[(i * 6) + 2];
        uint8_t x_coord = scanline_sprite_buffer[(i * 6) + 3];

        if(tile_id != 0xFF || attribute_sprite != 0xFF || x_coord != 0xFF)
        {
            uint8_t sprite_lsb = scanline_sprite_buffer[(i * 6) + 4];
            uint8_t sprite_msb = scanline_sprite_buffer[(i * 6) + 5]; 
            palette_sprite = attribute_sprite & 0x3;
            bool flip_horizontally = attribute_sprite & 0x40;

            for(int j = 0; j < 8; j++)
            {
                if(flip_horizontally)
                {
                    pixel = (sprite_lsb & 0x1) | ((sprite_msb & 0x1) << 1);
                    sprite_lsb >>= 1;
                    sprite_msb >>= 1;
                }
                else
                {
                    pixel = ((sprite_lsb & 0x80) >> 7) | ((sprite_msb & 0x80) >> 6);
                    sprite_lsb <<= 1;
                    sprite_msb <<= 1;
                }

                uint32_t color = get_palette_color(palette_sprite, pixel + 0x10);
                uint32_t x = x_coord + j;
                uint32_t screen_index = scanline * 256 + x;

                if((scanline_buffer[x] & 0x4) == 0)
                {
                    if(scanline_buffer[x] == 0x00 && pixel == 0x00)
                        screen[screen_index] = system_palette[get_palette_color(0, 0)];
                    else if(scanline_buffer[x] == 0x00 && pixel != 0x00)
                    {
                        screen[screen_index] = system_palette[color];
                        scanline_buffer[x] |= 0x4;
                    }
                    else if( (scanline_buffer[x] != 0x00) && (pixel != 0x00) && !(attribute_sprite & 0x20) )
                    {
                        screen[screen_index] = system_palette[color];
                        scanline_buffer[x] |= 0x4;
                    }

                }
            }
        }
    }
}

void PPU::draw_background_pixel()
{
    // Combine bit shift and masking operations to reduce complexity
    uint16_t mask = 0x8000 >> fine_x;
    bool pixel0 = (bg_shift_register & mask) != 0;
    bool pixel1 = (bg_shift_register1 & mask) != 0;
    
    bool final_palette_bit_0 = (palette_bit_0 & mask) != 0;
    bool final_palette_bit_1 = (palette_bit_1 & mask) != 0;

    uint8_t pixel = pixel0 | (pixel1 << 1);
    uint8_t palette_index = final_palette_bit_0 | (final_palette_bit_1 << 1);

    // Update screen2 and screen arrays
    uint32_t index = scanline * 256 + (cycles - 1);
    scanline_buffer[cycles - 1] = pixel; 
    screen[index] = system_palette[get_palette_color(palette_index & 0x3, pixel)];

    uint8_t sprite_row = scanline - scanline_sprite_buffer[0];

    if((scanline_sprite_buffer[0] == OAM[0]) && (scanline_sprite_buffer[1] == OAM[1]) && scanline_sprite_buffer[2] == OAM[02] && scanline_sprite_buffer[3] == OAM[3])
        if (((sprite_row < 8) && (sprite_row >= 0)) && (cycles - scanline_sprite_buffer[3] >= 0) &&  (cycles - scanline_sprite_buffer[3] < 8))
            check_sprite_0_hit();
}

void PPU::check_sprite_0_hit()
{
    uint8_t x_coord = scanline_sprite_buffer[3];
    uint8_t sprite_lsb = scanline_sprite_buffer[4];
    uint8_t sprite_msb = scanline_sprite_buffer[5]; 

    uint8_t offset = cycles - x_coord;
    uint8_t pixel = (((sprite_lsb << offset) & 0x80) >> 7) | (((sprite_msb << offset) & 0x80) >> 6);

    uint32_t x = x_coord + offset;

    if (IS_PPUMASK_SET(PPUMASK) && (x < 256) && !(PPUSTATUS & 0x40))
    {
        if (pixel != 0x00 && scanline_buffer[x] != 0x00)
        {
            if (!((((PPUMASK >> 1) & 0x3) != 3) && (x < 8)))
            {
                PPUSTATUS |= 0x40;  // Set sprite 0 hit flag
            }
        }
    }       
}


void PPU::tick()
{
    if((scanline < 240) || (scanline == 261))
    {
        if( ((cycles > 0) && (cycles < 257)) || ((cycles > 320) && (cycles < 337)) ) 
        {

            if(IS_PPUMASK_SET(PPUMASK) && (scanline != 261) && (cycles < 257))
                draw_background_pixel();
            shift_bits(); 

            switch(cycles & 0x7)
            {
                case 0:
                {
                    bg_msb = read((nametable_id * 16) + 0x1000 * ((PPUCTRL & 0x10) > 0) + ((v & 0x7000) >> 12) + 8);
                    load_shifters();
                    if( IS_PPUMASK_SET(PPUMASK) )
                        increment_hori_v();                
                    
                    break;
                }

                case 1:
                { 
                    if( (scanline == 261) && (cycles == 1) )
                    {
                        //clear vblank
                        PPUSTATUS &= 0x1F;
                        vblank = false;
                    }                
                    break; 
                }

                case 2:
                {
                    nametable_id = read((0x2000 | (v & 0x0FFF)));
                    break;
                }

                case 3: { break; }

                case 4:
                {
                    attribute = read((0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07)));
                    break;
                }

                case 5: { break; }

                case 6:
                {
                    bg_lsb = read((nametable_id * 16) + 0x1000 * ((PPUCTRL & 0x10) > 0) + ((v & 0x7000) >> 12));
                    break;
                }

                case 7: 
                { 
                    break; 
                }         
            }
        }

        if(IS_PPUMASK_SET(PPUMASK) && ((cycles >= 280) && ((cycles < 305))) && (scanline == 261))
            v = (v & ~(0x7BE0)) | (t & 0x7BE0);

        if( (scanline != 261) && ((cycles > 64) && ( cycles < 257)))
            sprite_evaluation();

        if(IS_PPUMASK_SET(PPUMASK) && cycles == 256 && scanline != 261)
            draw_sprite_pixel();
        if( IS_PPUMASK_SET(PPUMASK) && cycles == 256)
            increment_vert_v();

        if(IS_PPUMASK_SET(PPUMASK) && (cycles == 257))
            v = (v & ~(0x41F)) | (t & 0x41F);  

        if( (cycles > 256) && (cycles < 321) && (scanline != 261))
        {
            switch(cycles & 0x7)
            {
                case 1:
                    scanline_sprite_buffer[(i * 6) ] = secondary_oam[(i * 4)];
                    break;
                case 2: 
                    scanline_sprite_buffer[(i * 6) + 1] = secondary_oam[(i * 4) + 1];
                    break;
                case 3:
                    scanline_sprite_buffer[(i * 6) + 2] = secondary_oam[(i * 4) + 2];
                    break;
                case 4:
                    scanline_sprite_buffer[(i * 6) + 3] = secondary_oam[(i * 4) + 3];
                    break;
                case 5: 
                    break;
                case 6:
                    scanline_sprite_buffer[(i * 6) + 4] = bus->ppu_reads(((PPUCTRL & 0x8) > 0) * 0x1000 + (secondary_oam[(i*4) + 1] * 16) + (scanline - secondary_oam[i * 4]));
                    break;
                case 7: 
                    break;
                case 0:
                    scanline_sprite_buffer[(i * 6) + 5] = bus->ppu_reads(((PPUCTRL & 0x8) > 0) * 0x1000 + (secondary_oam[(i*4) + 1] * 16) + (scanline - secondary_oam[i * 4]) + 8);
                    i++;
                    break;
           }            
        }

        if( (cycles >= 257) && (cycles <= 320) )
            OAMADDR = 0;

        if( (scanline < 240) && ((cycles > 0) && (cycles < 65)))
        {
            if(cycles & 1)
                byte = 0xFF;
            else
            {
                secondary_oam[j] = byte;
                j++;
            }
        }
    }
                
    if( (scanline == 241) && (cycles == 1) )
    {
        PPUSTATUS |= 0x80;
        vblank = true;
        if(NMI_output)
        {
            bus->set_nmi(true);
            frame = !frame;
        } 
    }

    cycles++;
    if(cycles == 341)
    {
        cycles = 0;
        scanline++;
        j = 0;
        i = 0;


        if(scanline == 262)
        {
            scanline = 0;
            if(IS_PPUMASK_SET(PPUMASK) && odd)
            {
                cycles = 1;
            }
            odd ^= 1;
        }
    }
}
