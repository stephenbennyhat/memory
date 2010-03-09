#ifndef CRC_H
#define CRC_H

#include "memory.h"

uint16 crc16(memory const& m, uint16 init = 0xFFFF);

#endif
