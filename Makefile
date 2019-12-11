




all: type_mst basic_test show_clusters


# add -pg for profiling
OPT = -ggdb3
OPT = -O2 -ggdb3

CPPFLAGS = -Wall -std=gnu++14 $(OPT) -pthread -DDLLX= -I. -I/bubba/electronicsDS/minecraft/leveldb-mcpe/include -std=c++0x -fno-builtin-memcmp -pthread -DOS_LINUX -DLEVELDB_PLATFORM_POSIX -DLEVELDB_ATOMIC_PRESENT -DSNAPPY

DEPFLAGS = -MT $@ -MMD -MP -MF $*.Td
CPPFLAGS += $(DEPFLAGS)

COMMON_OBJS := parse_bedrock.o MinecraftWorld.o BlockType.o SubChunk.o

%.o:%.c
	g++ $(CPPFLAGS) $(DEPFLAGS) -c -o $@ $<

basic_test: basic_test.o $(COMMON_OBJS)
	g++ $(OPT)  -L/bubba/electronicsDS/minecraft/leveldb-mcpe/out-static/ -o $@ $^ -lleveldb -lsnappy -lpthread -lz



type_mst: type_mst.o compute_mst.o $(COMMON_OBJS)
	g++ $(OPT)  -L/bubba/electronicsDS/minecraft/leveldb-mcpe/out-static/ -o $@ $^ -lleveldb -lsnappy -lpthread -lz	-lCGAL -lgmp -lboost_program_options

show_clusters: show_clusters.o compute_mst.o compute_ore_stats.o $(COMMON_OBJS)
	g++ $(OPT)  -L/bubba/electronicsDS/minecraft/leveldb-mcpe/out-static/ -o $@ $^ -lleveldb -lsnappy -lpthread -lz	-lCGAL -lgmp -lboost_program_options

clean:
	rm *o  *.Td type_mst basic_test



include $(wildcard *.Td)
