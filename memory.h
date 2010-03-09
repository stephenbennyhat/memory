#ifndef MEMORY_H
#define MEMORY_H

#include <map>
#include <vector>
#include <iostream> 

typedef unsigned char byte;
typedef unsigned long addr;
typedef unsigned short uint16;

class memory {
     typedef std::vector<byte> block;
     typedef std::map<addr,block> mmap;
     
     mmap m_;
     addr min_;
     addr max_;

     byte const *find(addr a) const;

public:

     memory() : min_(addr(-1)), max_(addr(0)) {}
     void add(addr a, byte b);
     byte operator[](addr a) const;
     void canonize();
     bool contiguous() const;
     bool includes(addr a) const;

     void print(std::ostream& os) const;

     addr min() const { return min_; };
     addr max() const { return max_; };

     friend memory offset(memory const &m, int off);
};

inline
std::ostream& operator<<(std::ostream& os, memory const& m) {
     m.print(os);
     return os;
}

bool operator==(memory const& m1, memory const& m2);

class range
{
   addr min_;
   addr max_;
public:
   range(addr min, addr max) : min_(min), max_(max) {}
   addr min() const { return min_;}
   addr max() const { return max_;}
};

memory fill(memory const& m, range r, byte v = 0xFF);
memory fill(memory const& m, byte v = 0xFF);
memory crop(memory const& m, range r);
memory offset(memory const &m, int off);

#endif
