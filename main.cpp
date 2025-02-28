// Standard Library Headers
#include <atomic>
#include <cmath>
#include <filesystem>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
#include "windows.h"

// NES Emulator Components
#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "Cartridge.h"
#include "Bus.h"

// UI and Rendering
#include "nfd.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

// Constants
constexpr int SCALE = 3;
constexpr int SCREEN_WIDTH = 256 * SCALE;
constexpr int SCREEN_HEIGHT = 240 * SCALE;

// Clock Rates
const double MASTER_CLOCK_NTSC = 236250000.0 / 11.0;
const double MASTER_CLOCK_PAL = 26601712.5;
const double CPU_CLOCK_NTSC = MASTER_CLOCK_NTSC / 12.0;
const double CPU_CLOCK_PAL = MASTER_CLOCK_PAL / 16.0;
const double SAMPLE_RATE = 44100.0;

// APU Ratios
const double apu_ratio_NTSC = CPU_CLOCK_NTSC / SAMPLE_RATE;
const double apu_ratio_PAL = CPU_CLOCK_PAL / SAMPLE_RATE;

// Audio Buffer
constexpr int BUFFER_SIZE = 8192;
int16_t audio_buffer[BUFFER_SIZE];
uint16_t read_pos = 0;
uint16_t write_pos = 0;

// Emulator State
bool game_not_initialized = true;
std::atomic<bool> running(true);
std::mutex framebuffer_mutex;
std::vector<uint32_t> screen(256 * 240, 0);

// SDL2 Textures
SDL_Texture* screenBuffer = nullptr;
SDL_Texture* nametableBuffer0 = nullptr;
SDL_Texture* nametableBuffer1 = nullptr;
SDL_Texture* spriteBuffer = nullptr;
SDL_Texture* patternBuffer0 = nullptr;
SDL_Texture* patternBuffer1 = nullptr;

// SDL2 Windows & Renderers
SDL_Window* nametable_window = nullptr;
SDL_Renderer* nametable_renderer = nullptr;

SDL_Window* pattern_window = nullptr;
SDL_Renderer* pattern_renderer = nullptr;

SDL_Window* sprite_window = nullptr;
SDL_Renderer* sprite_renderer = nullptr;

// FPS Management
double desired_fps = 60.0;
std::atomic<double> frame_time = 1000.0 / desired_fps;
bool show_fps = false;
int FPS;
int padding;

// UI Elements
bool showWindow = false;
float windowDuration = 3.0f;  // Time in seconds to fully display the window
float fadeDuration = 0.25f;   // Time in seconds for the fade-out effect
std::chrono::time_point<std::chrono::steady_clock> windowStartTime;
using namespace std::chrono;

void audio_callback(void* userdata, Uint8* stream, int len); 

class SDL_manager
{
    public:
        SDL_manager()
        {
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
            // Initialize SDL
            if ((SDL_Init(SDL_INIT_VIDEO) | SDL_Init(SDL_INIT_AUDIO)) < 0)
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

            SDL_AudioSpec desired_spec{};
            desired_spec.freq = 44100;         // 44.1 kHz sample rate
            desired_spec.format = AUDIO_S16SYS; // 16-bit signed samples
            desired_spec.channels = 1;         // Mono audio
            desired_spec.samples = 1024;        // Buffer size (lower = less latency)
            desired_spec.callback = audio_callback;
            audio_device = SDL_OpenAudioDevice(NULL, 0, &desired_spec, NULL, 0);
            SDL_PauseAudioDevice(audio_device, 0);
            if (audio_device == 0) {
                std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
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

        SDL_AudioDeviceID get_audio_device()
        {
            return audio_device;
        }

    private:
        SDL_Renderer *renderer;
        SDL_Window *window;
        SDL_AudioDeviceID audio_device;
};

class NES
{
    public:
        NES()
        {
            cpu = std::make_shared<CPU>();
            ppu = std::make_shared<PPU>();
            cart = std::make_shared<Cartridge>();
            apu = std::make_shared<APU>();
            bus = std::make_shared<Bus>(ppu, cart, apu);

            cpu->connect_bus(bus);
            ppu->connect_bus(bus);
        }

        bool load_game(std::string filename)
        {
            reset_flag = true;
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
                    if(log.length() > 50)
                        log = log.substr(0, 50) + "...";
                    game_title = log;
                    game_not_initialized = false;
                }
            }

            else
                log = std::string("File does not have .nes extension");
            reset_flag = false;
            return game_loaded;
        }

        void run_frame()
        {
            current_frame = ppu->get_frame();
        
            while (current_frame == ppu->get_frame() && !pause && !reset_flag) 
            {          
                //NTSC NES
                if(!region)
                {
                    cpu->tick();
                    apu->tick();

                    apu_cycle_accumulator += 1;
        
                    if (apu_cycle_accumulator >= apu_ratio_NTSC) // 1 audio sample every 40.584 APU cycles
                    {
                        double alpha = apu_cycle_accumulator - apu_ratio_NTSC;
                        double previous_sample = last_sample;
                        double current_sample = apu->get_output();
                        
                        // Linear interpolation
                        double interpolated_sample = (previous_sample * (1.0 - alpha)) +( current_sample * alpha);
                        {
                            audio_buffer[write_pos] = interpolated_sample * 32767;
                            write_pos = (write_pos+1) & (BUFFER_SIZE - 1);
                        }
            
                        last_sample = current_sample;
                        apu_cycle_accumulator -= apu_ratio_NTSC;
                    }

                    ppu->tick();
                    ppu->tick();
                    ppu->tick();
                }

                else //PAL NES
                {
                    cpu->tick();
                    apu->tick();

                    apu_cycle_accumulator += 1;
        
                    if (apu_cycle_accumulator >= apu_ratio_PAL) // 1 audio sample every 40.584 APU cycles
                    {
                        double alpha = apu_cycle_accumulator - apu_ratio_PAL;
                        double previous_sample = last_sample;
                        double current_sample = apu->get_output();
                        
                        // Linear interpolation
                        double interpolated_sample = (previous_sample * (1.0 - alpha)) +( current_sample * alpha);
                        {
                            audio_buffer[write_pos] = interpolated_sample * 32767;
                            write_pos = (write_pos+1) & (BUFFER_SIZE - 1);
                        }
            
                        last_sample = current_sample;
                        apu_cycle_accumulator -= apu_ratio_PAL;
                    }

                    ppu_accumulator += 3.2;
                    while (ppu_accumulator >= 1.0)
                    { 
                        ppu->tick();
                        ppu_accumulator -= 1.0;
                    }              
                }        
            }
        }
        

        inline void change_pause(SDL_AudioDeviceID audio_device)
        {
            pause = !pause;
            SDL_PauseAudioDevice(audio_device, pause);
        }

        inline void change_timing()
        {
            region = !region;
            region_info = (region) ? "PAL" : "NTSC";
            ppu->set_ppu_timing(region);
            apu->set_timing(region);                       
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

        void reset()
        {
            cpu->soft_reset();
            ppu->soft_reset();
            cart->soft_reset();
            bus->soft_reset();
            apu->soft_reset();
            game_loaded = false;
            ppu_accumulator = 0;
            pause = false;
            log = "";
        }

        void reload_game()
        {
            reset_flag = true;
            reset();
            load_game(old_game_filename);
            reset_flag = false;
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
        
        bool get_region()
        {
            return region;
        }

        std::string get_info()
        {
            std::string info = " | Game: " + game_title + " | Active region: " + region_info;
            return info;
        }



    private:
        std::shared_ptr<CPU> cpu;
        std::shared_ptr<PPU> ppu;
        std::shared_ptr<APU> apu;
        std::shared_ptr<Cartridge> cart;
        std::shared_ptr<Bus> bus;
        bool current_frame;
        float ppu_accumulator = 0.0;
        bool region = 0; // 0: NTSC, 1: PAL
        bool pause = false;
        bool game_loaded = false;
        std::string old_game_filename;
        std::string log;
        bool zapper_connected = false;
        double apu_cycle_accumulator = 0;
        double last_sample = 0;
        bool reset_flag = false;
        std::string game_title = "";
        std::string region_info = "NTSC";

};

//SDL2
void destroy_textures();
void handle_events(NES* nes, SDL_manager* manager);
//ImGUI
void initImGui(SDL_Window* window, SDL_Renderer* renderer);
void handle_imGui(NES* nes, SDL_Renderer* renderer, SDL_manager* manager);
void cleanupImGui();
//Functions to draw
void draw_frame(std::shared_ptr<PPU> ppu, SDL_Renderer*);
void draw_pattern_table(std::shared_ptr<PPU> ppu);

void emulate_nes(NES * nes, SDL_manager* manager);

int main(int argc, char *argv[])
{
    NES nes;
    SDL_manager manager;
    if(argc > 1)
    {
        std::string filename(argv[1]);
        nes.load_game(filename);
        showWindow = true;
        windowStartTime =  std::chrono::steady_clock::now();
        SDL_SetWindowTitle(manager.get_window(), ("CalascioNES" + nes.get_info()).c_str());
    }

    initImGui(manager.get_window(), manager.get_renderer());
    screenBuffer = SDL_CreateTexture(manager.get_renderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);

    std::thread emulation_thread(emulate_nes, &nes, &manager);

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::BeginMainMenuBar();
    padding = ImGui::GetFrameHeight();
    ImGui::EndMainMenuBar();
    ImGui::Render();

    SDL_SetWindowSize(manager.get_window(), SCREEN_WIDTH, SCREEN_HEIGHT + padding);
    while (running) 
    {
        handle_events(&nes, &manager);
    
        // Render ImGui only when needed
        static auto last_ui_update = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - last_ui_update).count();
    
        if (elapsed > (1000.0 / 30.0)) // ~30 FPS for UI updates
        {
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            handle_imGui(&nes, manager.get_renderer(), &manager);
            last_ui_update = now;
        }

        draw_frame(nes.get_ppu(), manager.get_renderer());
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), manager.get_renderer());
        SDL_RenderPresent(manager.get_renderer());
        SDL_Delay(2);
    }
    

    emulation_thread.join();
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

void emulate_nes(NES *nes, SDL_manager *manager)
{
    using namespace std::chrono;
    auto last_time = high_resolution_clock::now();
    int frame_count = 0;

    while (running)
    {
        auto frame_time_ms = duration<double, std::milli>(frame_time);
        auto frame_start = high_resolution_clock::now();

        if (nes->is_game_loaded())
            nes->run_frame();

        {
            std::lock_guard<std::mutex> lock(framebuffer_mutex);
            screen = nes->get_ppu()->get_screen();
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

        auto frame_end = high_resolution_clock::now();
        auto elapsed = duration<double, std::milli>(frame_end - frame_start);

        auto remaining_time = frame_time_ms - elapsed;
        auto busy_wait_until = high_resolution_clock::now() + remaining_time;
        if(remaining_time.count() > 0)
        {
            SDL_Delay(remaining_time.count() * 0.95);
            while (high_resolution_clock::now() < busy_wait_until) {} 
        }
    }
}

void audio_callback(void* userdata, Uint8* stream, int len) 
{
    int16_t* output = reinterpret_cast<int16_t*>(stream);
    int samples_needed = len / sizeof(int16_t);
    int available_data = 0;

    available_data = (write_pos >= read_pos) ? (write_pos - read_pos) : ((BUFFER_SIZE - read_pos) + write_pos);

    if (available_data >= samples_needed) 
    {
        if (read_pos + samples_needed > BUFFER_SIZE) 
        {
            int first_chunk_size = BUFFER_SIZE - read_pos;
            std::copy(audio_buffer + read_pos, audio_buffer + BUFFER_SIZE, output);
            std::copy(audio_buffer, audio_buffer + (samples_needed - first_chunk_size), output + first_chunk_size);
        } 
        else 
            std::copy(audio_buffer + read_pos, audio_buffer + read_pos + samples_needed, output);
    
        read_pos = (read_pos + samples_needed) & (BUFFER_SIZE-1);
    }
    
    else if(game_not_initialized)
        std::fill(output, output + samples_needed, 0);
}

void draw_frame(std::shared_ptr<PPU> ppu, SDL_Renderer* renderer)
{
     
    SDL_Rect screen_rect = {0, padding , SCREEN_WIDTH, SCREEN_HEIGHT};
    {
        std::lock_guard<std::mutex> lock(framebuffer_mutex);
        SDL_UpdateTexture(screenBuffer, NULL, screen.data(), 256 * 4);
        SDL_RenderCopy(renderer, screenBuffer, NULL, &screen_rect);
    }

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

void handle_events(NES* nes, SDL_manager* manager)
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
                        nes->change_pause(manager->get_audio_device());
                    break;

                case SDL_SCANCODE_ESCAPE:
                    running = false; // Exit the main loop
                    break;

                case SDL_SCANCODE_O:
                    if (nes->is_game_loaded())
                        nes->change_timing();

                    desired_fps = (nes->get_region() > 0) ? 50 : 60;
                    frame_time = (1000.0 / desired_fps);
                    SDL_SetWindowTitle(manager->get_window(), ("CalascioNES" + nes->get_info()).c_str());
                    break;
                    case SDL_SCANCODE_R:
                        if (nes->is_game_loaded())
                            nes->reload_game();
                        break;
                    
                default: break;
            }
        }

        else if(event.type == SDL_DROPFILE)
        {
            std::string filename(event.drop.file);
            nes->load_game(filename);
            desired_fps = (nes->get_region() > 0) ? 50 : 60;
            frame_time = (1000.0 / desired_fps);
            showWindow = true;
            windowStartTime =  std::chrono::steady_clock::now();
            SDL_free(event.drop.file);
            SDL_SetWindowTitle(manager->get_window(), ("CalascioNES" + nes->get_info()).c_str());
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

void handle_imGui(NES* nes, SDL_Renderer* renderer, SDL_manager* manager)
{

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) 
        {
            if (ImGui::MenuItem("Open")) 
            {
                nes->change_pause(manager->get_audio_device());
                nfdchar_t *outPath = NULL;
                nfdresult_t result = NFD_OpenDialog( NULL, NULL, &outPath );
                nes->change_pause(manager->get_audio_device());
                if ( result == NFD_OKAY ) 
                {
                    nes->load_game(outPath);
                    showWindow = true;
                    windowStartTime =  std::chrono::steady_clock::now();
                    free(outPath);
                }
                desired_fps = (nes->get_region() > 0) ? 50 : 60;
                frame_time = (1000.0 / desired_fps);
                SDL_SetWindowTitle(manager->get_window(), ("CalascioNES" + nes->get_info()).c_str());
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
                    nes->change_pause(manager->get_audio_device());
            }

            if(ImGui::MenuItem("Reset", "R"))
            {
                desired_fps = (nes->get_region() > 0) ? 50 : 60;
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
                desired_fps = (nes->get_region() > 0) ? 50 : 60;
                frame_time = (1000.0 / desired_fps);
                SDL_SetWindowTitle(manager->get_window(), ("CalascioNES" + nes->get_info()).c_str());
            }

            if(ImGui::BeginMenu("Speed"))
            {
                if(ImGui::MenuItem("Normal"))
                {
                    desired_fps = (nes->get_region() > 0) ? 50 : 60;
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
                    desired_fps = ((nes->get_region() > 0) ? 50 : 60) * 2;
                    frame_time = (1000.0 / desired_fps);
                }  

                if(ImGui::MenuItem("Half speed"))
                {
                    desired_fps = ((nes->get_region() > 0) ? 50 : 60) / 2;
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
        SDL_SetWindowTitle(manager->get_window(), ("CalascioNES" + nes->get_info() + " | FPS: " + std::to_string(FPS)).c_str());
    

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
}