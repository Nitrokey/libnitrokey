#ifndef LIBNITROKEY_NK_C_API_H
#define LIBNITROKEY_NK_C_API_H

#include <iostream>
#include <string>
#include "include/NitrokeyManager.h"
#include "include/inttypes.h"

extern "C"
{
extern void NK_set_debug(bool state);
extern int NK_login(const char *pin, const char *temporary_password);
extern int NK_logout();
extern const char * NK_status();
extern const char * NK_get_totp_slot_name(uint8_t slot_number);
extern const char * NK_get_hotp_slot_name(uint8_t slot_number);
extern int NK_erase_slot(uint8_t slot_number);
extern int NK_write_hotp_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint8_t hotp_counter, const char *temporary_password);
extern int NK_write_totp_slot(uint8_t slot_number, const char *secret, uint16_t time_window);
extern uint32_t NK_get_hotp_code(uint8_t slot_number);
extern uint32_t NK_get_totp_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time, uint8_t last_interval);
}


#endif //LIBNITROKEY_NK_C_API_H
