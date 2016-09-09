#include <sstream>
#include <string>
#include "misc.h"
#include "inttypes.h"
#include <cstdlib>
#include <cstring>
#include <cassert>

namespace nitrokey {
namespace misc {

std::vector<uint8_t> hex_string_to_byte(const char* hexString){
    const size_t s_size = strlen(hexString);
    const size_t d_size = (s_size+1)/2; // add 1 for odd, ignore for even
    assert(s_size%2==0);
    assert(s_size<256); //arbitrary 'big' number
    auto data = std::vector<uint8_t>(d_size, 0);

    char buf[2];
    for(int i=0; i<s_size; i++){

        char c = hexString[i];
        bool char_from_range = ('0' <= c && c <='9' || 'A' <= c && c <= 'F' || 'a' <= c && c<= 'f');
        if (!char_from_range){
            return {};
        }
        buf[i%2] = c;
        if (i%2==1){
            data[i/2] = strtoul(buf, NULL, 16) & 0xFF;
        }
    }
    return data;
};

std::string hexdump(const char *p, size_t size, bool print_header) {
  std::stringstream out;
  char formatbuf[128];
  const char *pstart = p;

  for (const char *pend = p + size; p < pend;) {
      if (print_header){
          snprintf(formatbuf, 128, "%04lx\t", p - pstart);
          out << formatbuf;
      }

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
