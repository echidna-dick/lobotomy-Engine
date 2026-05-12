# === Compiler ===
CXX      := g++
CC       := gcc

# === Flags ===
CXXFLAGS := -O2 -Wall -std=c++17 -I src -I vendor/imgui -I /mingw64/include/SDL2
CFLAGS   := -O2 -Wall -std=c99   -I src
LIBS     := -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer

# === Sources ===
SRC_CPP  := src/engine.cpp \
            vendor/imgui/imgui.cpp \
            vendor/imgui/imgui_draw.cpp \
            vendor/imgui/imgui_widgets.cpp \
            vendor/imgui/imgui_tables.cpp \
            vendor/imgui/imgui_demo.cpp \
            vendor/imgui/imgui_impl_sdl2.cpp \
            vendor/imgui/imgui_impl_sdlrenderer2.cpp

SRC_C    := src/world_map.c \
            src/player.c \
            src/minimap.c \
            src/textures.c

# === Object files ===
OBJ_CPP  := $(SRC_CPP:%.cpp=build/%.o)
OBJ_C    := $(SRC_C:%.c=build/%.o)
OBJ      := $(OBJ_CPP) $(OBJ_C)

# === Output ===
TARGET   := build/lobotomy-engine.exe

# === Rules ===
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LIBS)

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build

.PHONY: all run clean