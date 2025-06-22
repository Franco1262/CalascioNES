# Compiler and flags
CXX := g++
CXXFLAGS := -O3 -flto -march=native -fomit-frame-pointer -funroll-loops -Wall -mwindows

# Include paths
INCLUDES := \
    -I./nativefiledialog/src/include \
    -I./include \
    -I./imgui \
    -I./imgui/backends \
    -I./SDL2/x86_64-w64-mingw32/include/SDL2

# Library paths and libraries
LDFLAGS := \
    -L./SDL2/x86_64-w64-mingw32/lib \
    -L./nativefiledialog/build/lib/Release/x64

LDLIBS := \
    -lmingw32 -lSDL2main -lSDL2 -lnfd -lcomctl32 -lole32 -luuid -lshell32

# Sources
SRC := \
    main.cpp \
    src/CPU.cpp \
    src/PPU.cpp \
    src/Cartridge.cpp \
    src/Bus.cpp \
    src/NROM.cpp \
    src/UxROM.cpp \
    src/CNROM.cpp \
    src/SxROM.cpp \
    src/AxROM.cpp \
    src/TxROM.cpp \
    src/APU.cpp \
    src/NES.cpp \
    imgui/imgui.cpp \
    imgui/imgui_draw.cpp \
    imgui/imgui_tables.cpp \
    imgui/imgui_widgets.cpp \
    imgui/backends/imgui_impl_sdl2.cpp \
    imgui/backends/imgui_impl_sdlrenderer2.cpp

# Output
TARGET := main.exe

# Build rule
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(INCLUDES) $(LDFLAGS) $(LDLIBS) -o $@

# Clean rule
clean:
	rm -f $(TARGET)
