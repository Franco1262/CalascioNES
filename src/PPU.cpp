#include <iostream>
#include <cstdint>
#include "PPU.h"
#include "Bus.h"
#include <sstream>
#include <iomanip>

PPU::PPU() : w(false) ,cycles(0), scanline(0)
{
    odd = false;
    frame = false;
    screen = std::vector<uint32_t>( 256*240 );
    scanline_buffer = std::vector<uint8_t>( 256 );
    pattern_buffer = std::vector<uint32_t>( 128*128 );
    nametable_buffer = std::vector<uint32_t>( 256 * 240 );
    sprite_buffer = std::vector<uint32_t>(64 * 64);
    ppu_timing = 0;
    pre_render_scanline = 261;
    secondary_oam_index = 0;
    i = 0;
    mapper = 0;
}

PPU::~PPU() {}


void PPU::tick()
{
    //Delay for toggling rendering, important for Battletoads
    if((is_rendering_enabled != ((PPUMASK >> 3) & 0x3)))
    {
        toggling_rendering_counter--;
        if(toggling_rendering_counter == 0)
        {
            is_rendering_enabled = ((PPUMASK >> 3) & 0x3);
            toggling_rendering_counter = 3;
        }
    }

    if(is_rendering_enabled)
    {
        if((scanline < 240) || (scanline == pre_render_scanline))
        {
            //Render background
            if(((cycles > 0) && (cycles < 257)) || ((cycles > 320) && (cycles < 337)))
            {
                if((scanline != pre_render_scanline) && (cycles < 257) && (PPUMASK & 0x8))
                    draw_background_pixel();
                shift_bits(); 

                switch(cycles & 0x7)
                {
                    case 0:
                    {
                        bg_msb = read(PPU_BUS);
                        load_shifters();
                        increment_hori_v(); 
                        if(cycles == 256)
                            increment_vert_v();       
                        break;
                    }

                    case 1:
                    {
                        PPU_BUS = (0x2000 | (v & 0x0FFF));
                    }
                    
                    case 2:
                    {
                        nametable_id = read(PPU_BUS);
                        break;
                    }

                    case 3:
                    {
                        PPU_BUS = (0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07));
                        break;
                    }

                    case 4:
                    {
                        attribute = read(PPU_BUS);
                        break;
                    }

                    case 5:
                    {
                        PPU_BUS = (nametable_id * 16) + 0x1000 * ((PPUCTRL & 0x10) > 0) + ((v & 0x7000) >> 12);
                        break;
                    }

                    case 6:
                    {
                        bg_lsb = read(PPU_BUS);
                        break;
                    }

                    case 7:
                    {
                        PPU_BUS = (nametable_id * 16) + 0x1000 * ((PPUCTRL & 0x10) > 0) + ((v & 0x7000) >> 12) + 8;
                        break;
                    }
                }
            }
            
            //Update loopy registers
            if((cycles == 257))
                v = (v & ~(0x41F)) | (t & 0x41F);  

            if(((cycles >= 280) && ((cycles < 305))) && (scanline == pre_render_scanline))
                v = (v & ~(0x7BE0)) | (t & 0x7BE0);

            //Sprite evaluation for next scanline
            if((scanline != pre_render_scanline) && (cycles > 64) && ( cycles < 257))
                sprite_evaluation();
            
            //Sprite fetches
            if((cycles > 256) && (cycles < 321))
            {      
                switch(cycles & 0x7)
                {
                    case 1:
                        sprite_y_coord = secondary_oam[(i * 4)];
                        scanline_sprite_buffer[(i * 6) ] = sprite_y_coord;
                        break;
                    case 2: 
                        tile_id = secondary_oam[(i * 4) + 1];
                        scanline_sprite_buffer[(i * 6) + 1] = tile_id;   
                        break;
                    case 3:
                        attribute_sprite = secondary_oam[(i * 4) + 2]; 
                        scanline_sprite_buffer[(i * 6) + 2] = attribute_sprite;
                        break;
                    case 4:
                        scanline_sprite_buffer[(i * 6) + 3] = secondary_oam[(i * 4) + 3];
                        break;
                    case 5: 
                        {
                            uint8_t fine_y = (scanline - sprite_y_coord);
                            PPU_BUS = 0;
                            if((attribute_sprite & 0x80) && (PPUCTRL & 0x20))
                                PPU_BUS += 16;
                                
                            if(fine_y >= 8 && PPUCTRL & 0x20)
                            {
                                if(attribute_sprite & 0x80)
                                    PPU_BUS -= 16;
                                else
                                    PPU_BUS += 16;
                            }
                            
                            
                            if(attribute_sprite & 0x80)
                                fine_y = 7 - (fine_y & 0x7);
        
                            //8x16 sprites
                            if(PPUCTRL & 0x20)
                                PPU_BUS += ((tile_id & 0x1) * 0x1000) + ((tile_id & 0xFE) * 16) + (fine_y & 0x7);

                            //8x8 sprites 
                            else
                                PPU_BUS = (((PPUCTRL & 0x8) > 0) * 0x1000) + (tile_id * 16) + (fine_y & 0x7);
                        }
                        break;
                    case 6:
                    case 0:
                    {                   
                        if((cycles & 0x7) == 0)
                        {
                            PPU_BUS += 8;
                            scanline_sprite_buffer[(i * 6) + 5] = read(PPU_BUS);
                            i++;
                        }
                        else
                            scanline_sprite_buffer[(i * 6) + 4] = read(PPU_BUS);  
    
                        break;      
                        }
                    
                    case 7: 
                        break;
                }            
            }

            //Drawing sprites
            if((is_rendering_enabled & 0x2)  && cycles == 256 && scanline != pre_render_scanline && scanline != 0)
                draw_sprite_pixel();
            //Clearing OAMADDR
            if( (cycles >= 257) && (cycles <= 320) )
                OAMADDR = 0; 
        }
    }

    if(!is_rendering_enabled || (scanline >= 241 && scanline <= 260))
        PPU_BUS = v;

    //When background is disabled draw the ext color
    if(((is_rendering_enabled & 1) == 0) && (scanline < 240) && cycles > 0 && cycles < 257)
        draw_background_pixel();

    //Clearing secondary OAM
    if((cycles > 0) && (cycles < 65) && (scanline < 240))
    {
        if(!(cycles & 1))
        {
            secondary_oam[secondary_oam_index] = 0xFF;
            secondary_oam_index++;
        }
    }
    
    //Clear vblank, sprite overflow and sprite 0 flag
    if((scanline == pre_render_scanline) && (cycles == 1))
        PPUSTATUS &= 0x1F;
    
    //Set vblank flag and fire NMI
    if( (scanline == 241) && (cycles == 1))
    {   
        if(!supress)
            PPUSTATUS |= 0x80;
        if((PPUCTRL & 0x80) && (PPUSTATUS & 0x80))
            bus->set_nmi(true);
        frame = !frame;   
    } 

    if(mapper == 4)
        detect_filtered_A12();
    cycles++;
    if(cycles == 341)
    {
        cycles = 0;
        scanline++;
        secondary_oam_index = 0;
        i = 0;
        sprite_0_current_scanline = sprite_0_next_scanline;
        supress = false;
        
        if(scanline == (pre_render_scanline+1))
        {
            scanline = 0;
            if( is_rendering_enabled && odd && (ppu_timing == 0))
                cycles = 1;
            odd ^= 1;
        }
        if(zapper_connected)
            is_pixel_bright(zapper_x, zapper_y);
    }
}

uint8_t PPU::read(uint16_t address)
{
    uint8_t data;

    address = address & 0x3FFF;
    
    if((address >= 0x0000) && (address < 0x2000))
    {
        data = bus->ppu_reads(address);
    }

    else if((address >= 0x2000) && (address < 0x3F00))
    {
        if(address >= 0x3000)
            address -= 0x1000;

        if (mirroring_mode == MIRROR::FOUR_SCREEN)
        {
            if (address >= 0x2000 && address < 0x2400)
                data = nametable[address & 0x3FF];
            else if (address >= 0x2400 && address < 0x2800)
                data = nametable[0x400 + (address & 0x3FF)];
            else if (address >= 0x2800 && address < 0x2C00)
                data = nametable[0x800 + (address & 0x3FF)];
            else if (address >= 0x2C00 && address < 0x3000) // Changed upper bound for consistency
                data = nametable[0xC00 + (address & 0x3FF)];
        }
            
        else if(mirroring_mode == MIRROR::HORIZONTAL)
        {
            if((address >= 0x2000) && (address < 0x2800))
                data = nametable[address & 0x3FF]; 
            
            else if((address >= 0x2800) && (address < 0x3000))          
                data = nametable[0x400 + (address & 0x3FF)];                  
        }
        
        else if(mirroring_mode == MIRROR::VERTICAL)
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
     

        else if(mirroring_mode == MIRROR::ONE_SCREEN_LOWER)
            data = nametable[address & 0x3FF];        

        else if(mirroring_mode == MIRROR::ONE_SCREEN_UPPER)
            data = nametable[0x400 + (address & 0x3FF)];      
    } 

    else if( (address >= 0x3F00) && (address <= 0x3FFF) )
    {
        if((address & 0x3) == 0x00)
        {
            data = frame_palette[0];
        }       
        else 
            data = frame_palette[address & 0x1F];
    }

    return data;
}

void PPU::write(uint16_t address, uint8_t value)
{
    address = address & 0x3FFF;

    if(address >= 0x0000 && address < 0x2000)
        bus->ppu_writes(address, value);

    else if((address >= 0x2000) && (address < 0x3F00))
    {
        if(address >= 0x3000)
            address -= 0x1000;

        if(mirroring_mode == MIRROR::FOUR_SCREEN)
        {
            if (address >= 0x2000 && address < 0x2400)
                nametable[address & 0x3FF] = value;
            else if (address >= 0x2400 && address < 0x2800)
                nametable[0x400 + (address & 0x3FF)] = value;
            else if (address >= 0x2800 && address < 0x2C00)
                nametable[0x800 + (address & 0x3FF)] = value;
            else if (address >= 0x2C00 && address < 0x3000)
                nametable[0xC00 + (address & 0x3FF)] = value;
        }

        else if(mirroring_mode == MIRROR::HORIZONTAL)
        {
            if((address >= 0x2000) && (address < 0x2800))            
                nametable[address & 0x3FF] = value;            

            else if((address >= 0x2800) && (address < 0x3000))           
                nametable[0x400 + (address & 0x3FF)] = value;                   
        }
        
        else if(mirroring_mode == MIRROR::VERTICAL)
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

        else if(mirroring_mode == MIRROR::ONE_SCREEN_LOWER)
            nametable[address & 0x3FF] = value;        

        else if(mirroring_mode == MIRROR::ONE_SCREEN_UPPER)
            nametable[0x400 + (address & 0x3FF)] = value; 
    }  

    else if( (address >= 0x3F00) && (address <= 0x3FFF) )
    {
        if((address & 0x3) == 0x00 )
            frame_palette[address & 0xF] = value;
        else 
            frame_palette[address & 0x1F] = value;
    }
}

uint8_t PPU::cpu_reads(uint16_t address)
{
    uint8_t data;
    switch(address)
    {
        case 0: {data = open_bus; break; }
        case 1: {data = open_bus; break; }
        case 2: 
        {
            if((scanline == 241 && cycles == 0))
            {
                data = PPUSTATUS & 0x7F;
                supress = true;
            }
            else if( (scanline == 241 && cycles == 1) || (scanline == 241 && cycles == 2))
            {
                data = PPUSTATUS | 0x80;
                supress = true;
                bus->set_nmi(false);
            }
            else
                data = PPUSTATUS;
            w = 0;
            PPUSTATUS &= 0x7F;
            open_bus = (open_bus & ~0xE0) | (PPUSTATUS & 0xE0);
            break; 
        }
        case 3: {data = open_bus; break; }
        case 4: 
        {
            data = OAM[OAMADDR];
            open_bus = data;
            if(scanline == pre_render_scanline && (cycles > 0) && (cycles < 65))
                data = 0xFF;
            break; 
        }
        case 5: {data = open_bus; break; }
        case 6: {data = open_bus; break; }
        case 7: 
        {
            if(v >= 0x3F00 && v <= 0x3FFF)
            {
                if((v & 0x3) == 0x00)
                    data = frame_palette[v & 0xF];       
                else 
                    data = frame_palette[v & 0x1F];
                ppudata_read_buffer = read(v - 0x1000);
            }
            else
            {
                data = ppudata_read_buffer;
                ppudata_read_buffer = read(v);
            }
            open_bus = data;
            increment_v_ppudata();
            break; 
        }
    }

    return data;
}

void PPU::cpu_writes(uint16_t address, uint8_t value)
{
    open_bus = value;

    switch(address) 
    {
    
        case 0: 
        {
            if(((PPUCTRL & 0x80) == 0) && (value & 0x80) && (PPUSTATUS & 0x80))
                bus->set_nmi(true);
            
            PPUCTRL = value;
            t = (t & ~(0x3 << 10)) | ((PPUCTRL & 0x3) << 10);

            break; 
        }
        case 1: 
        {
            PPUMASK = value;
            break; 
        }
        case 2: { break; }
        case 3: {OAMADDR = value; break; }
        case 4: 
        {
            OAM[OAMADDR] = value;  // Directly write value to OAM
            OAMADDR = (OAMADDR + 1) & 0xFF;  // Increment OAMADDR with wrapping
            break; 
        }
        case 5: 
        {
            PPUSCROLL = value;
            if(!w)
            {
                t = (t & ~(0x001F)) | ((PPUSCROLL & 0xF8) >> 3);
                fine_x = PPUSCROLL & 0x07;
                w = 1;
            }
            else
            {
                t = (t & ~(0x73E0)) | ((PPUSCROLL & 0x7) << 12) | (((PPUSCROLL & 0xF8) << 2));
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
        sprite_0_next_scanline = false;
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
            int sprite_size = ((PPUCTRL & 0x20) > 0) ? 16 : 8;
            if ((scanline >= y_coord) && (scanline < (y_coord + sprite_size)))
            {
                if(n == 0)
                    sprite_0_next_scanline = true;
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
                int sprite_size = ((PPUCTRL & 0x20) > 0) ? 16 : 8;
                if( (scanline >= oam_data) && (scanline < (oam_data + sprite_size)))
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

void PPU::check_sprite_0_hit()
{
    uint8_t x_coord = scanline_sprite_buffer[3];
    uint8_t sprite_y_coord = scanline_sprite_buffer[0];
    uint8_t attribute_sprite = scanline_sprite_buffer[2];
    uint8_t sprite_lsb;
    uint8_t sprite_msb;
    uint8_t  pixel;

    sprite_lsb = scanline_sprite_buffer[4];
    sprite_msb = scanline_sprite_buffer[5]; 

    bool flip_horizontally = attribute_sprite & 0x40;
    uint8_t offset = cycles - x_coord - 1;

    if(flip_horizontally)
    {
        pixel = ((sprite_lsb >> offset) & 0x1) | (((sprite_msb >> offset) & 0x1) << 1);
        sprite_lsb >>= 1;
        sprite_msb >>= 1;
    }
    else
    {
        pixel = (((sprite_lsb << offset) & 0x80) >> 7) | (((sprite_msb << offset) & 0x80) >> 6);
        sprite_lsb <<= 1;
        sprite_msb <<= 1;
    }

    uint8_t x = x_coord + offset ;
    bool left_clipping_enabled = (PPUMASK & 0x6) != 0x6;
    if ( (x < 255) && !(PPUSTATUS & 0x40) && pixel && scanline_buffer[x] && !(left_clipping_enabled && (x < 8)) && sprite_y_coord < 0xEF)
        PPUSTATUS |= 0x40;  // Set sprite 0 hit flag                  
}

void PPU::shift_bits()
{
    bg_shift_register1 <<= 1;
    bg_shift_register <<= 1;
    palette_bit_0 <<=1;
    palette_bit_1 <<=1;
}

void PPU::increment_hori_v()
{
    if ((v & 0x001F) == 31) // if coarse X == 31
    {
        v &= ~0x001F;          // coarse X = 0
        v ^= 0x0400;           // switch horizontal nametable
    }
    else
        v += 1;                // increment coarse X
}

void PPU::increment_vert_v()
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

void PPU::increment_v_ppudata()
{
    if(((scanline < 240) || (scanline == pre_render_scanline)) && is_rendering_enabled)
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

void PPU::load_shifters()
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

//Drawing data to screen
void PPU::draw_sprite_pixel()
{
    uint8_t pixel;
    uint8_t sprite_lsb;
    uint8_t sprite_msb;

    for(int i = 0; i < 8; i++)
    {
        uint8_t sprite_y_coord = scanline_sprite_buffer[(i * 6)];
        uint8_t tile_id = scanline_sprite_buffer[(i * 6) + 1];
        uint8_t attribute_sprite = scanline_sprite_buffer[(i * 6) + 2];
        uint8_t sprite_0_x_coord = scanline_sprite_buffer[(i * 6) + 3];

        if((tile_id != 0xFF || attribute_sprite != 0xFF || sprite_0_x_coord != 0xFF) && sprite_y_coord < 0xEF)
        {
            
            sprite_lsb = scanline_sprite_buffer[(i * 6) + 4];
            sprite_msb = scanline_sprite_buffer[(i * 6) + 5]; 

            uint8_t palette_sprite = attribute_sprite & 0x3;
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

                uint32_t color = get_palette_color(palette_sprite, pixel | 0x10);
                uint32_t x = sprite_0_x_coord + j;
                uint32_t screen_index = scanline * 256 + x;

                if( !(!(PPUMASK & 0x4) && (x < 8)))
                {
                    if((scanline_buffer[x] & 0x4) == 0)
                    {
                        if(scanline_buffer[x] == 0x00 && pixel == 0x00)         
                            screen[screen_index] = system_palette[get_palette_color(0, 0)];
                        
                        else if(scanline_buffer[x] == 0x00 && pixel != 0x00)
                        {
                            screen[screen_index] = system_palette[color];
                            scanline_buffer[x] |= 0x4;
                        }

                        else if(pixel == 0x00 && (scanline_buffer[x] != 0x00));

                        else if( (scanline_buffer[x] != 0x00) && (pixel != 0x00) && !(attribute_sprite & 0x20) )
                        {
                            screen[screen_index] = system_palette[color];
                            scanline_buffer[x] |= 0x4;
                        }
                        
                        else if( (scanline_buffer[x] != 0x00) && (pixel != 0x00) && (attribute_sprite & 0x20) )  
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

    //if background is enabled in the leftmost 8 pixels...
    if( !(!(PPUMASK & 0x2) && ((cycles-1) < 8)) )
    {
        if((PPUMASK & 0x8))
        {
            screen[index] = system_palette[get_palette_color(palette_index, pixel)];
            scanline_buffer[cycles - 1] = pixel;
        }
        else
        {
            screen[index] = system_palette[get_palette_color(0, 0)];

            if(!is_rendering_enabled && (v >= 0x3F00) && (v <= 0x3FFF))
                    screen[index] = system_palette[read(v)];     
            
            scanline_buffer[cycles - 1] = 0x00;            
        }
    }
    
    else
    {
        screen[index] = system_palette[get_palette_color(0, 0)];
        scanline_buffer[cycles - 1] = 0x00;
    }
    
    if(sprite_0_current_scanline && ((PPUMASK & 0x18) == 0x18))
    {
        uint8_t sprite_0_x_coord = scanline_sprite_buffer[3];
        if ( ((cycles - 1 ) - sprite_0_x_coord >= 0) && ((cycles - 1) - sprite_0_x_coord < 8) )
            check_sprite_0_hit();
    }

        
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

void PPU::set_ppu_timing(uint8_t value)
{
    ppu_timing = value;

    if(ppu_timing == 0)
        pre_render_scanline = 261;
    else
        pre_render_scanline = 311;
}

void PPU::connect_bus(std::shared_ptr<Bus> bus)
{
    this->bus = bus;
}

std::vector<uint32_t>& PPU::get_screen()
{
    return screen;
}

void PPU::soft_reset()
{
    // Reset PPU registers
    PPUCTRL = 0x00;
    PPUMASK = 0x00;
    PPUSTATUS = 0x00;
    OAMADDR = 0x00;
    OAMDATA = 0x00;
    PPUSCROLL = 0x00;
    PPUADDR = 0x00;
    PPUDATA = 0x00;
    OAMDMA = 0x00;

    // Reset VRAM, OAM, and secondary OAM
    std::fill(std::begin(nametable), std::end(nametable), 0);
    std::fill(std::begin(frame_palette), std::end(frame_palette), 0);
    std::fill(std::begin(OAM), std::end(OAM), 0);
    std::fill(std::begin(secondary_oam), std::end(secondary_oam), 0);
    std::fill(std::begin(scanline_sprite_buffer), std::end(scanline_sprite_buffer), 0);

    // Reset internal registers
    v = 0x0000;
    t = 0x0000;
    fine_x = 0;
    w = false;
    odd = false;

    // Reset rendering and sprite evaluation states
    cycles = 0;
    scanline = 0;
    frame = false;

    nametable_id = 0;
    attribute = 0;
    bg_lsb = 0;
    bg_msb = 0;
    bg_shift_register = 0;
    bg_shift_register1 = 0;
    palette_bit_0 = 0;
    palette_bit_1 = 0;
    coarse_x_bit1 = 0;
    coarse_y_bit1 = 0;
    ppudata_read_buffer = 0;

    // Clear screen buffers
    std::fill(screen.begin(), screen.end(), 0);
    std::fill(scanline_buffer.begin(), scanline_buffer.end(), 0);
    std::fill(pattern_buffer.begin(), pattern_buffer.end(), 0);
    std::fill(nametable_buffer.begin(), nametable_buffer.end(), 0);
    std::fill(sprite_buffer.begin(), sprite_buffer.end(), 0);

    // Reset sprite evaluation variables
    n = 0;
    m = 0;
    oam_data = 0;
    y_coord = 0;
    oam_writes = false;
    secondary_oam_pos = 0;
    sprites_found = 0;
    overflow_n = false;
    in_range = false;
    secondary_oam_index = 0;
    i = 0;

    // Reset rendering flags
    is_rendering_enabled = 0;

    // Reset sprite 0 hit handling
    sprite_0_next_scanline = false;
    sprite_0_current_scanline = false;
    sprite_y_coord = 0;
    attribute_sprite = 0;
    tile_id = 0;
    address = 0;
    pre_render_scanline = 261;

    open_bus = 0;
    supress = false;
    ppu_timing = 0;

    sc.irq_counter = 0;
    sc.irq_enable = false;
    sc.irq_latch = 0;
    sc.irq_reload = false;
}

//Functions useful for zapper
void PPU::is_pixel_bright(int x, int y)
{
    bool ok = false;
    uint32_t pixel = screen[(y * 256) + x];
    if(pixel == system_palette[0x20])
        ok = true;
    for(int i = 0x31 ; i <= 0x3C; i++)
    {
        if(pixel == system_palette[i])
            ok = true;
    }
    bus->set_light_sensed(ok);
}

void PPU::check_target_hit(int x, int y)
{
    zapper_x = x;
    zapper_y = y;
}

void PPU::set_zapper(bool zapper)
{
    zapper_connected = zapper;
}

bool PPU::get_frame() { return frame; }

void PPU::clock_scanline_counter()
{
    if ((sc.irq_counter == 0) || sc.irq_reload) 
    {     
        sc.irq_counter = sc.irq_latch; // Reload the counter
        sc.irq_reload = false;
    }
    else
        sc.irq_counter--;

    if (sc.irq_enable && sc.irq_counter == 0)
        bus->trigger_irq();
}


void PPU::detect_filtered_A12()
{
    bool current_a12 = PPU_BUS & 0x1000;
    if(!current_a12)
    {
        ppu_cycles++;
        if(ppu_cycles == 3)
        {
            ppu_cycles = 0;
            M2_falling_edges++; 
        }
    }

    else
    {
        if(!prev_A12 && M2_falling_edges > 2)
            clock_scanline_counter();
        
        M2_falling_edges = 0;
        ppu_cycles = 0;
    }

    prev_A12 = current_a12;
}