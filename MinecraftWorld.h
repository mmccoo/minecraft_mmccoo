

#ifndef MINECRAFT_WORLD_HH
#define MINECRAFT_WORLD_HH

#include <map>
#include <vector>
#include <optional>

#include <SubChunk.h>
#include <biome.h>
#include <NBTObject.h>

// based on https://www.artificialworlds.net/blog/2017/05/11/c-iterator-example-and-an-iterable-range/

class MinecraftWorld {
  public:
    void write_world_js(std::string filename);


    std::map<int, std::map<int, std::map<int, SubChunk> > > theworld;
    std::map<int, std::map<int, Grid16> > chunk_biomes;

    std::map<int, std::map<int, Grid16> > chunk_elevation;

    std::map<int, std::map<int, std::vector<EntityType*> > > chunk_entities;
    std::map<long, EntityType*> entities_by_id;

    std::map<int, std::map<int, std::vector<BlockEntityType> > > chunk_block_entities;
    // can't populate this table during parsing because entities can move around due to
    // vector resizing.
    void populate_entity_table();

    int num_chunks();

    void    set_type_at(int x, int y, int z, uint8_t type);

    // a return value of 0 means undefined. It's not in the database
    uint8_t get_type_at(int x, int y, int z);

    SubChunk &get_sub_chunk(int chunkx, int chunky, int chunkz);

    VillageType &get_village(std::string vname) {
        auto it = villages.find(vname);
        if (it == villages.end()) {
            return (*(villages.insert(std::make_pair(vname, VillageType(vname))).first)).second;
        } else {
            return (*it).second;
        }
    }

    std::optional<EntityType *> get_entity_by_id(long id) {
        auto it = entities_by_id.find(id);
        if (it == entities_by_id.end()) {
            return std::optional<EntityType *>();
        } else {
            EntityType *e = (*it).second;
            return std::optional<EntityType *>(e);
        }
    }

    std::map<std::string, VillageType> villages;

};



#endif
