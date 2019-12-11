
#ifndef BLOCK_TYPE_HH
#define BLOCK_TYPE_HH

#include <string>
#include <map>
#include <sstream>

class BlockType {
  public:

    BlockType() : id(-1), count(0) { /* empty */ };

    std::string get_string() const;
    std::string lookup_string() const;

    std::string name;
    int id;
    std::map<std::string, uint8_t> byte_properties;
    std::map<std::string, int32_t> int_properties;
    std::map<std::string, std::string> string_properties;

    int count;

    static int get_block_type_id(BlockType &bt);
    static void add_block_type(BlockType &bt);
    static void print_block_types();
    static BlockType &get_block_type_by_id(int id);
    static int get_block_type_id_by_name(std::string name);
};

std::ostream& operator << (std::ostream& o, const BlockType &bt);


#endif
