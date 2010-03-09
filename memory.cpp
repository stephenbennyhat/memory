#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "memory.h"
#include "crc.h"

void
memory::add(addr a, byte b)
{
    if (0) std::cout << std::hex << "m[" << a << "]=" << int(b) << std::endl;

    bool found = false;

    mmap::iterator const end = m_.end();
    for (mmap::iterator i = m_.begin(); i != end; ++i) {
        addr ba = i->first;
        block &blk = i->second;
        int blksize = blk.size();

        if (a >= ba && a < ba + blksize) {
            std::cerr << "error: already in memory" << std::endl;
            return;
        }
        else if (a == ba + blksize) {
            blk.push_back(b);
            found = true;
            if (0) std::cout << "info: append m[" << a << "]=" << int(b) << std::endl;
        }
    }

    if (!found) {
        block blk;
        blk.push_back(b);
        m_[a] = blk;
        if (0) std::cout << "info: new block mem[" << a << "]=" << int(b) << std::endl;
    }

    if (min_ > a) min_ = a;
    if (max_ < a) max_ = a;
}

byte 
memory::operator[](addr a) const
{
    byte const *b = find(a);
    if (!b) 
        throw std::out_of_range("memory");
    return *b;
}

byte const *
memory::find(addr a) const
{
    mmap::const_iterator const end = m_.end();
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

            for (int j = 0; j < i2->second.size(); j++) {
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
memory::print(std::ostream& os) const
{
    os << "memory: nblocks=" << std::dec << m_.size() << std::hex
       << " min=" << min() << " max=" << max()
       << " crc16=" << crc16(*this) << std::endl;

    mmap::const_iterator const end = m_.end();
    for (mmap::const_iterator i = m_.begin(); i != end; ++i) {
        addr ba = i->first;
        int blksize = i->second.size();

        os << std::hex << "[" << ba << "," << ba + blksize << ") len=" << blksize << std::endl;
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
            nm.add(a, v);
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
        if (!m.includes(a)) {
            throw std::out_of_range("crop");
        }
        nm.add(a, m[a]);
    }

    nm.canonize();
    return nm;
}

memory
offset(memory const& m, int off)
{
    memory nm;

    memory::mmap::const_iterator end = m.m_.end();
    for (memory::mmap::const_iterator i = m.m_.begin(); i != end; ++i) {
        nm.m_[i->first + off] = i->second;
    }

    return nm;
}
