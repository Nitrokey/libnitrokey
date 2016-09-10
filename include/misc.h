#ifndef MISC_H
#define MISC_H
#include <stdio.h>
#include <string>
#include <vector>

namespace nitrokey {
namespace misc {

template <typename T>
typename T::CommandPayload get_payload(){
    //Create, initialize and return by value command payload
    typename T::CommandPayload st;
    bzero(&st, sizeof(st));
    return st;
}


    std::string hexdump(const char *p, size_t size, bool print_header=true);
    uint32_t stm_crc32(const uint8_t *data, size_t size);
    std::vector<uint8_t> hex_string_to_byte(const char* hexString);
}
}

#endif
