#This Makefile works on macOS.
#When use bear make to generate compile_commands.json need to reinstall make
#<brew install make>
#<export PATH=/usr/local/opt/make/libexec/gnubin:$PATH>


SRC = $(wildcard src/*.cpp)

CC = g++ -std=c++11

COMPILER_FLGAS = -w -Wall -O
INCLUDE = -Iinclude/

FRAMEWORK = -framework SDL2 -framework SDL2_image -framework SDL2_ttf

LINKER_FLAGS = -F /Library/Frameworks -I /Library/Frameworks/SDL2.framework/Headers/ -I /Library/Frameworks/SDL2_image.framework/Headers/ -I /Library/Frameworks/SDL2_ttf.framework/Headers/

OBJ_NAME = game

all: $(OBJS)
	 $(CC) $(SRC) $(COMPILER_FLGAS) $(INCLUDE) $(FRAMEWORK) $(LINKER_FLAGS) -o $(OBJ_NAME)
