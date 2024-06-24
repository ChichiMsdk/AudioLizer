NAME = editor.exe
SRC_DIR = src
SRC_GUI_DIR = src/gui
BUILD_DIR = build
OBJ_DIR = build/obj
JSON_FILE = temp.json

INCLUDE_DIRS = -IC:\Lib\tracy\public -IC:\Lib\tracy\public\tracy -Iinclude -IC:\Lib\SDL3\include
LIBS = -lSDL3 -lSDL3_image -lSDL3_ttf
LIB_PATH = -L C:\Lib\SDL3\lib
CFLAGS = -fsanitize=address -DWIN_32 -fdeclspec -MJ$(JSON_FILE)
LDFLAGS = -g -O0 -fdeclspec -MJ$(JSON_FILE)

FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_GUI_DIR)/*.c)
# OBJS = $(FILES:%.c:%.o)
# OBJS := $(patsubst $(SRC_GUI_DIR)/%.c, $(OBJ_DIR)/%.o, $(OBJS))
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(FILES))
OBJS := $(patsubst $(OBJ_DIR)/gui/%.o, $(OBJ_DIR)/%.o, $(OBJS))

all:
	.\build.bat

clang: $(BUILD_DIR)/$(NAME) database

$(BUILD_DIR)/$(NAME): $(OBJS)
	@clang $(CFLAGS) -o $@ $^ $(INCLUDE_DIRS) $(LIBS) $(LIB_PATH)
	@del $(JSON_FILE) >nul 2>&1

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@clang $(LDFLAGS) -c $< -o $@ $(INCLUDE_DIRS)
	@cat $(JSON_FILE) >> soon.json

$(OBJ_DIR)/%.o: $(SRC_GUI_DIR)/%.c
	@clang $(LDFLAGS) -c $< -o $@ $(INCLUDE_DIRS)
	@cat $(JSON_FILE) >> soon.json

database:
	@echo [ > compile_commands.json
	@cat soon.json >> compile_commands.json
	@echo ] >> compile_commands.json
	@del soon.json >nul 2>&1

clean:
	@del /s /q $(BUILD_DIR)\editor* >nul 2>&1
	@del /s /q $(BUILD_DIR)\*.pdb >nul 2>&1 
	@del /s /q $(BUILD_DIR)\*.obj >nul 2>&1
	@del /s /q $(BUILD_DIR)\*.o >nul 2>&1
	@del /s /q $(BUILD_DIR)\*.ilk >nul 2>&1

re: clean all

.PHONY: all clean re database
