#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <assert.h>
#include "mem.h"

namespace mem {

byte const *
memory::find(addr a) const
{
    mmap::const_iterator end = m_.end();
    for (mmap::const_iterator i = m_.begin(); i != end; ++i) {
        addr ba = i->first;
        block const &blk = i->second;
        int blksize = blk.size();

        if (a >= ba && a < ba + blksize) {
            return &blk[a - ba];
        }
    }

    return 0;
}

byte&
memory::insert(addr a, byte v)
{
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
memory::operator[](addr a) const
{
    byte const *b = find(a);
    if (!b) 
        throw std::out_of_range("memory");
    return *b;
}

byte& 
memory::operator[](addr a)
{
    return insert(a, byte());
}

void
memory::canonize()
{
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
memory::includes(addr a) const
{
    return find(a);
}

bool
memory::contiguous() const
{
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
memory::print(std::ostream& os, bool verbose) const
{
    os << "memory: nblocks=" << std::dec << m_.size();
    if (max() < min())
        os << " empty";
    else
        os << std::hex << " min=" << min() << " max=" << max();
   
    os << " crc16=";

    if (contiguous())
        os << crc16(*this);
    else
        os << "n/a" ;

    if (verbose) {
        os << std::endl;
        mmap::const_iterator const end = m_.end();
        for (mmap::const_iterator i = m_.begin(); i != end; ++i) {
            addr ba = i->first;
            size_t blksize = i->second.size();

            os << std::hex << "[" << ba << "," << ba + blksize << ") len=" << blksize << std::endl;
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

bool operator==(memory const& m1, memory const& m2)
{
    if (m1.min() != m2.min())
        return false;
    if (m1.max() != m2.max())
        return false;
    for (addr a = m1.min(); a < m1.max(); a++)
        if (m1[a] != m2[a])
            return false;
    return true;
}

memory
fill(memory const& m, byte v)
{
    return fill(m, range(m.min(), m.max()), v);
}

memory
fill(memory const& m, range r, byte v)
{
    memory nm = m;

    for (addr a = r.min(); a < r.max(); ++a) {
        if (!m.includes(a)) {
            nm.insert(a, v);
        }
    }

    nm.canonize();
    return nm;
}

memory
crop(memory const& m, range r)
{
    memory nm;

    for (addr a = r.min(); a < r.max(); ++a) {
        if (m.includes(a)) {
            nm.insert(a, m[a]);
        }
    }

    nm.canonize();
    return nm;
}

memory
offset(memory const& m, int off)
{
    memory nm;

    memory::const_iterator end = m.end();
    for (memory::const_iterator i = m.begin(); i != end; ++i) {
        nm[i->first + off] = i->second;
    }

    return nm;
}

}
