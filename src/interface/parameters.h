#pragma once
#include <optional>
#include <string_view>

namespace quarkbot {


template<typename T>
struct parameter_parse ;

template<>
struct parameter_parse<std::string_view> {
    std::string_view operator()(std::string_view str) const  {
        return str;
    }
};


template<>
struct parameter_parse<std::string> {
    std::string operator()(std::string_view str) const  {
        return std::string(str);
    }
};

template<>
struct parameter_parse<bool> {
    bool operator()(std::string_view str) const  {
        if (str == "true") return true;
        if (str == "false") return false;
        throw std::runtime_error("Required boolean type (true or false)");
    }
};

template<typename T>
requires(requires(std::string_view sv) { {T::from_string(sv)}->std::same_as<T>;})
struct parameter_parse<T> {
    T operator()(std::string_view sv) const {
        return T::from_string(sv);
    }
};

template<typename T>
requires(std::is_arithmetic_v<T>)
struct parameter_parse<T> {
    T operator()(std::string_view str) const {
        T value;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
        if (ec != std::errc()) {
            throw std::runtime_error("Required numeric type (failed to parse number)");
        }
        return value;
    }
};



}
