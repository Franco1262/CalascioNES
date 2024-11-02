#include <iostream>
#include <vector>
#include <cstdint>
#include "CPU.h"
#include "PPU.h"
#include "Cartridge.h"
#include "Bus.h"
#include "Logger.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 240;
const int SCALE = 3;

class SDL_manager
{
    public:
        SDL_manager()
        {
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
            // Initialize SDL
            if (SDL_Init(SDL_INIT_VIDEO) < 0)
            {
                std::cerr << "Couldn't initialize SDL: " << SDL_GetError() << std::endl;
                exit(1);
            }

            // Create the window
            window = SDL_CreateWindow("NES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (SCREEN_WIDTH * SCALE), SCREEN_HEIGHT * SCALE, 0);
            if (!window)
            {
                std::cerr << "Failed to open " << (SCREEN_WIDTH * SCALE) << " x " << (SCREEN_HEIGHT * SCALE) << " window: " << SDL_GetError() << std::endl;
                exit(1);
            }

            // Create the renderer with hardware acceleration
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (!renderer)
            {
                std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
                exit(1);
            }
        }
        ~SDL_manager()
        {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
        }

        SDL_Renderer* get_renderer()
        {
            return renderer;
        }

        SDL_Window* get_window()
        {
            return window;
        }

    private:
        SDL_Renderer *renderer;
        SDL_Window *window;
};


class NES
{
    public:
        NES()
        {
            cpu = std::make_shared<CPU>();
            ppu = std::make_shared<PPU>();
            cart = std::make_shared<Cartridge>();
            bus = std::make_shared<Bus>(ppu, cart);

            cpu->connect_bus(bus);
            ppu->connect_bus(bus);
        }

        void load_game(std::string filename)
        {
            cart->load_game(filename);
            game_loaded = true;
        }

        void run_frame()
        {
            current_frame = ppu->get_frame();
            while (current_frame == ppu->get_frame() && !pause) 
            {   
                ppu_accumulator += PPU_TIMING;
                while (ppu_accumulator >= 1.0)
                { 
                    ppu->tick();
                    ppu_accumulator -= 1.0;
                }
                cpu->tick();
            }      
        }

        void change_pause()
        {
            pause = !pause;
        }

        void change_timing()
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

        bool is_game_loaded()
        {
            return game_loaded;
        }

        std::shared_ptr<PPU> get_ppu()
        {
        return ppu;
        }

        void reset()
        {
            cpu->soft_reset();
            ppu->soft_reset();
            cart->soft_reset();
            bus->soft_reset();
            ppu_accumulator = 0;
        }

    private:
        std::shared_ptr<CPU> cpu;
        std::shared_ptr<PPU> ppu;
        std::shared_ptr<Cartridge> cart;
        std::shared_ptr<Bus> bus;
        bool current_frame;
        float ppu_accumulator = 0.0;
        float PPU_TIMING = 3;
        bool pause = false;
        bool game_loaded;

};


//SDL2
void create_textures(SDL_Renderer* renderer);
void destroy_textures();
void handle_events(NES* nes, bool& running, SDL_Renderer* renderer);
//ImGUI
void initImGui(SDL_Window* window, SDL_Renderer* renderer);
void cleanupImGui();
//Functions to draw
void draw_frame(std::shared_ptr<PPU> ppu, SDL_Renderer*);
void draw_pattern_table(std::shared_ptr<PPU> ppu, SDL_Renderer*);

SDL_Texture* screenBuffer;
SDL_Texture* nametableBuffer0;
SDL_Texture* nametableBuffer1;
SDL_Texture* spriteBuffer;
SDL_Texture* patternBuffer0;
SDL_Texture* patternBuffer1;

int main(int argc, char *argv[])
{
    NES nes;
    SDL_manager manager;
    bool running = true;

    if(argc > 1)
    {
        std::string filename(argv[1]);
        nes.load_game(filename);
    }

    initImGui(manager.get_window(), manager.get_renderer());

    create_textures(manager.get_renderer());

    while (running) 
    {
        handle_events(&nes, running, manager.get_renderer());
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if(nes.is_game_loaded())
        {
            nes.run_frame();
            draw_frame(nes.get_ppu(), manager.get_renderer());
        }

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), manager.get_renderer());
        SDL_RenderPresent(manager.get_renderer());
    }

    cleanupImGui();
    destroy_textures();

    return 0;
}

void draw_frame(std::shared_ptr<PPU> ppu, SDL_Renderer* renderer)
{
    // Retrieve pixel data from PPU
/*     std::vector<uint32_t> nametable0 = ppu->get_nametable(0); 
    std::vector<uint32_t> nametable1 = ppu->get_nametable(1);
    std::vector<uint32_t> sprites = ppu->get_sprite();  */
    std::vector<uint32_t> screen = ppu->get_screen();

    // Define texture rendering positions
    SDL_Rect screen_rect = {0, 0, 256 * SCALE, 240 * SCALE};
/*     SDL_Rect nametable_rect = {256 * SCALE, 0, 256, 240};
    SDL_Rect nametable_rect1 = {256 * SCALE, 240, 256, 240};
    SDL_Rect sprite_rect = {256 * SCALE, 480, 256, 240}; */

    // Update textures
    SDL_UpdateTexture(screenBuffer, NULL, screen.data(), 256 * 4);
/*     SDL_UpdateTexture(nametableBuffer0, NULL, nametable0.data(), 256 * 4);
    SDL_UpdateTexture(nametableBuffer1, NULL, nametable1.data(), 256 * 4);
    SDL_UpdateTexture(spriteBuffer, NULL, sprites.data(), 64 * 4); */

    SDL_RenderCopy(renderer, screenBuffer, NULL, &screen_rect);
/*     SDL_RenderCopy(renderer, nametableBuffer0, NULL, &nametable_rect);
    SDL_RenderCopy(renderer, nametableBuffer1, NULL, &nametable_rect1);
    SDL_RenderCopy(renderer, spriteBuffer, NULL, &sprite_rect); */
}

void draw_pattern_table(std::shared_ptr<PPU> ppu, SDL_Renderer* renderer)
{
    std::vector<uint32_t> pattern0 = ppu->get_pattern_table(0);
    std::vector<uint32_t> pattern1 = ppu->get_pattern_table(1);


    //Pattern table texture
    static SDL_Rect pattern_rect0 = {(256 * SCALE) + 256, 0, 256, 240};
    static SDL_Rect pattern_rect1 = {(256 * SCALE) + 256, 240, 256, 240};

    SDL_UpdateTexture(patternBuffer0, NULL, pattern0.data(), 128 * sizeof(uint32_t));
    SDL_RenderCopy(renderer, patternBuffer0, NULL, &pattern_rect0); 


    SDL_UpdateTexture(patternBuffer1, NULL, pattern1.data(), 128 * sizeof(uint32_t));
    SDL_RenderCopy(renderer, patternBuffer1, NULL, &pattern_rect1); 
}

void create_textures(SDL_Renderer* renderer)
{
    screenBuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
    nametableBuffer0 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
    nametableBuffer1 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
    spriteBuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 64);
    patternBuffer0 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);
    patternBuffer1 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);
}

void destroy_textures()
{
    SDL_DestroyTexture(screenBuffer);
    SDL_DestroyTexture(nametableBuffer0);
    SDL_DestroyTexture(nametableBuffer1);
    SDL_DestroyTexture(spriteBuffer);
    SDL_DestroyTexture(patternBuffer0);
    SDL_DestroyTexture(patternBuffer1);  
}

void handle_events(NES* nes, bool& running, SDL_Renderer* renderer)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        if (event.type == SDL_QUIT) 
            running = false;
        else if (event.type == SDL_KEYDOWN)
        {
            switch(event.key.keysym.scancode)
            {
                case SDL_SCANCODE_P:
                        nes->change_pause();
                        break;

                case SDL_SCANCODE_ESCAPE:
                    running = false;
                    break;

                case SDL_SCANCODE_O:
                    nes->change_timing();
                    break;

                default: break;
            }
        }

        else if(event.type == SDL_DROPFILE)
        {
            std::string filename(event.drop.file);
            nes->reset();
            nes->load_game(filename);
            SDL_free(event.drop.file);
        }
    }   
}

void initImGui(SDL_Window* window, SDL_Renderer* renderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    ImGui::StyleColorsDark();
}

void cleanupImGui() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}