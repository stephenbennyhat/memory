#ifndef MEMORY_H
#define MEMORY_H

#include <map>
#include <vector>
#include <iostream> 
#include <iterator> 

namespace mem {

typedef unsigned char byte;
typedef unsigned long addr;
typedef unsigned short uint16;

class range
{
   addr min_;
   addr max_;
public:
   range() : min_(addr(-1)), max_(addr(0)) {}
   range(addr min, addr max) : min_(min), max_(max) {}
   addr min() const { return min_;}
   addr max() const { return max_;}
   void extend(addr a) {
       if (min_ > a) min_ = a;
       if (max_ < a) max_ = a;
   }
   size_t size() const { return max_ - min_; }
};

class memory {
     typedef std::vector<byte> block;
     typedef std::map<addr,block> mmap;
     
     mmap m_;
     range r_;

     byte const *find(addr a) const;

public:
     byte& insert(addr a, byte b = byte());
     byte  operator[](addr a) const;
     byte& operator[](addr a);
     void canonize();
     bool contiguous() const;
     bool includes(addr a) const;

     void print(std::ostream& os) const;

     addr min() const { return r_.min(); };
     addr max() const { return r_.max(); };

     friend memory offset(memory const &m, int off);

     class memory_const_iterator :
             public std::iterator<std::input_iterator_tag,
                                  byte,
                                  size_t,
                                  byte const*,
                                  byte const&>
     {
         typedef std::pair<addr,byte> pab;
         memory const *m_;
         addr          a_;
         mutable pab   p_; // just so operator-> works.
     public:
         memory_const_iterator(memory const& m, addr a)
             : m_(&m), a_(a) {}
         memory_const_iterator(memory const& m)
             : m_(&m), a_(m.min()) {}
         memory_const_iterator() : m_(0), a_(0) {}

         pab *operator->() const {
             p_ = pab(a_, (*m_)[a_]);
             return &p_;
         }
         pab operator*() const {
             return p_ = pab(a_, (*m_)[a_]);
         }

         memory_const_iterator& operator++() {
             next();
             return *this;
         }
         memory_const_iterator operator++(int) {
             memory_const_iterator tmp = *this;
             next();
             return tmp;
         }

         friend bool operator==(memory_const_iterator i1, memory_const_iterator i2) {
            return i1.a_ == i2.a_;
         }
         friend bool operator!=(memory_const_iterator i1, memory_const_iterator i2) {
            return !(i1 == i2);
         }
     private:
         void next() {
             while (a_ != m_->max() + 1) {
                 a_++;
                 if (m_->includes(a_))
                     break;
             }
         }
     };

     typedef memory_const_iterator const_iterator;
     memory_const_iterator begin() const { return memory_const_iterator(*this, min()); }
     memory_const_iterator end()   const { return memory_const_iterator(*this, max() + 1); }
};

inline
std::ostream& operator<<(std::ostream& os, memory const& m) {
     m.print(os);
     return os;
}

bool operator==(memory const& m1, memory const& m2);

memory fill(memory const& m, range r, byte v = 0xFF);
memory fill(memory const& m, byte v = 0xFF);
memory crop(memory const& m, range r);
memory offset(memory const &m, int off);


void writemoto(std::ostream& os, memory const& mem, int addrlen=4, int maxline=16);
void readmoto(std::istream& is, memory& mem);
void readmoto(std::string filename, memory& mem);

uint16 crc16(memory const& m, uint16 init = 0xFFFF);

};

#endif

