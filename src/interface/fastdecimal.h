#pragma once

#include <bit>
#include <limits>
#include <iomanip>
#include <cstdint>
#include <format>
#include <charconv>
#include <vector>

namespace _details {

template<unsigned int max_decimal_count>
struct Pow10Table {
    double div_table[max_decimal_count+1] = {};
    double mult_table[max_decimal_count+1] = {};
    constexpr Pow10Table() {
        double m= 1.0;
        double d = 1.0;
        for (unsigned int i = 0; i < max_decimal_count; ++i) {
            div_table[i] = d;
            mult_table[i] = m;
            d /= 10;
            m *= 10;
        }
    }
};

}

class FastDecimal {
public:

    static constexpr unsigned int decimal_bits = 4;
    static constexpr unsigned int max_decimal_count = (1<<decimal_bits) - 1;

    constexpr FastDecimal(double value, unsigned int decimal_count)
        :_value(encode(value, decimal_count)) {}
    constexpr FastDecimal(double value, FastDecimal template_number)
        :_value(encode(value, template_number.get_decimal_count())) {}
    constexpr FastDecimal() = default;

    constexpr operator double() const {return get_rounded();}

    constexpr double get_rounded() const {return round(_value, decode_decimal_count(_value));}

    friend constexpr FastDecimal operator+(FastDecimal first, FastDecimal second)  {
        return {first._value+second._value, std::max(first.get_decimal_count(),second.get_decimal_count())};
    }
    friend constexpr FastDecimal operator-(FastDecimal first, FastDecimal second)  {
        return {first._value-second._value, std::max(first.get_decimal_count(),second.get_decimal_count())};
    }
    friend constexpr FastDecimal operator-(FastDecimal first)  {
        return {-first._value, first.get_decimal_count()};
    }
    friend constexpr FastDecimal operator*(FastDecimal first, FastDecimal second)  {
        return {first._value*second._value, std::max(first.get_decimal_count(),second.get_decimal_count())};
    }
    friend constexpr FastDecimal operator/(FastDecimal first, FastDecimal second)  {
        return {first._value/second._value, std::max(first.get_decimal_count(),second.get_decimal_count())};
    }
    friend constexpr FastDecimal operator+(FastDecimal first, double second)  {
        return {first._value+second, first.get_decimal_count()};
    }
    friend constexpr FastDecimal operator-(FastDecimal first, double second)  {
        return {first._value-second, first.get_decimal_count()};
    }
    friend constexpr FastDecimal operator*(FastDecimal first, double second)  {
        return {first._value*second, first.get_decimal_count()};
    }
    friend constexpr FastDecimal operator/(FastDecimal first, double second)  {
        return {first._value/second, first.get_decimal_count()};
    }
    friend constexpr FastDecimal operator+(double first, FastDecimal second)  {
        return {first+second._value, second.get_decimal_count()};
    }
    friend constexpr FastDecimal operator-(double first, FastDecimal second)  {
        return {first-second._value, second.get_decimal_count()};
    }
    friend constexpr FastDecimal operator*(double first, FastDecimal second)  {
        return {first*second._value, second.get_decimal_count()};
    }
    friend constexpr FastDecimal operator/(double first, FastDecimal second)  {
        return {first/second._value, second.get_decimal_count()};
    }
    constexpr FastDecimal &operator+=(double second)  {
        _value = encode(_value + second, get_decimal_count());
        return *this;
    }
    constexpr FastDecimal &operator-=(double second)  {
        _value = encode(_value - second, get_decimal_count());
        return *this;
    }
    constexpr FastDecimal &operator*=(double second)  {
        _value = encode(_value * second, get_decimal_count());
        return *this;
    }
    constexpr FastDecimal &operator/=(double second)  {
        _value = encode(_value / second, get_decimal_count());
        return *this;
    }

    constexpr unsigned int get_decimal_count() const {
        return decode_decimal_count(_value);
    }



    friend std::ostream &operator << (std::ostream &out, const FastDecimal &d) {
        out <<std::fixed << std::setprecision(decode_decimal_count(d._value)) << d._value;
        return out;
    }

    static constexpr double encode(double value, unsigned int decimal_count) {
        return std::bit_cast<double>(
                (std::bit_cast<std::uint64_t>(value) & ~std::uint64_t(max_decimal_count))
                    | (decimal_count & max_decimal_count));
    }
    static constexpr unsigned int decode_decimal_count(double value) {
        return std::bit_cast<std::uint64_t>(value) & max_decimal_count;

   }

   std::to_chars_result to_chars( char* first, char* last ) const {
        return std::to_chars(first, last, _value, std::chars_format::fixed, get_decimal_count());
   }
   
   std::string_view to_string(std::vector<char> &buffer) const {
        int count = 20;
        while (true) {
            buffer.clear();
            buffer.resize(count);
            auto result = to_chars(buffer.data(), buffer.data()+buffer.size());
            if (result.ec == std::errc::value_too_large) {
                count +=20;                
            } else {
                buffer.resize(result.ptr -buffer.data());
                return {buffer.data(), buffer.size()};
            }
        }
   }

   template<typename X>
   requires(std::is_constructible_v<X, const char *, std::size_t> && (requires(X v) {{v.c_str()}->std::same_as<const char *>;}))
   operator X() const {
        char buffer[128];
        auto result = to_chars(buffer, buffer+sizeof(buffer));
        if (result.ec == std::errc::value_too_large) {
            std::vector<char> dynbuff;
            to_string(dynbuff);
            return X(dynbuff.data(), dynbuff.size());
        } else {
            return X(buffer, result.ptr - buffer);
        }
   }

    template<typename Stream>
    friend Stream &operator<<(Stream &stream, FastDecimal me) {
        char buffer[128];
        auto result = to_chars(buffer, buffer+sizeof(buffer));
        std::string_view res;
        if (result.ec == std::errc::value_too_large) {
            std::vector<char> dynbuff;
            res = to_string(dynbuff);            
        } else {
            res = {buffer,result.ptr - buffer};
        }
        stream << res;
        return stream;
    }

    static constexpr double decode_value(double value) {
        return encode(value, 0);
    }

    static constexpr double constexpr_round(double x) noexcept {

        if (!std::is_constant_evaluated()) {
            return std::round(x);
        } else {
            constexpr double max_val = 
                static_cast<double>(std::numeric_limits<std::uint64_t>::max());
            constexpr double min_val = 
                -static_cast<double>(std::numeric_limits<std::uint64_t>::max());
            if (x < 1 && x > -1) [[unlikely]] return 0;
            if (x >= max_val || x <= min_val) [[unlikely]] return x;
            return x >= 0.0
                ? static_cast<double>(static_cast<std::uint64_t>(x + 0.5))
                : -static_cast<double>(static_cast<std::uint64_t>(-x + 0.5));
        }
        
    }


    static constexpr _details::Pow10Table<max_decimal_count> pow10table = {};

    static constexpr double round(double value, unsigned int decimal_count) {
        if (decimal_count >= max_decimal_count) return encode(value,0);
        return constexpr_round(pow10table.mult_table[decimal_count]*value)
                    * pow10table.div_table[decimal_count];
    }

protected:
    double _value = 0;
};


template <>
struct std::formatter<FastDecimal> : std::formatter<double> {
      auto format(const FastDecimal& id, std::format_context& ctx) const {
        return std::formatter<double>::format(id.get_rounded(), ctx);
    }
};

