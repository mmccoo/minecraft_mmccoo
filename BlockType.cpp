
#include <BlockType.h>
#include <iostream>
#include <vector>
#include <cassert>

std::set<std::string> earthly_types = {
    "minecraft:bedrock",
    "minecraft:cobblestone",
    "minecraft:dirt",
    "minecraft:farmland",
    "minecraft:mossy_cobblestone",
    "minecraft:sand",
    "minecraft:sandstone",
    "minecraft:stone",
    "minecraft:stonebrick",
    "minecraft:gravel",
    "minecraft:clay",
};

std::set<std::string> liquid_types = {
    "minecraft:flowing_lava",
    "minecraft:flowing_water",
    "minecraft:lava",
    "minecraft:magma",
    "minecraft:water"
};

void BlockType::add_string(std::string tagname, std::string value) {
    // this is not a generic nbt parser. this is a minecraft
    // parser. If the current string tag has an nbt name "name",
    // it's the name of the palette entry.
    if (tagname == "name") {
        name = value;

        if (earthly_types.find(name) != earthly_types.end()) {
            earthly = true;
        }
        if (liquid_types.find(name) != liquid_types.end()) {
            liquid = true;
        }
        if (name == "minecraft:air") {
            air = true;
        }
    } else if ((tagname == "stone_type") || (tagname == "dirt_type")) {
        stone_dirt_type = value;
    } else {
        NBTObject::add_string(tagname, value);
    }
}



// seems like there'd be something like this in stl
int mycmp(const std::string &a, const std::string &b) { return a.compare(b); }

template <typename T>
int mycmp(T a, T b) {
    if (a==b) { return 0; }
    else if (a<b) { return -1; }
    else { return 1; }
}

template <typename T>
int map_compare(const T &a, const T &b)
{
    // for the purposes of uniqness in the map, the actual order is not important.
    if (a.size() != b.size()) {
        return mycmp(a.size(), b.size());
    }

    // At this point all of the maps have the same size.
    auto itera=a.begin();
    auto iterb=b.begin();
    for(/* got error if I try to include the auto assigns here */;
        itera!=a.end(); // the check above ensures a and b are the same size.
        itera++, iterb++) {
        if ((*itera).first  != (*iterb).first)  { return (*itera).first.compare((*iterb).first); }
        if ((*itera).second != (*iterb).second) { return mycmp((*itera).second, (*iterb).second); }
    }
    return 0;
}


static std::map<std::string, int> palette;
static std::vector<BlockType> bt_by_id;

int BlockType::get_block_type_id(BlockType & bt)
{
    auto iter = palette.find(bt.lookup_string());
    if (iter != palette.end()) {
        return (*iter).second;
    }
    return -1;
}

void BlockType::add_block_type(BlockType &bt)
{
    int id = get_block_type_id(bt);
    if (id != -1) { return; }

    if (palette.empty()) {
        palette.insert(std::pair<std::string, int>("nullblock", 0));
        bt_by_id.push_back(BlockType());
        bt_by_id[0].name = "nullblock";
    }

    id = palette.size();

    palette.insert(std::pair<std::string, int>(bt.lookup_string(), id) );

    bt.id = id;
    bt_by_id.push_back(bt);
}

BlockType& BlockType::get_block_type_by_id(unsigned int id)
{
    //std::cout << "id " << id << " " << bt_by_id.size() << "\n";
    assert(id<=bt_by_id.size());
    return bt_by_id[id];
}

int BlockType::get_block_type_id_by_name(std::string name)
{
    auto iter = palette.find(name);
    if (iter==palette.end()) {
        std::cout << name << " not found\n";
        return -1;
    } else {
        std::cout << "id for " << name << " is " << (*iter).second << "\n";
        return (*iter).second;
    }

}

void BlockType::print_block_types()
{
    std::cout << "there are " << palette.size() << " palette entries\n";

    int airid = BlockType::get_block_type_id_by_name("minecraft:air ()");
    int total_blocks = 0;
    int total_non_air = 0;
    for(auto iter : palette) {
        total_blocks += bt_by_id[iter.second].count;
        if (iter.second != airid) {
            total_non_air += bt_by_id[iter.second].count;
        }
    }
    for(auto iter : palette) {
        std::cout << iter.first << " id: " << iter.second  << " count: " << bt_by_id[iter.second].count << " " <<
            (double)bt_by_id[iter.second].count / total_blocks * 100.0 << "% " <<
            (double)bt_by_id[iter.second].count / total_non_air * 100.0 << "% non air " <<
            "\n";
    }
}

// this is a string generator for the purpose of palette lookup.
// many of the block types have different configurations that I usually
// don't care about.
std::string BlockType::lookup_string() const
{
    std::stringstream ss;
    ss << name << " (";
    // add more to the string if you want to differentiate.
    ss << ")";

    return ss.str();
}

std::string BlockType::get_string() const
{
    std::stringstream ss;
    ss << name << NBTObject::get_string();

    return ss.str();
}

std::ostream& operator << (std::ostream& o, const BlockType &bt)
{
    o << bt.get_string();
    return o;
}
