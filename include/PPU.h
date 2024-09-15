#include <iostream>
#include <cstdint>
#include <memory>
#include <vector>

class Bus;
class PPU
{
    public:
        PPU();
        ~PPU();
        void tick();

        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t value);
        
        uint8_t cpu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value); 
       
        void connect_bus(std::shared_ptr<Bus> bus);
        void reset();
        
        uint32_t get_palette_color(uint8_t paletteFF, uint8_t pixel);
        std::vector<uint32_t>& get_screen();
        std::vector<uint32_t> get_pattern_table(int);
        std::vector<uint32_t> get_nametable(int);
        std::vector<uint32_t> get_sprite();

        unsigned long get_frame();

        void increment_hori_v();
        void increment_vert_v();
        void increment_v_ppudata();


        void load_shifters();
        void shift_bits();
        void draw_background_pixel();
        void draw_sprite_pixel();
        void check_sprite_0_hit();

        void sprite_evaluation();
    private:
        //PPU Registers
        uint8_t PPUCTRL;
        uint8_t PPUMASK;
        uint8_t PPUSTATUS;
        uint8_t OAMADDR;
        uint8_t OAMDATA;
        uint8_t PPUSCROLL;
        uint8_t PPUADDR;
        uint8_t PPUDATA;
        uint8_t OAMDMA;

        std::shared_ptr<Bus> bus;


        uint8_t nametable[0x0800]; //VRAM 2kb
        const uint32_t system_palette[64] = {
                    0x666666FF, 0x002a88FF, 0x1412a7FF, 0x3b00a4FF, 0x5c007eFF, 0x6e0040FF, 0x6c0600FF, 0x561d00FF,
                    0x333500FF, 0x0b4800FF, 0x005200FF, 0x004f08FF, 0x00404dFF, 0x000000FF, 0x000000FF, 0x000000FF,
                    0xadadadFF, 0x155fd9FF, 0x4240ffFF, 0x7527feFF, 0xa01accFF, 0xb71e7bFF, 0xb53120FF, 0x994e00FF,
                    0x6b6d00FF, 0x388700FF, 0x0c9300FF, 0x008f32FF, 0x007c8dFF, 0x000000FF, 0x000000FF, 0x000000FF,
                    0xfffeffFF, 0x64b0ffFF, 0x9290ffFF, 0xc676ffFF, 0xf36affFF, 0xfe6eccFF, 0xfe8170FF, 0xea9e22FF,
                    0xbcbe00FF, 0x88d800FF, 0x5ce430FF, 0x45e082FF, 0x48cddeFF, 0x4f4f4fFF, 0x000000FF, 0x000000FF,
                    0xfffeffFF, 0xc0dfffFF, 0xd3d2ffFF, 0xe8c8ffFF, 0xfbc2ffFF, 0xfec4eaFF, 0xfeccc5FF, 0xf7d8a5FF,
                    0xe4e594FF, 0xcfef96FF, 0xbdf4abFF, 0xb3f3ccFF, 0xb5ebf2FF, 0xb8b8b8FF, 0x000000FF, 0x000000FF,
                };

        uint8_t frame_palette[0x20];       
        uint8_t OAM[0x100]; //256 bytes that determines how sprites are rendered
        uint8_t secondary_oam[0x20];
        uint8_t scanline_sprite_buffer[0x30];


        struct Sprite
        {
            uint16_t y_coord;
            uint16_t tile_index;
            uint16_t attribute;
            uint16_t x_coord;
        } sprites_shifter[8];

        //Flags
        bool even_odd;
        bool vblank;
        bool NMI_output;

        //PPU internal registers
        uint16_t v; //Current VRAM address; 15bits
        uint16_t t; //Temporary VRAM address; 15bits
        uint8_t fine_x; //X Scroll
        bool w = 0; //Firs or second write
        bool odd;

        int cycles;
        int scanline;
        bool frame;

        uint8_t palette;
        uint8_t palette_sprite;



        uint8_t nametable_id;
        uint8_t attribute;
        uint16_t attribute_next_tile;
        uint8_t attribute_current_tile;
        
        uint8_t bg_lsb;
        uint8_t bg_msb;

        uint16_t bg_shift_register;
        uint16_t bg_shift_register1;

        uint16_t palette_bit_0;
        uint16_t palette_bit_1;

        uint8_t coarse_x_bit1;
        uint8_t coarse_y_bit1; 

        uint8_t ppudata_read_buffer;   


        std::vector<uint32_t> screen;
        std::vector<uint8_t> scanline_buffer;
        std::vector<uint32_t> pattern_buffer;
        std::vector<uint32_t> nametable_buffer;
        std::vector<uint32_t> sprite_buffer;
           
        bool frame_complete;
        int n; // variable for OAM fetching
        int m;
        uint8_t oam_data;
        uint8_t y_coord;
        uint8_t byte;
        bool oam_writes;
        int secondary_oam_pos;
        int sprites_found;
        bool overflow_n;
        bool sprite_overflow;
        bool sprite_zero_hit;
        bool in_range;
        int j;
        int i;
};