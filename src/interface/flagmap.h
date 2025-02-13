#pragma once
#include <bitset>

namespace quarkbot {

template<typename Enum, typename MapType = std::bitset<sizeof(int)*8> >
class FlagMap {
public:

    constexpr FlagMap() = default;
    constexpr FlagMap(Enum enm) {_map.set(1<<static_cast<int>(enm));}
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
        _map.set(1<<static_cast<int>(f));
    }
    void reset(Enum f) {
        _map.reset(1<<static_cast<int>(f));
    }


public:
    MapType _map;
};

}