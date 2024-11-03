#include <iostream>
#include <vector>
#include <cstdint>
#include "CPU.h"
#include "PPU.h"
#include "Cartridge.h"
#include "Bus.h"
#include "nfd.h"
#include <filesystem>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

const int SCALE = 3;
const int SCREEN_WIDTH = 256 * SCALE;
const int SCREEN_HEIGHT = 240 * SCALE;

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
            window = SDL_CreateWindow("CalascioNES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
            if (!window)
            {
                std::cerr << "Failed to open " << SCREEN_WIDTH << " x " << SCREEN_HEIGHT << " window: " << SDL_GetError() << std::endl;
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
            if(std::filesystem::path(filename).extension().string() == ".nes")
            {
                old_game_filename = filename;           
                reset();
                cart->load_game(filename);
                game_loaded = true;
            }
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

        void reload_game()
        {
            reset();
            load_game(old_game_filename);
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
        bool game_loaded = false;
        std::string old_game_filename;

};


//SDL2
void create_textures(SDL_Renderer* renderer);
void destroy_textures();
void handle_events(NES* nes, bool& running, SDL_manager* manager);
//ImGUI
void initImGui(SDL_Window* window, SDL_Renderer* renderer);
void handle_imGui(NES* nes, bool& running, SDL_Renderer* renderer);
void cleanupImGui();
//Functions to draw
void draw_frame(std::shared_ptr<PPU> ppu, SDL_Renderer*);
void draw_pattern_table(std::shared_ptr<PPU> ppu);


SDL_Texture* screenBuffer;
SDL_Texture* nametableBuffer0;
SDL_Texture* nametableBuffer1;
SDL_Texture* spriteBuffer;
SDL_Texture* patternBuffer0;
SDL_Texture* patternBuffer1;

SDL_Window* nametable_window;
SDL_Renderer* nametable_renderer;

SDL_Window* pattern_window;
SDL_Renderer* pattern_renderer;

SDL_Window* sprite_window;
SDL_Renderer* sprite_renderer;

int padding;

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

    padding = ImGui::GetFrameHeight();
    SDL_SetWindowSize(manager.get_window(), SCREEN_WIDTH, SCREEN_HEIGHT + padding);
    screenBuffer = SDL_CreateTexture(manager.get_renderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);

    while (running) 
    {
        handle_events(&nes, running, &manager);
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();  
        SDL_SetRenderDrawColor(manager.get_renderer(), 0, 0, 0, 255); // Clear to black
        SDL_RenderClear(manager.get_renderer());

        if(nes.is_game_loaded())
        {
            nes.run_frame();
            draw_frame(nes.get_ppu(), manager.get_renderer());
            //draw_pattern_table(nes.get_ppu());
        }
        handle_imGui(&nes, running, manager.get_renderer());
        SDL_RenderPresent(manager.get_renderer());
    }

    cleanupImGui();
    SDL_DestroyRenderer(nametable_renderer);
    SDL_DestroyRenderer(pattern_renderer);
    SDL_DestroyRenderer(sprite_renderer);
    SDL_DestroyWindow(nametable_window);
    SDL_DestroyWindow(pattern_window);
    SDL_DestroyWindow(sprite_window);
    destroy_textures();

    return 0;
}

void draw_frame(std::shared_ptr<PPU> ppu, SDL_Renderer* renderer)
{
     
    //Screen
    std::vector<uint32_t> screen = ppu->get_screen();
    SDL_Rect screen_rect = {0, padding , SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_UpdateTexture(screenBuffer, NULL, screen.data(), 256 * 4);
    SDL_RenderCopy(renderer, screenBuffer, NULL, &screen_rect);

    //Nametable Viewer
    if(nametable_window != nullptr)
    {
        std::vector<uint32_t> nametable0 = ppu->get_nametable(0); 
        std::vector<uint32_t> nametable1 = ppu->get_nametable(1);
        SDL_Rect nametable_rect = {0, 0, 256, 240};
        SDL_Rect nametable_rect1 = {256, 0, 256, 240};
        SDL_UpdateTexture(nametableBuffer0, NULL, nametable0.data(), 256 * 4);
        SDL_UpdateTexture(nametableBuffer1, NULL, nametable1.data(), 256 * 4);
        SDL_RenderCopy(nametable_renderer, nametableBuffer0, NULL, &nametable_rect);
        SDL_RenderCopy(nametable_renderer, nametableBuffer1, NULL, &nametable_rect1);
        SDL_RenderPresent(nametable_renderer);
    }

    if(sprite_window != nullptr)
    {
        std::vector<uint32_t> sprites = ppu->get_sprite();
        SDL_Rect sprite_rect = {0, 0, 256, 240};
        SDL_UpdateTexture(spriteBuffer, NULL, sprites.data(), 64 * 4);
        SDL_RenderCopy(sprite_renderer, spriteBuffer, NULL, &sprite_rect);
        SDL_RenderPresent(sprite_renderer);
    }
}

void draw_pattern_table(std::shared_ptr<PPU> ppu)
{
    if(pattern_window != nullptr)
    {
        std::vector<uint32_t> pattern0 = ppu->get_pattern_table(0);
        std::vector<uint32_t> pattern1 = ppu->get_pattern_table(1);
        //Pattern table texture
        static SDL_Rect pattern_rect0 = {0, 0, 256, 240};
        static SDL_Rect pattern_rect1 = {256, 0, 256, 240};

        SDL_UpdateTexture(patternBuffer0, NULL, pattern0.data(), 128 * sizeof(uint32_t));
        SDL_RenderCopy(pattern_renderer, patternBuffer0, NULL, &pattern_rect0); 


        SDL_UpdateTexture(patternBuffer1, NULL, pattern1.data(), 128 * sizeof(uint32_t));
        SDL_RenderCopy(pattern_renderer, patternBuffer1, NULL, &pattern_rect1);
        SDL_RenderPresent(pattern_renderer);
    }
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

void handle_events(NES* nes, bool& running, SDL_manager* manager)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                // Check if it's the nametable window
                if (event.window.windowID == SDL_GetWindowID(nametable_window))
                {
                    // Clean up resources
                    SDL_DestroyRenderer(nametable_renderer);
                    SDL_DestroyWindow(nametable_window);
                    nametable_window = nullptr; // Set to nullptr after destruction
                    nametable_renderer = nullptr; // Set to nullptr after destruction
                }
                // Check if it's the main window
                else if (event.window.windowID == SDL_GetWindowID(manager->get_window()))
                {
                    running = false; // Exit the main loop
                }

                else if(event.window.windowID == SDL_GetWindowID(pattern_window))
                {
                    SDL_DestroyRenderer(pattern_renderer);
                    SDL_DestroyWindow(pattern_window);
                    pattern_window = nullptr;
                    pattern_renderer = nullptr;
                }

                else if(event.window.windowID == SDL_GetWindowID(sprite_window))
                {
                    SDL_DestroyRenderer(sprite_renderer);
                    SDL_DestroyWindow(sprite_window);
                    sprite_window = nullptr;
                    sprite_renderer = nullptr;
                }
            }
        }
        else if (event.type == SDL_KEYDOWN)
        {
            switch(event.key.keysym.scancode)
            {
                case SDL_SCANCODE_P:
                    if (nes->is_game_loaded())
                        nes->change_pause();
                    break;

                case SDL_SCANCODE_ESCAPE:
                    running = false; // Exit the main loop
                    break;

                case SDL_SCANCODE_O:
                    if (nes->is_game_loaded())
                        nes->change_timing();
                    break;

                default: break;
            }
        }
        else if(event.type == SDL_DROPFILE)
        {
            std::string filename(event.drop.file);
            nes->load_game(filename);
            SDL_free(event.drop.file);
        }
    }   
}


void initImGui(SDL_Window* window, SDL_Renderer* renderer) 
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    ImGui::StyleColorsLight();
}

void cleanupImGui() 
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void handle_imGui(NES* nes, bool& running, SDL_Renderer* renderer)
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open")) 
            {
                nfdchar_t *outPath = NULL;
                nfdresult_t result = NFD_OpenDialog( NULL, NULL, &outPath );
                    
                if ( result == NFD_OKAY ) {
                    nes->load_game(outPath);
                    free(outPath);
                }     
            }
            if (ImGui::MenuItem("Exit", "Esc")) 
            {
                running = false;
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Game"))
        {
            if(ImGui::MenuItem("Pause","P"))
            {
                if(nes->is_game_loaded())
                    nes->change_pause();
            }

            if(ImGui::MenuItem("Reset"))
            {
                if(nes->is_game_loaded())
                    nes->reload_game();
            }

            if(ImGui::MenuItem("Alternate region (NTSC/PAL)", "O"))
            {
                if(nes->is_game_loaded())
                    nes->change_timing();
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Debug"))
        {
            if(ImGui::MenuItem("Nametable viewer"))
            {
                if(nametable_window == nullptr)
                {
                    nametable_window = SDL_CreateWindow("Nametable viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 240, 0);
                    nametable_renderer = SDL_CreateRenderer(nametable_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                    nametableBuffer0 = SDL_CreateTexture(nametable_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
                    nametableBuffer1 = SDL_CreateTexture(nametable_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
                }
            }

            if(ImGui::MenuItem("Pattern table viewer"))
            {
                if(pattern_window == nullptr)
                {
                    pattern_window = SDL_CreateWindow("Pattern table viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 240, 0);
                    pattern_renderer = SDL_CreateRenderer(pattern_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                    patternBuffer0 = SDL_CreateTexture(pattern_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);
                    patternBuffer1 = SDL_CreateTexture(pattern_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);
                    draw_pattern_table(nes->get_ppu());
                }
            }

            if(ImGui::MenuItem("Sprite viewer"))
            {
                if(sprite_window == nullptr)
                {
                    sprite_window = SDL_CreateWindow("Sprite viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256, 240, 0);
                    sprite_renderer = SDL_CreateRenderer(sprite_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                    spriteBuffer = SDL_CreateTexture(sprite_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 64);
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    } 

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}