CC = g++
CC_OLD = gcc
CFLAGS = -Wall -ansi -pedantic -I include -O2 -g -fopenmp
LDFLAGS = -lSDL -lGL -fopenmp

SRC_VOXEL_PATH = src_voxel_maker
SRC_TERRAIN_PATH = src_terrain_builder
SRC_DISPLAY_PATH = src_display
SRC_COMMON_PATH = src_common
DRN_PATH = $(SRC_COMMON_PATH)/drn
BIN_PATH = bin

EXEC_VOXEL = hg_voxel_maker
EXEC_TERRAIN = hg_terrain_builder
EXEC_DISPLAY = hg_display

SRC_VOXEL_FILES = $(shell find $(SRC_VOXEL_PATH) -type f -name '*.cpp')
OBJ_VOXEL_FILES = $(patsubst $(SRC_VOXEL_PATH)/%.cpp, $(SRC_VOXEL_PATH)/%.o, $(SRC_VOXEL_FILES))

SRC_TERRAIN_FILES = $(shell find $(SRC_TERRAIN_PATH) -type f -name '*.cpp')
OBJ_TERRAIN_FILES = $(patsubst $(SRC_TERRAIN_PATH)/%.cpp, $(SRC_TERRAIN_PATH)/%.o, $(SRC_TERRAIN_FILES))

SRC_DISPLAY_FILES = $(shell find $(SRC_DISPLAY_PATH) -type f -name '*.cpp')
OBJ_DISPLAY_FILES = $(patsubst $(SRC_DISPLAY_PATH)/%.cpp, $(SRC_DISPLAY_PATH)/%.o, $(SRC_DISPLAY_FILES))

SRC_COMMON_FILES = $(shell find $(SRC_COMMON_PATH) -type f -name '*.cpp')
OBJ_COMMON_FILES = $(patsubst $(SRC_COMMON_PATH)/%.cpp, $(SRC_COMMON_PATH)/%.o, $(SRC_COMMON_FILES))

SRC_DRN = $(shell find $(DRN_PATH) -type f -name '*.c')
OBJ_DRN = $(patsubst $(DRN_PATH)/%.c, $(DRN_PATH)/%.o, $(SRC_DRN))

all: $(BIN_PATH)/$(EXEC_VOXEL) $(BIN_PATH)/$(EXEC_DISPLAY) $(BIN_PATH)/$(EXEC_TERRAIN)
	@echo [--FINISHED--]

$(BIN_PATH)/$(EXEC_VOXEL): $(OBJ_VOXEL_FILES) $(OBJ_COMMON_FILES)  $(SRC_COMMON_PATH)/glew-1.9/glew.o $(OBJ_DRN)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_PATH)/$(EXEC_TERRAIN): $(OBJ_TERRAIN_FILES) 
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_PATH)/$(EXEC_DISPLAY): $(OBJ_DISPLAY_FILES) $(SRC_COMMON_PATH)/glew-1.9/glew.o $(OBJ_COMMON_FILES) $(OBJ_DRN)
	$(CC) -o $@ $^ $(LDFLAGS)

$(SRC_COMMON_PATH)/glew-1.9/glew.o: $(SRC_COMMON_PATH)/glew-1.9/glew.c
	$(CC_OLD) -c -o $@ $(CFLAGS) $^ 
	@echo [--$@ compiled--]
	@echo 

$(DRN_PATH)/%.o: $(DRN_PATH)/%.c
	$(CC_OLD) -c -o $@ $(CFLAGS) $^
	@echo [--$@ compiled--]
	@echo 

$(SRC_VOXEL_PATH)/%.o: $(SRC_VOXEL_PATH)/%.cpp
	$(CC) -c -o $@ $(CFLAGS) $^ 
	@echo [--$@ compiled--]
	@echo 

$(SRC_TERRAIN_PATH)/%.o: $(SRC_TERRAIN_PATH)/%.cpp
	$(CC) -c -o $@ $(CFLAGS) $^ 
	@echo [--$@ compiled--]
	@echo 

$(SRC_DISPLAY_PATH)/%.o: $(SRC_DISPLAY_PATH)/%.cpp
	$(CC) -c -o $@ $(CFLAGS) $^ 
	@echo [--$@ compiled--]
	@echo 

$(SRC_COMMON_PATH)/%.o: $(SRC_COMMON_PATH)/%.cpp
	$(CC) -c -o $@ $(CFLAGS) $^ 
	@echo [--$@ compiled--]
	@echo 

clean:
	rm $(OBJ_VOXEL_FILES) $(OBJ_TERRAIN_FILES) $(OBJ_DISPLAY_FILES) $(OBJ_COMMON_FILES) $(SRC_COMMON_PATH)/glew-1.9/glew.o $(OBJ_DRN)

cleanall:
	rm $(BIN_PATH)/$(EXEC_VOXEL) $(BIN_PATH)/$(EXEC_TERRAIN) $(BIN_PATH)/$(EXEC_DISPLAY) $(OBJ_VOXEL_FILES) $(OBJ_TERRAIN_FILES) $(OBJ_DISPLAY_FILES) $(OBJ_COMMON_FILES) $(SRC_COMMON_PATH)/glew-1.9/glew.o $(OBJ_DRN)
