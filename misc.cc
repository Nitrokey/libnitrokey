#include <sstream>
#include <string>
#include "misc.h"
#include "inttypes.h"

namespace nitrokey {
namespace misc {

std::string hexdump(const char *p, size_t size) {
  std::stringstream out;
  char formatbuf[128];
  const char *pstart = p;

  for (const char *pend = p + size; p < pend;) {
    snprintf(formatbuf, 128, "%04x\t", p - pstart);
    out << formatbuf;

    for (const char *le = p + 16; p < le && p < pend; p++) {
      snprintf(formatbuf, 128, "%02x ", uint8_t(*p));
      out << formatbuf;
    }
    out << std::endl;
  }
  return out.str();
}

static uint32_t _crc32(uint32_t crc, uint32_t data) {
  int i;
  crc = crc ^ data;

  for (i = 0; i < 32; i++) {
    if (crc & 0x80000000)
      crc = (crc << 1) ^ 0x04C11DB7;  // polynomial used in STM32
    else
      crc = (crc << 1);
  }

  return crc;
}

uint32_t stm_crc32(const uint8_t *data, size_t size) {
  uint32_t crc = 0xffffffff;
  const uint32_t *pend = (const uint32_t *)(data + size);
  for (const uint32_t *p = (const uint32_t *)(data); p < pend; p++)
    crc = _crc32(crc, *p);
  return crc;
}
}
}
