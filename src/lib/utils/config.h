#pragma once

#include <memory>
#include <variant>
#include <string>
#include <map>

class Config {
public:


    struct SectionData;

    class Section : public std::shared_ptr<const SectionData> {
    public:
        using std::shared_ptr<const SectionData>::shared_ptr;

        std::optional<std::string_view> get_string(std::string_view key) const;
        Section get_subsection(std::string_view key) const;

    };

    using Value = std::variant<std::string,  Section>;


    struct SectionData {
        std::string name;
        std::shared_ptr<const SectionData> _parent;
        std::map<std::string, Value, std::less<> > _kv_map;
        std::vector<std::weak_ptr<const SectionData> > _references;

    };


    Section find_section(std::string_view name) const {
        auto iter = _root._kv_map.find(name);
        if (iter != _root._kv_map.end()) {
            return std::get<Section>(iter->second);
        } else {
            return std::make_shared<SectionData>(SectionData{std::string(name),{},{},{}});
        }
    }

    static bool set_inheritence(Section base, Section derived) {
        if (derived->_parent) return false;
        auto d = std::const_pointer_cast<SectionData>(derived);
        auto b = std::const_pointer_cast<SectionData>(base);
        b->_references.push_back(d);
        d->_parent = b;
        return true;
    }


protected:

    SectionData _root;


};
