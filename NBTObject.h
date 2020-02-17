

#pragma once

#include <string>
#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <sstream>
#include <optional>

#include <cassert>

class NBTObject {

  public:
    static std::string indent;

    virtual std::string get_string() const;


    virtual void add_byte(std::string tagname, uint8_t value) {
        std::cout << indent << "byte " << tagname << ":" <<  std::hex << (unsigned int) value << std::dec << "\n";
        byte_properties[tagname] = value;
    };

    virtual void add_int(std::string tagname, int32_t value) {
        std::cout << indent << "int " << tagname << "(" << value << ")\n";
        int_properties[tagname] = value;
    }

    virtual void add_short(std::string tagname, int16_t value) {
        std::cout << indent << "short " << tagname << "(" << value << ")\n";
        short_properties[tagname] = value;
    }

    virtual void add_string(std::string tagname, std::string value) {
        std::cout << indent << "astring " << tagname << ":" << value << ":\n";
        string_properties[tagname] = value;
    }

    virtual void add_long(std::string tagname, long value) {
        std::cout << indent << "long " << tagname << "(" << value << ")\n";
        long_properties[tagname] = value;
    }

    virtual std::optional<uint8_t> get_byte_property(std::string tagname) {
        auto i = byte_properties.find(tagname);
        if (i==byte_properties.end()) {
            return std::optional<uint8_t>();
        } else {
            return (*i).second;
        }

    }

    virtual std::optional<std::string> get_string_property(std::string tagname) {
        auto i = string_properties.find(tagname);
        if (i==string_properties.end()) {
            return std::optional<std::string>();
        } else {
            return (*i).second;
        }

    }

  protected:
    std::map<std::string, uint8_t> byte_properties;
    std::map<std::string, int32_t> int_properties;
    std::map<std::string, int16_t> short_properties;
    std::map<std::string, std::string> string_properties;
    std::map<std::string, long> long_properties;
};


class Attribute : public NBTObject {
  public:
    std::string get_string() const override {
        std::stringstream ss;
        ss << "Attribute: " << name << "(" << Base << "," << Current << "," << Max << "," << Amount;
        ss << NBTObject::get_string();
        return ss.str();
    }

    virtual void add_string(std::string tagname, std::string value) override {
        if (tagname == "Name") {
            name = value;
        } else {
            NBTObject::add_string(tagname, value);
        }
    }

    virtual void add_float(std::string tagname, float value) {
        if (tagname == "Base") {
            Base = value;
        } else if (tagname == "Current") {
            Current = value;
        } else if (tagname == "Max") {
            Max = value;
        } else if (tagname == "Amount") {
            Amount = value;
        } else {
            std::cout << "unexpected attr tag " << tagname << "\n";
        }
    }
  private:
    std::string name;
    float Base;
    float Current;
    float Max;
    float Amount;
};

class TupleType : public NBTObject {
  public:
    void add_value(float value) {
        values.push_back(value);
    }
    float get_value(int i) { return values[i]; }

    std::string get_string();
  private:
    std::vector<float> values;
};

class PositionType : public TupleType {
    // empty
};

class RotationType : public TupleType {
    // empty. the action is in the tuple. this type is just for error catching.
};

class MotionType : public TupleType {
    // empty
};

class DefinitionsType : public NBTObject {
  public:
    std::set<std::string> get_defs() { return defs; }

    void add_string(std::string tagname, std::string value) override {
        if (tagname == "") {
            defs.insert(value);
        } else {
            NBTObject::add_string(tagname, value);
        }
    }

  private:
    std::set<std::string> defs;
};

class EquipmentType : public NBTObject {
  public:
    void add_byte(std::string tagname, uint8_t value) override {
        if (tagname == "Count") {
            Count = value;
        } else {
            NBTObject::add_byte(tagname, value);
        }
    }

    void add_short(std::string tagname, int16_t value) override {
        if (tagname == "Damage") {
            Damage = value;
        } else {
            NBTObject::add_short(tagname, value);
        }
    }

    void add_int(std::string tagname, int32_t value) override {
        if (tagname == "Damage") {
            Damage = value;
        } else {
            NBTObject::add_int(tagname, value);
        }
    }

    void add_string(std::string tagname, std::string value) override {
        if (tagname == "Name") {
            name = value;
        } else {
            NBTObject::add_string(tagname, value);
        }
    }
  private:
    uint8_t Count;
    uint16_t Damage;
    std::string name;
};



class EntityType : public NBTObject {
  public:
    std::string get_type() { return type; };
    long get_unique_id() { return unique_id; };
    std::string get_name() { return name; };

    EquipmentType &get_armor(int id)  {
        assert(sizeof(armor)/sizeof(armor[0]) > (long unsigned int) id);
        return armor[id];
    };

    Attribute &get_attr(int id) {
        if ((long unsigned int)id == attrs.size()) {
            attrs.push_back(Attribute());
            return attrs.back();
        } else {
            return attrs[id];
        }
    };

    RotationType &get_rotation() { return rotation; }
    PositionType &get_position() { return position; }
    MotionType   &get_motion()   { return motion; }

    EquipmentType &get_main_hand() { return main_hand; }
    EquipmentType &get_off_hand()  { return off_hand;  }
    EquipmentType &get_item_in_hand()  { return item_in_hand;  }

    DefinitionsType &get_definitions() { return definitions; }

    void add_string(std::string tagname, std::string value) override {
        if (tagname == "identifier") {
            type = value;
        } else if ((tagname == "name") || (tagname == "Name")) {
            name = value;
        } else {
            NBTObject::add_string(tagname, value);
        }
    };

    void add_long(std::string tagname, long value) override {
        if (tagname == "UniqueID") {
            unique_id = value;
        } else {
            NBTObject::add_long(tagname, value);
        }
    }

    // these are in cpp because I filter messages for a long list of known property names.
    void add_byte(std::string tagname, uint8_t value) override;
    void add_short(std::string tagname, int16_t value) override;
    void add_int(std::string tagname, int32_t value) override;
  private:

    EquipmentType armor[4];
    std::vector<Attribute> attrs;
    RotationType  rotation;
    PositionType  position;
    MotionType    motion;
    EquipmentType main_hand;
    EquipmentType item_in_hand;
    EquipmentType off_hand;
    DefinitionsType definitions;
    std::string type;
    std::string name;
    long        unique_id;

};


class BlockEntityType : public NBTObject {
  public:
    std::string get_string() const override {
        std::stringstream ss;

        ss << "blockentity at (" << x << "," << y << "," << z << ") of type: " << type << " moveable " << moveable;
        ss << NBTObject::get_string();
        return ss.str();
    }


    std::string get_type() { return type; };

    std::string get_position_string() {
        std::stringstream ss;
        ss << x << "," << y << "," << z;
        return ss.str();
    }
    std::vector<int> get_position() {
        return {x,y,z};
    };

    void add_int(std::string tagname, int32_t value) override {
        if (tagname == "x") {
            x = value;
        } else if (tagname == "y") {
            y = value;
        } else if (tagname == "z") {
            z = value;
        } else {
            NBTObject::add_int(tagname, value);
        }
    }

    void add_byte(std::string tagname, uint8_t value) override {
        if (tagname == "isMovable") {
            moveable = value;
        } else {
            NBTObject::add_byte(tagname, value);
        }
    }

    void add_string(std::string tagname, std::string value) override {
        if (tagname == "id") {
            type = value;
        } else {
            NBTObject::add_string(tagname, value);
        }
    };

  private:
    int x,y,z;
    bool moveable;
    std::string type;
};

class VillageType : public NBTObject {
public:
    struct BBox {
        int xl, yl, zl, xh, yh, zh;
    };

    std::string get_string() const override {
        std::stringstream ss;

        ss << "village at (";
        ss << bbox.xl << "," << bbox.yl << "," << bbox.zl << ")(";
        ss << bbox.xh << "," << bbox.yh << "," << bbox.zh << ") named: " << village_name;
        ss << " with " << dwellers.size() << " dwellers";
        ss << " " << NBTObject::get_string();
        return ss.str();
    }


    VillageType(std::string name) : village_name(name) { /* empty */ };

    std::string get_name() { return village_name; }

    BBox get_bbox() { return bbox; };
    std::vector<long> get_dwellers() { return dwellers; }

    void add_long(std::string tagname, long value) override {
        if (tagname == "ID") {
            dwellers.push_back(value);
        } else {
            NBTObject::add_long(tagname, value);
        }
    }

    void add_int(std::string tagname, int32_t value) override {
        if (tagname == "X0") {
            bbox.xl = value;
        } else if (tagname == "Y0") {
            bbox.yl = value;
        } else if (tagname == "Z0") {
            bbox.zl = value;
        } else if (tagname == "X1") {
            bbox.xh = value;
        } else if (tagname == "Y1") {
            bbox.yh = value;
        } else if (tagname == "Z1") {
            bbox.zh = value;
        } else {
            NBTObject::add_int(tagname, value);
        }
}

private:
    std::vector<long> dwellers;

    std::string village_name;
    BBox bbox;
};
