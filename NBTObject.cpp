

#include <NBTObject.h>

#include <sstream>
std::string NBTObject::indent;

// this class is just to force printing some memory stats
class NBTObjectProperties {
public:
    NBTObjectProperties() {
        std::cout << "memory properties for NBTObjects\n";
        std::cout << "NBTObject " << sizeof(NBTObject) << "\n";
        std::cout << "Equipment " << sizeof(EquipmentType) << "\n";
        std::cout << "TupleType " << sizeof(TupleType) << "\n";
        std::cout << "EntityType " << sizeof(EntityType) << "\n";

    }
};

static NBTObjectProperties justadummyvar;

std::string NBTObject::get_string() const
{
    std::stringstream ss;
    ss << "(";

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
    for(auto elt : short_properties) {
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

std::string TupleType::get_string()
{
    std::stringstream ss;
    ss << " (";
    bool first = true;
    for(auto value : values) {
        if (!first) { ss << ", "; }
        first = false;
        ss << value << std::dec;
    }
    ss << ")";
    return ss.str();
}

static std::set<std::string> known_byte_properties = {
    "Chested",
    "Color",
    "Color2",
    "Dead",
    "Invulnerable",
    "IsAngry",
    "IsAutonomous",
    "IsBaby",
    "IsEating",
    "IsGliding",
    "IsGlobal",
    "IsIllagerCaptain",
    "IsOrphaned",
    "IsPregnant",
    "IsRoaring",
    "IsScared",
    "IsStunned",
    "IsSwimming",
    "IsTamed",
    "IsTrusting",
    "LootDropped",
    "NaturalSpawn",
    "OnGround",
    "Persistent",
    "RewardPlayersOnFirstFounding",
    "Saddled",
    "Sheared",
    "ShowBottom",
    "Sitting",
    "Surface",
    "hasBoundOrigin",
};

static std::set<std::string> known_short_properties = {
    "Air",
    "AttackTime",
    "DeathTime",
    "Fire",
    "HurtTime",
};

static std::set<std::string> known_int_properties = {
  "LastDimensionId",
  "MarkVariant",
  "PortalCooldown",
  "SkinID",
  "Strength",
  "StrengthMax",
  "TradeExperience",
  "TradeTier",
  "Variant",
  "boundX",
  "boundY",
  "boundZ",
  "limitedLife",
};


void EntityType::add_byte(std::string tagname, uint8_t value)
{

    if (known_byte_properties.find(tagname) != known_byte_properties.end()) {
        byte_properties[tagname] = value;
    } else {
        NBTObject::add_byte(tagname, value);
    }
}

void EntityType::add_short(std::string tagname, int16_t value)
{
    if (known_short_properties.find(tagname) != known_short_properties.end()) {
        short_properties[tagname] = value;
    } else {
        NBTObject::add_short(tagname, value);
    }
}

void EntityType::add_int(std::string tagname, int32_t value)
{
    if (known_int_properties.find(tagname) != known_int_properties.end()) {
        int_properties[tagname] = value;
    } else {
        NBTObject::add_int(tagname, value);
    }
}
