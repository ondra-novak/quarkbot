#pragma once

#include <memory>

namespace quarkbot {
    
template<typename T>
class IIterator {
public:
    //start iteration (restart)
    virtual void begin() = 0;
    //iteration has been stoped prematurely
    virtual void stop() = 0;
    virtual IIterator *fetch_next_to(T *uninitalize_variable) = 0;
};

template<typename T>
class Iterator {
public:
    Iterator(std::unique_ptr<IIterator<T> > iter_impl)
        :_iter_impl(std::move(iter_impl)) {}


    class iterator {
        public:
    
            iterator(IIterator<T> *owner):_owner(owner->fetch_next_to(&_value)) {}
            iterator():_owner(nullptr) {}
            ~iterator() {
                if (_owner) {
                    _owner->stop();
                    std::destroy_at(&_value);
                }
            }
            iterator(const iterator &other):_owner(other._owner) {
                if (_owner) std::construct_at(&_value, other._value);
            }
            iterator(iterator &&other):_owner(other._owner) {
                if (_owner) {
                    std::construct_at(&_value, std::move(other._value));
                    other._owner = nullptr;
                    std::destroy_at(&other._value);
                }
            }
            iterator &operator=(const iterator &other) {
                if (this != &other) {
                    std::destroy_at(this);
                    std::construct_at(this, other);
                }
                return *this;
            }
            iterator &operator=(iterator &&other) {
                if (this != &other) {
                    std::destroy_at(this);
                    std::construct_at(this, std::move(other));
                }
                return *this;
            }
            const T &operator *() const {return _value;}
            const T *operator->() const {return &_value;}
            iterator &operator++() {
                if (_owner) {
                    std::destroy_at(&_value);
                    auto v = std::exchange(_owner, nullptr);
                    _owner = v->fetch_next_to(&_value);
                }
                return *this;
            }
            iterator operator++(int) {
                iterator c(*this);
                this->operator++();
                return c;
            }
            bool operator==(const iterator &other) const {
                return _owner  == other._owner;
            }
    
        protected:
            IIterator<T> *_owner = nullptr;
            union {
                T _value;
            };
        };

    iterator begin() {
        _iter_impl->begin();
        return iterator(_iter_impl.get());
    }

    iterator end() const {return {};}

protected:
    std::unique_ptr<IIterator<T> > _iter_impl;
};



}