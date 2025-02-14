#pragma once
#include <optional>
#include <string>
#include <string_view>
#include "coroutines.h"

#include <memory>
namespace quarkbot {

///Abstract key-value database
class IDatabase {
public:


    class IBatch {
    public:
        virtual ~IBatch() = default;
        virtual void set_key(std::string_view key, std::string_view value) = 0;
        virtual void erase_key(std::string_view key) = 0;
        virtual void clear() = 0;
    };

    virtual std::unique_ptr<IBatch> new_batch() const = 0;
    virtual void commit_batch(std::unique_ptr<IBatch> &batch) = 0;
    virtual std::optional<std::string> get_value(std::string_view key) const = 0;
    virtual async_generator<KeyValue> iterate(std::string_view from_key, bool backward, unsigned int skip_prefix) const = 0;
    virtual async_generator<KeyValue> range(std::string_view from_key, std::string_view to_key, unsigned int skip_prefix) const = 0;
    virtual ~IDatabase() = default;
};

class Database {
public:

    Database(std::shared_ptr<IDatabase> ptr):_ptr(ptr) {}


    class Batch {
    public:
        Batch(std::unique_ptr<IDatabase::IBatch> ptr):_ptr(std::move(ptr)) {}
        void set_key(std::string_view key, std::string_view value) {
            _ptr->set_key(key, value);
        }
        void erase_key(std::string_view key) {
            _ptr->erase_key(key);
        }
        void clear() {
            _ptr->clear();
        }

    protected:
        std::unique_ptr<IDatabase::IBatch> _ptr;
        friend class Database;
    };

    Batch new_batch() const {
        return Batch(_ptr->new_batch());
    }
    void commit_batch(Batch &b) {
        _ptr->commit_batch(b._ptr);
    }
    std::optional<std::string> get_value(std::string_view key) const {
        return _ptr->get_value(key);
    }
    async_generator<KeyValue> iterate(std::string_view from_key, bool backward, unsigned int skip_prefix) const {
        return _ptr->iterate(from_key, backward, skip_prefix);
    }
    async_generator<KeyValue> range(std::string_view from_key, std::string_view to_key, unsigned int skip_prefix) const {
        return _ptr->range(from_key, to_key, skip_prefix);
    }

    template<typename T, std::output_iterator<char> Iter>
    requires(std::is_unsigned_v<T>)
    static Iter append_number(T v, Iter iter) {
        for (int shift = sizeof(T); shift--; ) {
            *iter=static_cast<char>((v>>(8*shift)) & 0xFF);
            ++iter;
        }
        return iter;
    }

protected:
    std::shared_ptr<IDatabase> _ptr;
};




}
