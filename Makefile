#This Makefile works for both Linux and macOS.
#When use bear make to generate compile_commands.json need to reinstall make
#<brew install make>
#<export PATH=/usr/local/opt/make/libexec/gnubin:$PATH>


SRC = $(wildcard src/*.cpp)

CC = g++ -std=c++11

COMPILER_FLGAS = -w -Wall -O3
INCLUDE = -Iinclude/

FRAMEWORK = -framework SDL2 -framework SDL2_image -framework SDL2_ttf

LINKER_FLAGS = -F /Library/Frameworks -I /Library/Frameworks/SDL2.framework/Headers/ -I /Library/Frameworks/SDL2_image.framework/Headers/ -I /Library/Frameworks/SDL2_ttf.framework/Headers/

OBJ_NAME = game

mac: $(OBJS)
	 $(CC) $(SRC) $(COMPILER_FLGAS) $(INCLUDE) $(FRAMEWORK) $(LINKER_FLAGS) -o $(OBJ_NAME)


LINKER_FLAGS_LINUX = -lSDL2 -lSDL2_image -lSDL2_ttf
COMPILER_FLGAS_PROF = -w -Wall -pg

linux: $(OBJS)
	 $(CC) $(SRC) $(COMPILER_FLGAS) $(INCLUDE) $(LINKER_FLAGS_LINUX) -o $(OBJ_NAME)

linux_prof: $(OBJS)
	 $(CC) $(SRC) $(COMPILER_FLGAS_PROF) $(INCLUDE) $(LINKER_FLAGS_LINUX) -o $(OBJ_NAME)_prof
