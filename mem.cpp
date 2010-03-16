#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <assert.h>
#include "mem.h"

namespace mem {
    memory::memory_const_iterator::memory_const_iterator(mem::memory const& m)
            : end_(m.m_.end()),
              bi_(m.m_.begin()),
              off_(0) {
    }

    memory::memory_const_iterator::memory_const_iterator(mem::memory const& m, addr a)
           : end_(m.m_.end()),
             bi_(m.findblock(a)),
             off_(bi_ == end_ ? 0 : a - bi_->first) {
    }

    memory::memory_const_iterator&
    memory::memory_const_iterator::next() {
        if (++off_ >= bi_->second.size())
        {
            ++bi_;
            off_ = 0;
        }
        return *this;
    }


    memory::mmap::const_iterator
    memory::findblock(addr a) const {
        mmap::const_iterator end = m_.end();
        for (mmap::const_iterator i = m_.begin(); i != end; ++i) {
            addr ba = i->first;
            block const &blk = i->second;
            int blksize = blk.size();
            if (a >= ba && a < ba + blksize) {
                return i;
            }
        }
        return end;
    }

    byte const *
    memory::find(addr a) const {
        mmap::const_iterator i = findblock(a);
        return (i == m_.end()) ? 0 : &i->second[a - i->first];
    }

    byte&
    memory::insert(addr a, byte v) {
        if (byte const *bp = find(a))
            return const_cast<byte&>(*bp);
        mmap::const_iterator end = m_.end();
        for (mmap::iterator i = m_.begin(); i != end; ++i) {
            addr ba = i->first;
            block &blk = i->second;
            int blksize = blk.size();
            if (a == ba + blksize) {
                blk.push_back(v);
                r_.extend(a);
                return blk.back();
            }
        }
        r_.extend(a);
        block blk;
        blk.push_back(byte());
        m_[a] = blk;
        return m_[a].back();
    }

    byte 
    memory::operator[](addr a) const {
        byte const *b = find(a);
        if (!b) 
            throw std::out_of_range("memory");
        return *b;
    }

    byte& 
    memory::operator[](addr a) {
        return insert(a, byte());
    }

    void
    memory::canonize() {
        if (contiguous())
            return;
        mmap::iterator const end = m_.end();
        for (mmap::iterator i = m_.begin(); i != end; ++i) {
            addr ba = i->first;
            block& blk = i->second;
            int blksize = blk.size();
            mmap::iterator i2 = m_.find(ba + blksize);
            if (i2 != m_.end()) {
                if (0) std::cout << "could join " << ba << " and " << i2->first << std::endl;
                for (size_t j = 0; j < i2->second.size(); j++) {
                    blk.push_back(i2->second[j]);
                }
                m_.erase(i2);
            }
        }
    }

    bool
    memory::includes(addr a) const {
        return find(a);
    }

    bool
    memory::contiguous() const {
        if (m_.size() < 2)
            return true;
        mmap::const_iterator end = m_.end();
        mmap::const_iterator i = m_.begin();
        addr next = i->first + i->second.size();
        while (++i != end) {
            if (i->first != next) {
                return false;
            }
            next = i->first + i->second.size();
        }
        return true;
    }

    void
    memory::print(std::ostream& os, bool verbose) const {
        os << "memory: nblocks=" << std::dec << m_.size() << " r=" << getrange();
        os << " crc16=";
        if (contiguous())
            os << std::hex << crc16(*this);
        else
            os << "n/a" ;
        if (!getdesc().empty()) {
            os << " desc=\"" << getdesc() << '"';
        }
        os << " addr=" << getexecaddr();
        if (verbose) {
            os << std::endl;
            mmap::const_iterator const end = m_.end();
            for (mmap::const_iterator i = m_.begin(); i != end; ++i) {
                addr ba = i->first;
                size_t blksize = i->second.size();
                os << std::hex << "[" << ba << "," << ba + blksize << ") len="
                   << blksize << std::endl;
                os << ba << ":\t";
                for (size_t j = 0; j < blksize; j++) {
                    if (j == 0 || j % 16) os << " "; else os << std::endl << "\t";
                    if (j > 16 * 4) {
                        os << "...";
                        break;
                    }
                    os << int(i->second[j]);
                }
                os << std::endl;
            }
        }
    }

    bool operator==(memory const& m1, memory const& m2) {
        if (m1.getrange() != m2.getrange())
            return false;
        for (addr a = m1.getrange().begin(); a < m1.getrange().end(); a++)
            if (m1[a] != m2[a])
                return false;
        return true;
    }

}
