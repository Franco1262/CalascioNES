#include <iostream>
#include <vector>
#include <filesystem>
#include <cstdint>
#include <memory>
#include "CPU.h"
#include "PPU.h"
#include "Cartridge.h"
#include "Bus.h" 

const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 240;
const int SCALE = 3;

const int FREQUENCY = 1000000;
const int DELAY = 1 / FREQUENCY;

struct App{
	SDL_Renderer *renderer;
	SDL_Window *window;
} app;

void initSDL();
void draw_frame(std::shared_ptr<PPU>);
uint16_t doInput();
void get_buffer(std::vector<int>&);
void draw_pattern_table(std::shared_ptr<PPU>);

int main(int argc, char *argv[])
{
    std::shared_ptr<PPU> ppu = std::make_shared<PPU>();
    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>(argv[1]);
    CPU cpu;

    std::shared_ptr<Bus> bus = std::make_shared<Bus>(ppu, cart);
    cpu.connect_bus(bus);
    ppu->connect_bus(bus);

    cpu.reset();
    ppu->reset();


    initSDL(); 
    bool current_frame = ppu->get_frame();
    draw_pattern_table(ppu);



    float ppu_accumulator = 0.0;
    float PPU_TIMING = 3;

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
                    exit(0);
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

        while (current_frame == ppu->get_frame()) 
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
        draw_frame(ppu);
    }



    return 0;
}

void initSDL()
{
	int rendererFlags, windowFlags;

	rendererFlags = SDL_RENDERER_ACCELERATED;

	windowFlags = 0;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout<<"Couldn't initialize SDL"<<SDL_GetError();
		exit(1);
	}		

	app.window = SDL_CreateWindow("NES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (SCREEN_WIDTH * SCALE) + 512, SCREEN_HEIGHT * SCALE, windowFlags);

	if (!app.window)
	{
		printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH , SCREEN_HEIGHT, SDL_GetError());
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);

	if (!app.renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}
    SDL_SetRenderDrawColor(app.renderer, 0, 0 ,0, 0xff);
    SDL_RenderClear(app.renderer);
}
void draw_frame(std::shared_ptr<PPU> ppu)
{
    std::vector<uint32_t> nametable0 = ppu->get_nametable(0); 
    std::vector<uint32_t> nametable1 = ppu->get_nametable(1);
    std::vector<uint32_t> sprites = ppu->get_sprite(); 
    std::vector<uint32_t> screen = ppu->get_screen();
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    void* pixels = nullptr;
    int pitch = 256 *4;


    //Screen texture
    SDL_Rect screen_rect = {0, 0, 256*SCALE, 240*SCALE};

    SDL_Texture* buffer = SDL_CreateTexture(app.renderer,
                           SDL_PIXELFORMAT_RGBA8888,
                           SDL_TEXTUREACCESS_STREAMING, 
                           256,
                           240);
    SDL_LockTexture(buffer,
                    NULL,      // NULL means the *whole texture* here.
                    &pixels,
                    &pitch);

    memcpy(pixels, screen.data(), screen.size() * sizeof(unsigned int));  
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(app.renderer, buffer, NULL, &screen_rect);

    //Nametable texture
    SDL_Rect nametable_rect = {256 * SCALE, 0, 256, 240};
    buffer = SDL_CreateTexture(app.renderer,
                           SDL_PIXELFORMAT_RGBA8888,
                           SDL_TEXTUREACCESS_STREAMING, 
                           256,
                           240);
    SDL_LockTexture(buffer,
                    NULL,      // NULL means the *whole texture* here.
                    &pixels,
                    &pitch);

    memcpy(pixels, nametable0.data(), nametable0.size() * sizeof(unsigned int));  
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(app.renderer, buffer, NULL, &nametable_rect);

    nametable_rect = {256 * SCALE, 240, 256, 240};
    buffer = SDL_CreateTexture(app.renderer,
                           SDL_PIXELFORMAT_RGBA8888,
                           SDL_TEXTUREACCESS_STREAMING, 
                           256,
                           240);
    SDL_LockTexture(buffer,
                    NULL,      // NULL means the *whole texture* here.
                    &pixels,
                    &pitch);

    memcpy(pixels, nametable1.data(), nametable1.size() * sizeof(unsigned int));  
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(app.renderer, buffer, NULL, &nametable_rect);

    pitch = 64 * 8;
    nametable_rect = {256 * SCALE, 480, 256, 240};
    buffer = SDL_CreateTexture(app.renderer,
                           SDL_PIXELFORMAT_RGBA8888,
                           SDL_TEXTUREACCESS_STREAMING, 
                           64,
                           64);
    SDL_LockTexture(buffer,
                    NULL,      // NULL means the *whole texture* here.
                    &pixels,
                    &pitch);

    memcpy(pixels, sprites.data(), sprites.size() * sizeof(unsigned int));  
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(app.renderer, buffer, NULL, &nametable_rect );
   
    SDL_DestroyTexture(buffer);
    SDL_RenderPresent(app.renderer);
}

void draw_pattern_table(std::shared_ptr<PPU> ppu)
{
    std::vector<uint32_t> pattern = ppu->get_pattern_table(0);
    std::vector<uint32_t> pattern2 = ppu->get_pattern_table(1);

    void* pixels = nullptr;
    int pitch = 128 *4;

    //Pattern table texture
    SDL_Rect pattern_rect = {(256 * SCALE) + 256, 0, 256, 240};
    SDL_Texture* buffer = SDL_CreateTexture(app.renderer,
                           SDL_PIXELFORMAT_RGBA8888,
                           SDL_TEXTUREACCESS_STREAMING, 
                           128,
                           128);
    SDL_LockTexture(buffer,
                    NULL,      // NULL means the *whole texture* here.
                    &pixels,
                    &pitch);


    memcpy(pixels, pattern.data(), pattern.size() * sizeof(unsigned int));  
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(app.renderer, buffer, NULL, &pattern_rect); 


    pattern_rect = {(256 * SCALE) + 256, 240, 256, 240};
    buffer = SDL_CreateTexture(app.renderer,
                           SDL_PIXELFORMAT_RGBA8888,
                           SDL_TEXTUREACCESS_STREAMING, 
                           128,
                           128);
    SDL_LockTexture(buffer,
                    NULL,      // NULL means the *whole texture* here.
                    &pixels,
                    &pitch);

    memcpy(pixels, pattern2.data(), pattern2.size() * sizeof(unsigned int));  
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(app.renderer, buffer, NULL, &pattern_rect); 
   
    SDL_DestroyTexture(buffer);
    SDL_RenderPresent(app.renderer);   
}