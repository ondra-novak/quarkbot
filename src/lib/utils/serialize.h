#pragma once


#include <type_traits>
#include <string_view>
#include <string>
#include <stdexcept>



class SerializeUnexpectedEndOfData: public std::exception {
public:
    virtual const char *what() const noexcept override {
        return "Serialize read: unexpected end of data";
    }
};

class SerializeSyncError: public std::exception {
public:
    virtual const char *what() const noexcept override {
        return "Serialize read: synchronization error (unexpected data)";
    }
};


template<typename T>
class Serializable;


template<typename T>
std::string serialize_to_string(const T &data) {
    std::string s;
    Serializable<T>::output(data, std::back_inserter(s));
    return s;
}

template<typename T>
T deserialize_string(const std::string_view &str) {
    auto iter = str.begin();
    auto end = str.end();
    return Serializable<T>::output(iter,end);
}

struct any_serialized_value {
    std::string val;
    template<typename T>
    requires(requires(std::string_view v){{deserialize_string<T>(v)}->std::same_as<T>;})
    operator T() const {
        return deserialize_string<T>(val);
    }
};


template<typename T>
requires(std::is_trivially_copyable_v<T> && !std::is_unsigned_v<T>)
class Serializable<T> {
public:
    template<std::output_iterator<char> Iter>
    static Iter output(const T &val, Iter iter) {
        return std::copy(reinterpret_cast<const char *>(&val),
                reinterpret_cast<const char *>(&val) + sizeof(val),
                iter);

    }

    template<typename Iter>
    static T input(Iter &iter, Iter end) {
        T out;
        char *wr = reinterpret_cast<T *>(&out);
        for (std::size_t i = 0; i < sizeof(out); ++i) {
            if (iter == end) throw SerializeUnexpectedEndOfData();
            wr[i] = iter;
            ++iter;
        }
        return iter;
    }
};

template<typename T>
requires(std::is_unsigned_v<T>)
class Serializable<T> {

    static constexpr std::uint8_t max_inline = 255-8;

    template<std::output_iterator<char> Iter>
    static Iter output(const T &val, Iter iter) {
        if (val <= max_inline) {
            *iter = static_cast<char>(val);
            ++iter;
            return iter;
        }

        T tmp = val;
        std::uint8_t cnt = 0;
        while (val) {
            ++cnt;
            tmp >>= 8;
        }
        *iter = static_cast<char>(cnt+max_inline);
        tmp = val - max_inline;
        while (cnt) {
            --cnt;
            *iter = static_cast<char>(val >> (8*cnt));
            ++iter;
        }
        return iter;
    }

    template<typename Iter>
    static T input(Iter &iter, Iter end) {
        T out = {};
        if (iter == end) throw SerializeUnexpectedEndOfData();
        auto cnt = static_cast<std::uint8_t>(iter, end);
        ++iter;
        if (cnt <= max_inline) {
            out = static_cast<T>(cnt);
        } else {
            cnt -= max_inline;
            if (iter == end) throw SerializeUnexpectedEndOfData();
            for (std::uint8_t i = 0; i < cnt; ++i) {
                out = (out << 8) | static_cast<T>(static_cast<uint8_t>(*iter));
            }
            out += max_inline;
        }
        return out;
    }

};


template<>
class Serializable<std::string> {
public:
     template<std::output_iterator<char> Iter>
     static Iter output(const std::string &val, Iter iter) {
        return std::copy(val.begin(), val.end(),
                Serializable<std::size_t>::output(val.size(), iter));
    }
    template<typename Iter>
    static std::string input(Iter &iter, Iter end) {
        std::size_t sz = Serializable<std::size_t>::input(iter, end);
        std::string out;
        out.resize(sz);
        for (std::size_t i = 0; i < sz; ++i) {
            if (iter == end) throw SerializeUnexpectedEndOfData();
            out[i] = iter;
            ++iter;
        }
        return out;

    }
};


template<>
class Serializable<std::string_view>: public Serializable<std::string> {
public:
    template<std::output_iterator<char> Iter>
    static Iter output(const std::string_view &val, Iter iter) {
       return std::copy(val.begin(), val.end(),
               Serializable<std::size_t>::output(val.size(), iter));
   }
};

template<typename T>
requires(requires(T v){     //container
    std::begin(v);
    std::end(v);
    std::distance(std::begin(v), std::end(v));
    std::back_inserter(v);
})
class Serializable<T> {
public:

    using value_type = std::decay_t<decltype(*std::begin(std::declval<T>()))>;

    template<std::output_iterator<char> Iter>
    static Iter output(const T &val, Iter iter) {
        std::size_t sz = std::distance(std::begin(val), std::end(val));
        iter = Serializable<std::size_t>::output(sz, iter);
        for (const auto &v: val) {
            iter = Serializable<value_type>::output(v, iter);
        }
        return iter;
    }
    template<typename Iter>
    static T input(Iter &iter, Iter end) {
        std::size_t sz = Serializable<std::size_t>::input(iter, end);
        T out;
        auto binsert = std::back_inserter(out);
        for (std::size_t i = 0; i < sz; ++i) {
            *binsert = Serializable<value_type>::input(iter, end);
            ++iter;
            ++binsert;
        }
        return out;

    }
};



template<typename T>
requires(requires(T v){     //variant
  typename std::variant_size<T>::type;
  { std::variant_alternative<0, T>{} };
})
class Serializable<T> {
public:


    template<std::output_iterator<char> Iter>
    static Iter output(const T &val, Iter iter) {
        std::size_t idx = val.index();
        iter = Serializable<std::size_t>::output(idx, iter);
        return std::visit([&](const auto &val){
            using X = std::decay_t<decltype(val)>;
            return Serializable<X>::output(val, iter);
        });
    }

    template<std::size_t N, typename LoadFn>
    T create_variant(LoadFn &&load_fn, std::size_t idx) {
        if constexpr(N >= std::variant_size_v<T>) {
            throw SerializeSyncError();
        } else if (idx == N) {
            return load_fn(std::integral_constant<std::size_t, N>{});
        } else {
            return create_variant<N+1, LoadFn>(std::forward<LoadFn>(load_fn), idx);
        }
    }

    template<typename Iter>
    static T input(Iter &iter, Iter end) {
        std::size_t index = Serializable<std::size_t>::input(iter, end);
        return create_variant<0>([&](auto ic){
            return T(std::in_place_index<ic.value>,
                    Serializable<std::variant_alternative<ic.value, T> >::input(iter,end));
        }, index);
    }
};



template<typename T>
requires(requires(T v){     //tuple
  typename std::tuple_size<T>::type;
  { std::tuple_element_t<0, T>{} };
})
class Serializable<T> {
public:

    template<typename U, std::output_iterator<char> Iter>
    static void output_one(const U &val, Iter &iter) {
        iter = Serializable<U>::output(val,iter);
    }

    template<typename U, typename Iter>
    static void input_one( U &val, Iter &iter, Iter end) {
        val = Serializable<U>::input(iter, end);
    }

    template<std::output_iterator<char> Iter>
    static Iter output(const T &val, Iter iter) {

        std::apply([&](const auto & ... items){
            (output_one(items, iter),...);
        },val);
        return iter;
    }

    template<typename Iter>
    static T input(Iter &iter, Iter end) {
        T out;
        std::apply([&](auto & ... items){
            (input_one(items, iter, end),...);
        },out);
        return out;
    }
};

template<typename T>
requires(requires(T v){     //optional
  v.has_value();
  *v;
})
class Serializable<T> {
public:

    using value_type = std::decay_t<decltype(*std::declval<T>())>;

    template<std::output_iterator<char> Iter>
    static Iter output(const T &val, Iter iter) {
        iter = Serializable<bool>::output(val.has_value(), iter);
        if (val.has_value()) {
            iter = Serializable<value_type>::output(*val, iter);
        }
        return iter;
    }

    template<typename Iter>
    static std::string input(Iter &iter, Iter end) {
        T out;
        bool hv = Serializable<bool>::input(iter, end);
        if (hv) {
            out.emplace(Serializable<value_type>::input(iter, end));
        }
        return out;
    }
};


