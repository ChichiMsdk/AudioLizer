NAME = key_record.exe
SRC_DIR = src
SRC_GUI_DIR = src/gui
BUILD_DIR = build
OBJ_DIR = build/obj
JSON_FILE = temp.json
SHELL := cmd.exe

INCLUDE_DIRS = -IC:\Lib\sources\fftw-3.3.10\api -IC:\Lib\tracy\public -IC:\Lib\tracy\public\tracy -Iinclude -IC:\Lib\SDL\include
LIBS =-lSDL3 -lSDL3_image -lSDL3_ttf -lSDL3_mixer -llibfftw3-3 -llibfftw3f-3

STATICLIB =-lUser32 -lwinmm -lAdvapi32 -lShell32 -lGdi32 -lOle32 \
		   -lSetupapi -lUuid -lImm32 -lStrmiids -lVersion -lOleAut32

LIB_PATH = -L C:\Lib\debug\SDL3-dll -L C:\Lib\sources\fftwdll
# CFLAGS = -fsanitize=address -DWIN_32 -DTRACY_ENABLE
CFLAGS = -DWIN_32 -DTRACY_ENABLE
CFLAGS += -g -O0 -fdeclspec -MJ$(JSON_FILE)
CPPFLAGS += $(CFLAGS)

FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_GUI_DIR)/*.c)
CPP_FILES = src/TracyClient.cpp
# OBJS = $(FILES:%.c:%.o)
# OBJS := $(patsubst $(SRC_GUI_DIR)/%.c, $(OBJ_DIR)/%.o, $(OBJS))
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(FILES))
OBJS := $(patsubst $(OBJ_DIR)/gui/%.o, $(OBJ_DIR)/%.o, $(OBJS))
CPP_OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CPP_FILES))

all:
	.\build.bat debug dll

clang: $(BUILD_DIR)/$(NAME) database

$(BUILD_DIR)/$(NAME): $(OBJS) $(CPP_OBJS)
	@clang $(CFLAGS) -o $@ $^ $(INCLUDE_DIRS) $(LIB_PATH) $(LIBS)
	@del $(JSON_FILE) >nul 2>&1

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@clang++ $(CPPFLAGS) -c $< -o $@ $(INCLUDE_DIRS)
	@cat $(JSON_FILE) >> soon.json

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@clang $(CFLAGS) -c $< -o $@ $(INCLUDE_DIRS)
	@cat $(JSON_FILE) >> soon.json

$(OBJ_DIR)/%.o: $(SRC_GUI_DIR)/%.c
	@clang $(CFLAGS) -c $< -o $@ $(INCLUDE_DIRS)
	@cat $(JSON_FILE) >> soon.json

database:
	@echo [ > compile_commands.json
	@cat soon.json >> compile_commands.json
	@echo ] >> compile_commands.json
	@del soon.json >nul 2>&1

clean:
	@del /s /q $(BUILD_DIR)\key_record* >nul 2>&1
	@del /s /q $(BUILD_DIR)\*.obj >nul 2>&1
	@del /s /q $(BUILD_DIR)\obj\*.o >nul 2>&1
	@del /s /q $(BUILD_DIR)\*.ilk >nul 2>&1

re: clean clang

.PHONY: all clean re database
