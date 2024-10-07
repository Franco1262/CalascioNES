#include <iostream>
#include <vector>
#include <filesystem>
#include <cstdint>
#include <memory>
#include "CPU.h"
#include "PPU.h"
#include "Cartridge.h"
#include "Bus.h"
#include "Logger.h"

const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 240;
const int SCALE = 3;

struct App{
	SDL_Renderer *renderer;
	SDL_Window *window;
} app;

void initSDL();
void draw_frame(std::shared_ptr<PPU> ppu, SDL_Texture* screenBuffer, SDL_Texture* nametableBuffer0, SDL_Texture* nametableBuffer1, SDL_Texture* spriteBuffer);
uint16_t doInput();
void get_buffer(std::vector<int>&);
void draw_pattern_table(std::shared_ptr<PPU> ppu, SDL_Texture* patternBuffer0, SDL_Texture* patternBuffer1);

int main(int argc, char *argv[])
{
    std::shared_ptr<Logger> logger = std::make_shared<Logger>("logger.txt");
    std::shared_ptr<PPU> ppu = std::make_shared<PPU>(logger);
    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>(argv[1]);
    CPU cpu(logger);

    std::shared_ptr<Bus> bus = std::make_shared<Bus>(ppu, cart);
    cpu.connect_bus(bus);
    ppu->connect_bus(bus);

    ppu->reset();
    cpu.reset();



    initSDL();

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_Texture* screenBuffer = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
    SDL_Texture* nametableBuffer0 = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
    SDL_Texture* nametableBuffer1 = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
    SDL_Texture* spriteBuffer = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 64);
    SDL_Texture* patternBuffer0 = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);
    SDL_Texture* patternBuffer1 = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);

    bool current_frame = ppu->get_frame();
    draw_pattern_table(ppu, patternBuffer0, patternBuffer1);



    float ppu_accumulator = 0.0;
    float PPU_TIMING = 3;

    bool pause = false;

    while (true) 
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            // Handle SDL events like quitting the window
            if (event.type == SDL_QUIT) 
                exit(0);
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    pause = !pause;
                else if(event.key.keysym.scancode == SDL_SCANCODE_P)
                {
                    if(PPU_TIMING == 3)
                    {
                        PPU_TIMING = 3.2;
                        ppu->set_ppu_timing(1);
                    }
                    else
                    {
                        PPU_TIMING = 3;
                        ppu->set_ppu_timing(0);                        
                    }
                }
            }
        }


        while (current_frame == ppu->get_frame() && !pause) 
        {   
            cpu.tick();  // 1 CPU cycle
            ppu_accumulator += PPU_TIMING;
            while (ppu_accumulator >= 1.0)
            {
                ppu->tick(); // 1 PPU cycle
                ppu_accumulator -= 1.0;
            }
        }

        current_frame = ppu->get_frame();        
        draw_frame(ppu, screenBuffer, nametableBuffer0, nametableBuffer1, spriteBuffer);
    }

    // Destroy textures
    SDL_DestroyTexture(screenBuffer);
    SDL_DestroyTexture(nametableBuffer0);
    SDL_DestroyTexture(nametableBuffer1);
    SDL_DestroyTexture(spriteBuffer);

    return 0;
}

void initSDL()
{
    // Set hint for scaling quality before initialization
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Couldn't initialize SDL: " << SDL_GetError() << std::endl;
        exit(1);
    }

    // Create the window
    app.window = SDL_CreateWindow("NES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (SCREEN_WIDTH * SCALE) + 512, SCREEN_HEIGHT * SCALE, 0);
    if (!app.window)
    {
        std::cerr << "Failed to open " << (SCREEN_WIDTH * SCALE) + 512 << " x " << (SCREEN_HEIGHT * SCALE) << " window: " << SDL_GetError() << std::endl;
        exit(1);
    }

    // Create the renderer with hardware acceleration
    app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!app.renderer)
    {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        exit(1);
    }

    // Optionally clear the renderer if rendering immediately
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0xff);
    // SDL_RenderClear(app.renderer); // Consider removing if not needed immediately
}


void draw_frame(std::shared_ptr<PPU> ppu, SDL_Texture* screenBuffer, SDL_Texture* nametableBuffer0, SDL_Texture* nametableBuffer1, SDL_Texture* spriteBuffer)
{
    // Retrieve pixel data from PPU
    std::vector<uint32_t> nametable0 = ppu->get_nametable(0); 
    std::vector<uint32_t> nametable1 = ppu->get_nametable(1);
    std::vector<uint32_t> sprites = ppu->get_sprite(); 
    std::vector<uint32_t> screen = ppu->get_screen();

    // Define texture rendering positions
    SDL_Rect screen_rect = {0, 0, 256 * SCALE, 240 * SCALE};
    SDL_Rect nametable_rect = {256 * SCALE, 0, 256, 240};
    SDL_Rect nametable_rect1 = {256 * SCALE, 240, 256, 240};
    SDL_Rect sprite_rect = {256 * SCALE, 480, 256, 240};

    // Update textures
    SDL_UpdateTexture(screenBuffer, NULL, screen.data(), 256 * 4);
    SDL_UpdateTexture(nametableBuffer0, NULL, nametable0.data(), 256 * 4);
    SDL_UpdateTexture(nametableBuffer1, NULL, nametable1.data(), 256 * 4);
    SDL_UpdateTexture(spriteBuffer, NULL, sprites.data(), 64 * 4);

    SDL_RenderCopy(app.renderer, screenBuffer, NULL, &screen_rect);
    SDL_RenderCopy(app.renderer, nametableBuffer0, NULL, &nametable_rect);
    SDL_RenderCopy(app.renderer, nametableBuffer1, NULL, &nametable_rect1);
    SDL_RenderCopy(app.renderer, spriteBuffer, NULL, &sprite_rect);

    // Present the final rendered frame
    SDL_RenderPresent(app.renderer);
}



void draw_pattern_table(std::shared_ptr<PPU> ppu, SDL_Texture* patternBuffer0, SDL_Texture* patternBuffer1)
{
    std::vector<uint32_t> pattern0 = ppu->get_pattern_table(0);
    std::vector<uint32_t> pattern1 = ppu->get_pattern_table(1);


    //Pattern table texture
    static SDL_Rect pattern_rect0 = {(256 * SCALE) + 256, 0, 256, 240};
    static SDL_Rect pattern_rect1 = {(256 * SCALE) + 256, 240, 256, 240};

    SDL_UpdateTexture(patternBuffer0, NULL, pattern0.data(), 128 * sizeof(uint32_t));
    SDL_RenderCopy(app.renderer, patternBuffer0, NULL, &pattern_rect0); 


    SDL_UpdateTexture(patternBuffer1, NULL, pattern1.data(), 128 * sizeof(uint32_t));
    SDL_RenderCopy(app.renderer, patternBuffer1, NULL, &pattern_rect1); 
   
    SDL_DestroyTexture(patternBuffer0);
    SDL_DestroyTexture(patternBuffer1);
    SDL_RenderPresent(app.renderer);   
}