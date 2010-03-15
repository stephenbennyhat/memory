#ifndef RANGE_H
#define RANGE_H

#include <iostream>
#include <ios>

namespace mem {
    template <typename T, typename I=size_t>
    class range_base
    {
    public:
       range_base() : begin_(T(0)), end_(T(0)) {}
       range_base(T begin, T end) : begin_(begin), end_(end) {}
       T begin() const { return begin_;}
       T end() const { return end_;}
       void extend(T a) {
           if (empty()) { begin_ = a; end_ = a + 1;}
           if (a > end_ + 1) end_ = a + 1;
           if (a < begin_) begin_ = a;
       }
       bool empty() {
           return begin() == end();
       }
       size_t size() const {
           return end_ - begin_;
       }
       typedef I iterator;
       typedef I const_iterator;
    private:
       T begin_;
       T end_;
    };

    template <typename T>
    std::ostream& operator<<(std::ostream& os, range_base<T> const& r) {
        return os << std::hex << std::showbase
                  << '[' << r.begin() << ".." << r.end() << ')';
    }

    template <typename T>
    bool operator==(const range_base<T>& r1, const range_base<T>& r2) {
        return r1.begin() == r2.begin() && r1.end() == r2.end();
    }

    template <typename T>
    bool operator!=(const range_base<T>& r1, const range_base<T>& r2) {
        return !(r1 == r2);
    }
}

#endif
