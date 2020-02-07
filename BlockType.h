
#ifndef BLOCK_TYPE_HH
#define BLOCK_TYPE_HH

#include <string>
#include <map>
#include <sstream>

#include <NBTObject.h>

#if 0
// this was an attempt to do a map with block_type as a lookup key. It didn't work. don't know
// why. didn't feel like debugging, but it bugs me that I don't know why it didn't work.
struct cmpPalettes {
    bool operator()(const BlockType& a, const BlockType& b) const {
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
#endif

class BlockType : public NBTObject {
  public:

    BlockType() : id(-1), count(0) { /* empty */ };

    const std::string &get_name() const { return name; }

    std::string get_string() const;
    std::string lookup_string() const;

    void add_byte(std::string tagname, uint8_t value) override {
        if (tagname == "infiniburn_bit") {
            infiniburn_bit = value;
        } else {
            NBTObject::add_byte(tagname, value);
        }
    };

    void add_short(std::string tagname, int16_t value) override {
        if (tagname == "val") {
            // don't know what val is but I have this here to supress the stdout
            short_properties[tagname] = value;
        } else {
            NBTObject::add_short(tagname, value);
        }
    }

    void add_int(std::string tagname, int32_t value) override {
        if (tagname == "version") {
            version = value;
        } else if (tagname == "liquid_depth") {
            liquid_depth = value;
        } else {
            NBTObject::add_int(tagname, value);
        }
    }
    void add_string(std::string tagname, std::string value) override {
        // this is not a generic nbt parser. this is a minecraft
        // parser. If the current string tag has an nbt name "name",
        // it's the name of the palette entry.
        if (tagname == "name") {
            name = value;
        } else if ((tagname == "stone_type") || (tagname == "dirt_type")) {
            stone_dirt_type = value;
        } else {
            NBTObject::add_string(tagname, value);
        }
    }

    void incr_count() { count++; };

    static int get_block_type_id(BlockType &bt);
    static void add_block_type(BlockType &bt);
    static void print_block_types();
    static BlockType &get_block_type_by_id(unsigned int id);
    static int get_block_type_id_by_name(std::string name);

private:
    int id;
    int count;
    std::string name;
    std::string stone_dirt_type;
    uint8_t infiniburn_bit;
    int version;
    int liquid_depth;


};

std::ostream& operator << (std::ostream& o, const BlockType &bt);


#endif
