#pragma once
#include <bitset>

namespace quarkbot {

template<typename Enum, typename MapType = std::bitset<sizeof(int)*8> >
class FlagMap : public MapType{
public:

    constexpr FlagMap() = default;
    constexpr FlagMap(Enum enm) { set(enm);}
    constexpr FlagMap(MapType map):MapType(map) {}
    constexpr friend FlagMap operator | (const FlagMap &m, Enum f ) {
        FlagMap ret(m);
        ret.set(f);
        return ret;
    }
    constexpr friend FlagMap operator | (Enum f, const FlagMap &m ) {
        FlagMap ret(m);
        ret.set(f);
        return ret;
    }
    constexpr friend FlagMap operator | (Enum f, Enum g ) {
        FlagMap ret(g);
        ret.set(f);
        return ret;
    }
    constexpr friend FlagMap operator + (const FlagMap &m, Enum f ) {
        FlagMap ret(m);
        ret.set(f);
        return ret;
    }
    constexpr friend FlagMap operator + (Enum f, const FlagMap &m ) {
        FlagMap ret(m);
        ret.set(f);
        return ret;
    }
    constexpr friend FlagMap operator - (const FlagMap &m, Enum f ) {
        FlagMap ret(m);
        ret.reset(f);
        return ret;
    }
    void set(Enum f) {
        MapType::set(1<<static_cast<int>(f));
    }
    void reset(Enum f) {
        MapType::reset(1<<static_cast<int>(f));
    }
    bool test(Enum f) const {
        return MapType::test(1<<static_cast<int>(f));
    }


};

}
