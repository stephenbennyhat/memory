#ifndef MEMORY_H
#define MEMORY_H

#include <map>
#include <vector>
#include <iostream> 
#include <iterator> 
#include "memtypes.h"
#include "range.h"

namespace mem {
    typedef range_base<addr> range;

    class memory {
        typedef std::vector<byte> block;
        typedef std::map<addr,block> mmap;
     
        mmap m_;
        range r_;
        std::string desc_;
        addr execaddr_;

        byte const *find(addr a) const;
        mmap::const_iterator findblock(addr a) const;
    public:
        byte& insert(addr a, byte b = byte());
        byte  operator[](addr a) const;
        byte& operator[](addr a);
        void canonize();
        bool contiguous() const;
        bool includes(addr a) const;

        void print(std::ostream& os, bool verbose = false) const;

        range getrange() const { return r_; }
        std::string getdesc() const { return desc_; }
        void setdesc(std::string const& s)  { desc_ = s; }

        addr getexecaddr() const { return execaddr_; }
        void setexecaddr(addr a) { execaddr_ = a; }

        class memory_const_iterator
        {
            typedef size_t difference_type;
            typedef byte value_type;
            typedef byte const* pointer;
            typedef byte const& reference;
            typedef std::input_iterator_tag iterator_category;

            typedef std::pair<addr,byte> pab;

            mmap::const_iterator end_;
            mmap::const_iterator bi_;
            size_t off_;

            mutable pab p_; // just so operator-> works.
        public:
            memory_const_iterator(memory const& m, addr a);
            memory_const_iterator(memory const& m);
            memory_const_iterator();

            pab *operator->() const {
                addr a = bi_->first + off_;
                byte b = bi_->second[off_];
                return &(p_ = pab(a, b));
            }
            pab operator*() const { return *operator->(); }

            memory_const_iterator& operator++() {
                return next();
            }
            memory_const_iterator operator++(int) {
                memory_const_iterator tmp = *this;
                ++*this;
                return tmp;
            }

            friend bool operator==(memory_const_iterator i1, memory_const_iterator i2) {
                return i1.bi_ == i2.bi_ && i1.off_ == i2.off_;
            }
            friend bool operator!=(memory_const_iterator i1, memory_const_iterator i2) {
                return !(i1 == i2);
            }
        private:
            memory_const_iterator& next();
        };

        typedef memory_const_iterator const_iterator;
        memory_const_iterator begin() const {
            return memory_const_iterator(*this);
        }
        memory_const_iterator end() const {
            return memory_const_iterator(*this, getrange().end());
        }
    };

    inline
    std::ostream& operator<<(std::ostream& os, memory const& m) {
        m.print(os);
        return os;
    }

    bool operator==(memory const& m1, memory const& m2);
    inline
    bool operator!=(memory const& m1, memory const& m2) {
        return !(m1==m2);
    }

    memory fill(memory const& m, range r, byte v = 0xFF);
    memory fill(memory const& m, byte v = 0xFF);
    memory crop(memory const& m, range r);
    memory offset(memory const &m, int off);
    memory join(memory const &m, memory const& m2);

    void writemoto(std::ostream& os, memory const& mem, int addrlen=4, int maxline=16);
    void readmoto(std::istream& is, memory& mem, std::string const& filename="");
    void readmoto(std::string const& filename, memory& mem);

    uint16 crc16(memory const& m, uint16 init = 0xFFFF);
};
#endif

