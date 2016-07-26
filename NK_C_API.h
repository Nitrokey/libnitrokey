#ifndef LIBNITROKEY_NK_C_API_H
#define LIBNITROKEY_NK_C_API_H

#include <iostream>
#include <string>
#include "include/NitrokeyManager.h"
#include "include/inttypes.h"

extern "C"
{
//Make sure each function's declaration is in one line (for automatic python declaration processing)
extern void NK_set_debug(bool state);
extern int NK_login(const char *admin_pin, const char *temporary_password);
extern int NK_logout();
extern const char * NK_status();
extern uint8_t NK_get_last_command_status();
extern int NK_lock_device();
//otp
extern const char * NK_get_totp_slot_name(uint8_t slot_number);
extern const char * NK_get_hotp_slot_name(uint8_t slot_number);
extern int NK_erase_slot(uint8_t slot_number);
extern int NK_write_hotp_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint8_t hotp_counter, const char *temporary_password);
extern int NK_write_totp_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint16_t time_window, bool use_8_digits, const char *temporary_password);
extern uint32_t NK_get_hotp_code(uint8_t slot_number);
extern uint32_t NK_get_totp_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time, uint8_t last_interval);
extern int NK_totp_set_time(uint64_t time);
extern int NK_totp_get_time();
//passwords
extern int NK_change_admin_PIN(char *current_PIN, char *new_PIN);
extern int NK_change_user_PIN(char *current_PIN, char *new_PIN);
extern uint8_t NK_get_user_retry_count();
extern uint8_t NK_get_admin_retry_count();
//password safe
extern int NK_enable_password_safe(const char *user_pin);
extern int NK_get_password_safe_slot_status();
extern const char *NK_get_password_safe_slot_name(uint8_t slot_number, const char *temporary_password);
extern const char *NK_get_password_safe_slot_login(uint8_t slot_number, const char *temporary_password);
extern const char *NK_get_password_safe_slot_password(uint8_t slot_number, const char *temporary_password);
extern int NK_write_password_safe_slot();
}


#endif //LIBNITROKEY_NK_C_API_H
