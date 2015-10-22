#ifndef MISC_H
#define MISC_H
#include <stdio.h>
#include <string>

namespace nitrokey {
namespace misc {

std::string hexdump(const char *p, size_t size);
uint32_t stm_crc32(const uint8_t *data, size_t size);

}
}

#endif
