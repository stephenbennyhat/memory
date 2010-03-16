#include "mem.h"

namespace mem {
    memory
    fill(memory const& m, byte v) {
        return fill(m, m.getrange(), v);
    }

    memory
    fill(memory const& m, range r, byte v) {
        memory nm = m;
        for (addr a = r.begin(); a < r.end(); ++a) {
            if (!m.includes(a))
                nm[a] = v;
        }
        nm.canonize();
        return nm;
    }

    memory
    crop(memory const& m, range r) {
        memory nm;
        for (memory::const_iterator i = m.begin(); i != m.end(); ++i) {
            if (r.contains(i->first))
                nm[i->first] = i->second;
        }
        nm.canonize();
        return nm;
    }

    memory
    offset(memory const& m, int off) {
        memory nm;
        for (memory::const_iterator i = m.begin(); i != m.end(); ++i)
            nm[i->first + off] = i->second;
        return nm;
    }

    memory
    join(memory const& m, memory const& m2) {
        memory nm(m); 
        for (memory::const_iterator i = m2.begin(); i != m2.end(); ++i)
            nm[i->first] = i->second;
        return nm;
    }
}
