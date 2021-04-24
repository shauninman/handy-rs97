PRGNAME     = handy
# PRGNAME     = Lynx.pak/handy # TODO: tmp

CROSS_COMPILE := /opt/trimui-toolchain/bin/arm-buildroot-linux-gnueabi-
CC			= $(CROSS_COMPILE)gcc
CXX			= $(CROSS_COMPILE)g++

PORT = RS97
SOUND_OUTPUT = sdl12
PROFILE = APPLY

SRCDIR		= ./src/ ./src/gui/ ./src/handy-libretro src/ports ./src/ports/input/sdl src/sdlemu ./src/unzip
VPATH		= $(SRCDIR)
SRC_C		= $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.c))
SRC_CP		= $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.cpp))
OBJ_C		= $(notdir $(patsubst %.c, %.o, $(SRC_C)))
OBJ_CP		= $(notdir $(patsubst %.cpp, %.o, $(SRC_CP)))
OBJS		= $(OBJ_C) $(OBJ_CP)

CFLAGS		= -Ofast -fdata-sections -ffunction-sections -fno-common -flto -fwhole-program -fsingle-precision-constant -fno-PIC -fpermissive -march=armv5te -mtune=arm926ej-s
CFLAGS		+= -DWANT_CRC32 -DANSI_GCC -DSDL_PATCH -D$(PORT)
CFLAGS		+= -I./src -I./src/handy-libretro -I./src/sdlemu -Isrc/ports -Isrc/ports/sound -Isrc/ports/sound -Isrc/ports/input/sdl

CFLAGS 		+= -D$(PORT)
SRCDIR		+= ./src/ports/graphics/$(PORT)
SRCDIR		+= ./src/ports/sound/$(SOUND_OUTPUT)

ifeq ($(PROFILE), YES)
CFLAGS 		+= -fprofile-generate=/mnt/SDCARD/profile
else ifeq ($(PROFILE), APPLY)
CFLAGS		+= -fprofile-use=profile -fbranch-probabilities
endif

CXXFLAGS	= $(CFLAGS)
LDFLAGS     = -nodefaultlibs -lc -lstdc++ -lgcc -lgcc_s -lm -lSDL -lz -no-pie -Wl,--gc-sections -s
# LDFLAGS	   += -flto
LDFLAGS    += -lSDL_image -lSDL_ttf -ldl

ifeq ($(SOUND_OUTPUT), portaudio)
LDFLAGS		+= -lportaudio
endif
ifeq ($(SOUND_OUTPUT), libao)
LDFLAGS		+= -lao
endif
ifeq ($(SOUND_OUTPUT), alsa)
LDFLAGS		+= -lasound
endif

# Rules to make executable
$(PRGNAME): $(OBJS)  
	$(CC) $(CFLAGS) -o $(PRGNAME) $^ $(LDFLAGS)

$(OBJ_C) : %.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_CP) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(PRGNAME) *.o
