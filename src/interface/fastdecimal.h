#pragma once

#include <bit>
#include <limits>
#include <iomanip>
#include <cstdint>
#include <string>
#include <iostream>
#include <sstream>
#include <format>

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

    constexpr operator double() const {return _value;}

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
    std::string to_string() {
        std::ostringstream s;
        s << *this;
        return s.str();
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

    static constexpr double decode_value(double value) {
        return encode(value, 0);
    }


    static constexpr double round(double value, unsigned int decimal_count) {
        constexpr _details::Pow10Table<max_decimal_count> pow10table = {};
        double tmp = value *= pow10table.mult_table[decimal_count];
        if (tmp < 0) {
            std::uint64_t v;
            tmp = -tmp;
            if (tmp > static_cast<double>(std::numeric_limits<std::uint64_t>::max())) return value;
            v = static_cast<std::uint64_t>(tmp + 0.5);
            return -(v * pow10table.div_table[decimal_count]);
        } else {
            std::uint64_t v;
            if (tmp > static_cast<double>(std::numeric_limits<std::uint64_t>::max())) return value;
            v = static_cast<std::uint64_t>(tmp + 0.5);
            return v * pow10table.div_table[decimal_count];
        }
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

