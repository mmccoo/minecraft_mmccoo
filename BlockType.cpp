
#include <BlockType.h>
#include <iostream>
#include <vector>
#include <cassert>

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

// this was an attempt to do a map with block_type as a lookup key. It didn't work. don't know
// why. didn't feel like debugging, but it bugs me that I don't know why it didn't work.
struct cmpPalettes {
    bool operator()(const BlockType& a, const BlockType& b) const {
        if ((a.name == "minecraft:stone") && ((*(a.string_properties.find("stone_type"))).second == "granite") &&
            (b.name == "minecraft:stone") && ((*(b.string_properties.find("stone_type"))).second == "granite")) {
            std::cout << "granite\n";
        }

        if (a.name != b.name) { return a.name.compare(b.name); }

        int c = map_compare(a.byte_properties, b.byte_properties);
        if (c != 0) { return c==-1; }

        c = map_compare(a.int_properties, b.int_properties);
        if (c != 0) { return c==-1; }

        c = map_compare(a.string_properties, b.string_properties);
        if (c != 0) { return c==-1; }

        return false;
    }
};

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

BlockType& BlockType::get_block_type_by_id(int id)
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
        std::cout << iter.first << " count: " << bt_by_id[iter.second].count << " " <<
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
    ss << name << " (";

    bool first = true;
    for(auto elt : byte_properties) {
        if (!first) {
            ss << ", ";
        }
        first = false;
        ss << elt.first << ":" << std::hex << static_cast<int>(elt.second) << std::dec;
    }
    for(auto elt : int_properties) {
        if (!first) {
            ss << ", ";
        }
        first = false;
        ss << elt.first << ":" << std::hex << elt.second << std::dec;
    }
    for(auto elt : string_properties) {
        if (!first) {
            ss << ", ";
        }
        first = false;
        ss << elt.first << ":" << elt.second;
    }

    ss << ")";
    return ss.str();
}

std::ostream& operator << (std::ostream& o, const BlockType &bt)
{
    o << bt.get_string();
    return o;
}
