NAME = key_record.exe
SRC_DIR = src
SRC_GUI_DIR = src/gui
BUILD_DIR = build
OBJ_DIR = build/obj
JSON_FILE = temp.json

INCLUDE_DIRS = -IC:\Lib\sources\fftw-3.3.10\api -IC:\Lib\tracy\public \
	-IC:\Lib\tracy\public\tracy -Iinclude -IC:\Lib\SDL\include

LIBS =-lSDL3 -lSDL3_image -lSDL3_ttf -llibfftw3-3 -llibfftw3f-3

STATICLIB =-lUser32 -lwinmm -lAdvapi32 -lShell32 -lGdi32 -lOle32 \
		   -lSetupapi -lUuid -lImm32 -lStrmiids -lVersion -lOleAut32

LIB_PATH = -L C:\Lib\debug\SDL3-dll -L C:\Lib\sources\fftwdll
# CFLAGS = -fsanitize=address -DWIN_32 -DTRACY_ENABLE
CFLAGS = -DWIN_32 -DTRACY_ENABLE
CFLAGS += -g -O3 -fdeclspec -MJ$(JSON_FILE)
CPPFLAGS = -DTRACY_ENABLE

FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_GUI_DIR)/*.c)
CPP_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(FILES))
OBJS := $(patsubst $(OBJ_DIR)/gui/%.o, $(OBJ_DIR)/%.o, $(OBJS))
CPP_OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CPP_FILES))

all: $(BUILD_DIR)/$(NAME) compile_commands.json

bat:
	.\build.bat debug dll

$(BUILD_DIR)/$(NAME): $(OBJS) $(CPP_OBJS)
	clang $(CFLAGS) -o $@ $^ $(INCLUDE_DIRS) $(LIB_PATH) $(LIBS)
	@rm -f $(JSON_FILE)

$(OBJ_DIR):
	@mkdir -p build\obj

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	-clang++ $(CPPFLAGS) -c $< -o $@ $(INCLUDE_DIRS)
	@-cat $(JSON_FILE) >> soon.json

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	clang $(CFLAGS) -c $< -o $@ $(INCLUDE_DIRS)
	@-cat $(JSON_FILE) >> soon.json

$(OBJ_DIR)/%.o: $(SRC_GUI_DIR)/%.c
	clang $(CFLAGS) -c $< -o $@ $(INCLUDE_DIRS)
	@-cat $(JSON_FILE) >> soon.json

compile_commands.json:
	@echo [ > compile_commands.json
	@-cat soon.json >> compile_commands.json
	@echo ] >> compile_commands.json
	@rm -f soon.json

clean:
	@rm -f $(BUILD_DIR)/key_record*
	@rm -f $(BUILD_DIR)/vc140.pdb
	@rm -f $(BUILD_DIR)/*.obj
	@rm -f $(BUILD_DIR)/obj/*.o
	@rm -f $(BUILD_DIR)/*.ilk
	@rm -f compile_commands.json

re: clean all

.PHONY: all clean re bat
