#include <iostream>
#include <cstdint>
#include <memory>
#include <vector>


struct ScanlineCounter
{
    bool irq_enable = false;
    uint8_t irq_counter = 0;
    uint8_t irq_latch = 0;
    bool irq_reload = false;
};

class Bus;
class PPU
{
    public:
        PPU();
        ~PPU();
        void tick();


        //PPU read an write functions
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t value);
        
        //CPU read and write functions
        uint8_t cpu_reads(uint16_t address);
        void cpu_writes(uint16_t address, uint8_t value); 
        //Functions for sprite handling
        void sprite_evaluation();
        void check_sprite_0_hit();

        //Functions for drawing data to screen
        void draw_background_pixel();
        void draw_sprite_pixel();
        
        void increment_hori_v();
        void increment_vert_v();
        void increment_v_ppudata();

        void load_shifters();
        void shift_bits();

        void connect_bus(std::shared_ptr<Bus> bus);
        void soft_reset();
        
        uint32_t get_palette_color(uint8_t paletteFF, uint8_t pixel);
        std::vector<uint32_t>& get_screen();
        //Functions useful for debugging
        std::vector<uint32_t> get_pattern_table(int);
        std::vector<uint32_t> get_nametable(int);
        std::vector<uint32_t> get_sprite();

        bool get_frame();
        void set_ppu_timing(uint8_t);


        //Zapper useful functions
        void is_pixel_bright(int x, int y);
        void set_zapper(bool zapper);
        void check_target_hit(int x, int y);

        void set_irq_latch(uint8_t value)
        {
            sc.irq_latch = value;
        }
        void set_irq_enable(bool value)
        {
            sc.irq_enable = value;
        }
        void set_irq_reload()
        {
            sc.irq_counter = 0;
            sc.irq_reload = true;
        }

        void clock_scanline_counter();
        void set_mapper(uint8_t value)
        {
            mapper = value;
        }
        
    private:
        int M2_falling_edges = 0;
        int M2_ppu_cycles = 0;
        void detect_a12_rising_edge();

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

        //Logger logger;


        uint8_t nametable[0x0800] = {0}; //VRAM 2kb
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

        uint8_t frame_palette[0x20] = {0};       
        uint8_t OAM[0x100] = {0}; //256 bytes that determines how sprites are rendered
        uint8_t secondary_oam[0x20] = {0};
        uint8_t scanline_sprite_buffer[0x30] = {0};

        ScanlineCounter sc; //Scanline counter for MMC3
        //PPU internal registers
        uint16_t v; //Current VRAM address; 15bits
        uint16_t t; //Temporary VRAM address; 15bits
        uint8_t fine_x; //X Scroll
        bool w = 0; //Firs or second write
        bool odd;

        int cycles;
        int scanline;
        bool frame;

        int mapper;


        uint8_t nametable_id;
        uint8_t attribute;
        
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
           
        int n; // variable for OAM fetching
        int m;


        //Variables used in sprite evaluation
        uint8_t oam_data;
        uint8_t y_coord;
        bool oam_writes;
        int secondary_oam_pos;
        int sprites_found;
        bool overflow_n;
        bool in_range;

        //Used for clearing OAM
        int secondary_oam_index;
        int i;


        //Variables for sprite 0 hit handling
        bool sprite_0_next_scanline;
        bool sprite_0_current_scanline;

        uint8_t sprite_y_coord;
        uint8_t attribute_sprite;
        uint8_t tile_id;
        uint16_t address;
        uint16_t pre_render_scanline;

        bool ppu_timing;
        uint8_t open_bus;
        bool supress = false;

        int zapper_x, zapper_y;
        bool zapper_connected = false;

        uint8_t is_rendering_enabled;
        uint8_t toggling_rendering_counter = 3;
};