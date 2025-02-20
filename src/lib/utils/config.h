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
        mutable std::vector<std::weak_ptr<const SectionData> > _references;

    };


    Section open(std::string_view name) const {
        auto iter = _root._kv_map.find(name);
        if (iter != _root._kv_map.end()) {
            return std::get<Section>(iter->second);
        } else {
            return std::make_shared<SectionData>(SectionData{std::string(name),{},{},{}});
        }
    }

    static std::vector<Section> get_derived_sections(Section sect) {
        std::vector<Section> ret;
        if (sect) {
            ret.reserve(sect->_references.size());
            for (const auto &c: sect->_references) {
                Section s = c.lock();
                if (s) ret.push_back(s);
            }
        }
        return ret;
    }



    static inline std::string_view trim(std::string_view str) {
        const size_t first = str.find_first_not_of(" \t");
        if (first == std::string_view::npos) return {};
        const size_t last = str.find_last_not_of(" \t");
        return str.substr(first, last - first + 1);
    }

    template<std::invocable<std::string &> LineSource,
             std::invocable<std::string_view, std::string_view, std::string_view> Callback >
    static void parseIniStream(LineSource &&input, Callback &&callback) {
        std::string line;
        std::string currentSection;

        static_assert(std::is_invocable_r_v<bool, LineSource, std::string &>,
                "Source must return true - valid line, false - eof");

        while (input(line)) {
            std::string_view line_view = trim(line);

            // Ignoruj prázdné řádky nebo komentáře
            if (line_view.empty() || line_view.front() == '#') continue;

            // Detekce sekce [sekce]
            if (line_view.front() == '[' && line_view.back() == ']') {
                currentSection = std::string(trim(line_view.substr(1, line_view.size() - 2)));
            } else {
                // Zpracuj klíč = hodnota
                size_t eqPos = line_view.find('=');
                if (eqPos != std::string_view::npos) {
                    std::string_view key_view = trim(line_view.substr(0, eqPos));
                    std::string_view value_view = trim(line_view.substr(eqPos + 1));
                    callback(currentSection, key_view, value_view);
                }
            }
        }
    }

    template<std::invocable<std::string &> LineSource>
    void parse(LineSource &&input) {
        std::map<std::string, std::shared_ptr<SectionData>, std::less<> > tmp_map;

    }


protected:

    SectionData _root;


};
