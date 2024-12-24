#include <iostream>
#include <vector>
#include <cstdint>
#include "CPU.h"
#include "PPU.h"
#include "Cartridge.h"
#include "Bus.h"
#include "nfd.h"
#include <filesystem>
#include <chrono>
#include <thread>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

constexpr int SCALE = 3;
constexpr int SCREEN_WIDTH = 256 * SCALE;
constexpr int SCREEN_HEIGHT = 240 * SCALE;

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
            window = SDL_CreateWindow("CalascioNES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT + 19, 0);
            if (!window)
            {
                std::cerr << "Failed to open " << SCREEN_WIDTH << " x " << SCREEN_HEIGHT << " window: " << SDL_GetError() << std::endl;
                exit(1);
            }

            // Create the renderer with hardware acceleration
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
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

        inline SDL_Renderer* get_renderer()
        {
            return renderer;
        }

        inline SDL_Window* get_window()
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

        bool load_game(std::string filename)
        {
            std::string extension = std::filesystem::path(filename).extension().string();
            if(extension == ".nes" || extension == ".NES")
            {
                old_game_filename = filename;           
                reset();
                if(cart->load_game(filename, log))
                    game_loaded = true;
                if(std::filesystem::is_regular_file(log))
                {
                    log = std::filesystem::path(log).stem().string();
                }
            }

            else
                log = std::string("File does not have .nes extension");

            return game_loaded;
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
                if(log_cpu)
                    std::cout << std::hex <<(int)cpu->get_opcode() << std::endl;
            }      
        }

        inline void change_pause()
        {
            pause = !pause;
        }

        inline void change_timing()
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

        inline bool is_game_loaded()
        {
            return game_loaded;
        }

        inline std::shared_ptr<PPU> get_ppu()
        {
            return ppu;
        }

        inline std::shared_ptr<CPU> get_cpu()
        {
            return cpu;
        }

        void set_log_cpu()
        {
            log_cpu = !log_cpu;
        }

        void reset()
        {
            cpu->soft_reset();
            ppu->soft_reset();
            cart->soft_reset();
            bus->soft_reset();
            game_loaded = false;
            ppu_accumulator = 0;
            pause = false;
            log = "";
        }

        void reload_game()
        {
            reset();
            load_game(old_game_filename);
        }

        void alternate_zapper()
        {
            zapper_connected = !zapper_connected;
            bus->set_zapper(zapper_connected);
        }

        bool get_zapper()
        {
            return zapper_connected;
        }

        void send_mouse_coordinates(int x, int y)
        {
            bus->update_zapper_coordinates(x, y);
        }

        void fire_zapper()
        {
            bus->fire_zapper();
        }

        std::string get_log()
        {
            return log;
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
        bool log_cpu = false;
        std::string log;
        bool zapper_connected = false;

};


//SDL2
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

int desired_fps = 60;
double frame_time = (1000.0 / desired_fps);
bool show_fps = false;
int FPS;
int padding;

bool showWindow = false;
float windowDuration = 3.0f;  // Time in seconds to fully display the window
float fadeDuration = 0.25f;    // Time in seconds for the fade-out effect
std::chrono::time_point<std::chrono::steady_clock> windowStartTime;

using namespace std::chrono;

int main(int argc, char *argv[])
{
    NES nes;
    SDL_manager manager;
    bool running = true;

    if(argc > 1)
    {
        std::string filename(argv[1]);
        nes.load_game(filename);
        showWindow = true;
        windowStartTime =  std::chrono::steady_clock::now();
    }

    initImGui(manager.get_window(), manager.get_renderer());
    screenBuffer = SDL_CreateTexture(manager.get_renderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::BeginMainMenuBar();
    padding = ImGui::GetFrameHeight();
    ImGui::EndMainMenuBar();
    ImGui::Render();

    SDL_SetWindowSize(manager.get_window(), SCREEN_WIDTH, SCREEN_HEIGHT + padding);

    auto last_time = std::chrono::high_resolution_clock::now();
    int frame_count = 0;

    while (running) 
    {
        handle_events(&nes, running, &manager);

        // Start ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Clear screen to black
        SDL_SetRenderDrawColor(manager.get_renderer(), 0, 0, 0, 255);
        SDL_RenderClear(manager.get_renderer());

        // Frame timing start
        auto frame_start = high_resolution_clock::now();

        if (nes.is_game_loaded())
        {
            nes.run_frame(); // Emulate a frame
            draw_frame(nes.get_ppu(), manager.get_renderer()); // Render frame
        }

        handle_imGui(&nes, running, manager.get_renderer()); // Render ImGui UI
        SDL_RenderPresent(manager.get_renderer()); // Display rendered frame

        auto frame_end = high_resolution_clock::now();
        duration<double, std::milli> frame_duration = frame_end - frame_start;
        while (frame_duration.count() < frame_time)
        {
            frame_end = high_resolution_clock::now();
            frame_duration = frame_end - frame_start;
        }

        // FPS calculation
        frame_count++;
        auto current_time = high_resolution_clock::now();
        duration<double> elapsed_seconds = current_time - last_time;
        
        if (elapsed_seconds.count() >= 1.0)
        {
            FPS = frame_count;
            frame_count = 0;
            last_time = current_time;
        }
    }

    SDL_DestroyRenderer(nametable_renderer);
    SDL_DestroyRenderer(pattern_renderer);
    SDL_DestroyRenderer(sprite_renderer);
    SDL_DestroyWindow(nametable_window);
    SDL_DestroyWindow(pattern_window);
    SDL_DestroyWindow(sprite_window);
    cleanupImGui();
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
                case SDL_SCANCODE_U:
                    nes->set_log_cpu();
                    break;
                

                default: break;
            }
        }

        else if(event.type == SDL_DROPFILE)
        {
            desired_fps = 60;
            frame_time = (1000.0 / desired_fps);
            std::string filename(event.drop.file);
            nes->load_game(filename);
            showWindow = true;
            windowStartTime =  std::chrono::steady_clock::now();
            SDL_free(event.drop.file);
        }

        else if ((event.type == SDL_MOUSEBUTTONUP) && nes->get_zapper()) 
        {
            nes->fire_zapper();
        }

        else if((event.type == SDL_MOUSEBUTTONDOWN) && nes->get_zapper())
        {
            // Get the mouse coordinates on release
            int x = event.button.x;
            int y = event.button.y - ImGui::GetFrameHeight();

            nes->send_mouse_coordinates(int(x / SCALE), int(y / SCALE));
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
                    
                if ( result == NFD_OKAY ) 
                {
                    desired_fps = 60;
                    frame_time = (1000.0 / desired_fps);
                    nes->load_game(outPath);
                    showWindow = true;
                    windowStartTime =  std::chrono::steady_clock::now();
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
                desired_fps = 60;
                frame_time = (1000.0 / desired_fps);
                if(nes->is_game_loaded())
                    nes->reload_game();
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Settings"))
        {
            if(ImGui::MenuItem("Alternate region (NTSC/PAL)", "O"))
            {
                if(nes->is_game_loaded())
                    nes->change_timing();
            }

            if(ImGui::BeginMenu("Speed"))
            {
                if(ImGui::MenuItem("Normal"))
                {
                    desired_fps = 60;
                    frame_time = (1000.0 / desired_fps);
                }

                if(ImGui::MenuItem("Increase speed"))
                {
                    if(desired_fps < 300 && frame_time)
                    {
                        desired_fps += 30;
                        frame_time = (1000.0 / desired_fps);
                    }
                }

                if(ImGui::MenuItem("Decrease speed"))
                {
                    if(desired_fps > 30)
                    {
                        if(frame_time == 0)
                            desired_fps = 60;
                        else
                            desired_fps -= 30;
                        frame_time = (1000.0 / desired_fps);
                    }

                    else 
                    {
                        desired_fps = floor(desired_fps / 2);
                        frame_time = 1000.0 / desired_fps;
                    }
                }

                if(ImGui::MenuItem("Maximum speed"))
                {
                    frame_time = 0.0;
                }
                ImGui::Separator();

                if(ImGui::MenuItem("Double speed"))
                {
                    desired_fps = 120;
                    frame_time = (1000.0 / desired_fps);
                }  

                if(ImGui::MenuItem("Half speed"))
                {
                    desired_fps = 30;
                    frame_time = (1000.0 / desired_fps);
                }               

                ImGui::Separator();

                if(ImGui::MenuItem("Show FPS"))
                {
                    show_fps = !show_fps;
                }       
                ImGui::EndMenu();
            }

            if(ImGui::MenuItem("Zapper"))
            {
                nes->alternate_zapper();
            }

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Debug"))
        {
            if(ImGui::MenuItem("Nametable viewer"))
            {
                if(nametable_window == nullptr && nes->is_game_loaded())
                {
                    nametable_window = SDL_CreateWindow("Nametable viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 240, 0);
                    nametable_renderer = SDL_CreateRenderer(nametable_window, -1, SDL_RENDERER_ACCELERATED);
                    nametableBuffer0 = SDL_CreateTexture(nametable_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
                    nametableBuffer1 = SDL_CreateTexture(nametable_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
                }
            }

            if(ImGui::MenuItem("Pattern table viewer"))
            {
                if(pattern_window == nullptr && nes->is_game_loaded())
                {
                    pattern_window = SDL_CreateWindow("Pattern table viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 240, 0);
                    pattern_renderer = SDL_CreateRenderer(pattern_window, -1, SDL_RENDERER_ACCELERATED);
                    patternBuffer0 = SDL_CreateTexture(pattern_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);
                    patternBuffer1 = SDL_CreateTexture(pattern_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);
                    draw_pattern_table(nes->get_ppu());
                }
            }

            if(ImGui::MenuItem("Sprite viewer"))
            {
                if(sprite_window == nullptr && nes->is_game_loaded())
                {
                    sprite_window = SDL_CreateWindow("Sprite viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256, 240, 0);
                    sprite_renderer = SDL_CreateRenderer(sprite_window, -1, SDL_RENDERER_ACCELERATED);
                    spriteBuffer = SDL_CreateTexture(sprite_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 64);
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if(show_fps)
    {
        ImGui::SetNextWindowPos(ImVec2(SCREEN_WIDTH - 80, ImGui::GetFrameHeight())); // Set position to top-left corner
        ImGui::Begin("Label Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // White color
        ImGui::Text(("FPS: " + std::to_string(FPS)).c_str());
        ImGui::PopStyleColor(); // Restore the previous text color
        ImGui::End();
    }

    if(showWindow)
    {
        float elapsedTime = std::chrono::duration<float>(std::chrono::steady_clock::now() - windowStartTime).count();

        float alpha = 1.0f;
        if (elapsedTime > windowDuration) {
            float fadeElapsed = elapsedTime - windowDuration;
            alpha = 1.0f - (fadeElapsed / fadeDuration);

            if (alpha <= 0.0f) {
                alpha = 0.0f;
                showWindow = false;
            }
        }

        ImGui::SetNextWindowPos(ImVec2(10, SCREEN_HEIGHT -  30));
        ImGui::SetNextWindowBgAlpha(alpha);
        ImGui::Begin("Fading Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // White color
        ImGui::Text(nes->get_log().c_str());
        ImGui::PopStyleColor(); // Restore the previous text color
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}


/*     for (const auto& entry : std::filesystem::directory_iterator("TESTS")) 
    {
        if (entry.path().extension() == ".json") {
            nes.get_cpu()->open_file(entry.path().string());
            bool test_passed = true;
            // Test each case in the file
            for (int i = 0; i < 9999; i++) {

                nes.get_cpu()->load(i);

                // Tick until test is finished
                while (!nes.get_cpu()->finished()) {
                    nes.get_cpu()->tick();
                }

                // Compare the result
                if (!nes.get_cpu()->compare(i)) {
                    test_passed = false;
                    std::cout << "Opcode 0x"
                              << std::hex << static_cast<int>(nes.get_cpu()->get_opcode())
                              << " failed in test case " << i 
                              << " from file " << entry.path().filename().string() 
                              << std::endl;
                    break;  // Stop further testing for this opcode if it fails
                }
            }

            // After testing all cases for this file, print result
            if (test_passed) {
                std::cout << "Opcode 0x"
                          << std::hex << static_cast<int>(nes.get_cpu()->get_opcode())
                          << " passed all tests in file " << entry.path().filename().string() 
                          << std::endl;
            }
        }
    }

    int i;
    std::cin >> i;  // Wait for user input before closing */