



all: type_mst basic_test show_clusters write_top


.SUFFIXES:

# add -pg for profiling
OPT = -ggdb3
#OPT = -O2 -ggdb3

CPPFLAGS = -Wall -std=c++17 $(OPT) -pthread -DDLLX= -I. -I../leveldb-mcpe/include -fno-builtin-memcmp -pthread -DOS_LINUX -DLEVELDB_PLATFORM_POSIX -DLEVELDB_ATOMIC_PRESENT -DSNAPPY

LDDFLAGS = -L../leveldb-mcpe/out-static -lleveldb -L../snappy/install/lib -lsnappy -lpthread -lz -lboost_program_options -lboost_filesystem -lboost_system

ifneq ("$(wildcard /usr/lib/x86_64-linux-gnu/ImageMagick-6.9.7/bin-q16/Magick++-config)","")
	IMAGICK_CPP = $(shell /usr/lib/x86_64-linux-gnu/ImageMagick-6.9.7/bin-q16/Magick++-config  --cxxflags)
	IMAGICK_LDD = $(shell /usr/lib/x86_64-linux-gnu/ImageMagick-6.9.7/bin-q16/Magick++-config  --ldflags)
else
	IMAGICK_CPP = $(shell Magick++-config  --cxxflags)
	IMAGICK_LDD = $(shell Magick++-config  --ldflags)
endif

DEPFLAGS = -MT $@ -MMD -MP -MF $*.Td
CPPFLAGS += $(DEPFLAGS)

COMMON_OBJS := parse_bedrock.o MinecraftWorld.o BlockType.o SubChunk.o polygon.o biome.o BiomeVectors.o BiomeImage.o NBTObject.o EntityJSON.o BlockEntityJSON.o VillageJSON.o ElevationVectors.o CaveVectors.o ResourceVectors.o

%.o:%.cpp
	echo IMAGIC $(IMAGICK_CPP)
	g++ $(CPPFLAGS) $(IMAGICK_CPP) $(DEPFLAGS) -c -o $@ $<

basic_test: basic_test.o $(COMMON_OBJS)
	g++ $(OPT) $(IMAGICK_LDD) -o $@ $^ $(IMAGICK_LDD) $(LDDFLAGS)

type_mst: type_mst.o compute_mst.o $(COMMON_OBJS)
	g++ $(OPT) -o $@ $^ -lCGAL -lgmp $(IMAGICK_LDD) $(LDDFLAGS)

show_clusters: show_clusters.o compute_mst.o compute_ore_stats.o $(COMMON_OBJS)
	g++ $(OPT) -o $@ $^ -lCGAL -lgmp $(IMAGICK_LDD) $(LDDFLAGS)

write_top: write_top.o $(COMMON_OBJS)
	g++ $(OPT) -o $@ $^ -lCGAL -lgmp $(IMAGICK_LDD) $(LDDFLAGS)

clean:
	rm *o  *.Td type_mst basic_test



include $(wildcard *.Td)
