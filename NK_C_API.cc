#include <cstring>
#include "NK_C_API.h"

using namespace nitrokey;

extern "C"
{
extern int NK_login(const char *pin, const char *temporary_password) {
    auto m = NitrokeyManager::instance();
    return m->connect() && m->authorize(pin, temporary_password);
}

extern int NK_logout() {
    auto m = NitrokeyManager::instance();
    return m->disconnect();
}
extern const char * NK_status() {
    auto m = NitrokeyManager::instance();
    string s = m->get_status();
    return strdup(s.c_str());
}

extern uint32_t NK_get_hotp_code(uint8_t slot_number){
    auto m = NitrokeyManager::instance();
    return m->get_HOTP_code(slot_number);
}

extern uint32_t NK_get_totp_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time,
                                 uint8_t last_interval){
    auto m = NitrokeyManager::instance();
    return m->get_TOTP_code(slot_number, 0, 0, 0);
}

extern int NK_erase_hotp_slot(uint8_t slot_number) {
    auto m = NitrokeyManager::instance();
    return m->erase_hotp_slot(slot_number);
}
extern int NK_erase_totp_slot(uint8_t slot_number) {
    auto m = NitrokeyManager::instance();
    return m->erase_totp_slot(slot_number);
}

extern int NK_write_hotp_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint8_t hotp_counter,
                              const char *temporary_password) {
    auto m = NitrokeyManager::instance();
    return m->write_HOTP_slot(slot_number, slot_name, secret, hotp_counter, temporary_password);
}

extern int NK_write_totp_slot(uint8_t slot_number, const char *secret, uint16_t time_window) {
    auto m = NitrokeyManager::instance();
    return m->write_TOTP_slot(slot_number, secret, time_window);
}

extern const char* NK_get_totp_slot_name(uint8_t slot_number){
    auto m = NitrokeyManager::instance();
    return m->get_totp_slot_name(slot_number);
}
extern const char* NK_get_hotp_slot_name(uint8_t slot_number){
    auto m = NitrokeyManager::instance();
    return m->get_hotp_slot_name(slot_number);
}

extern void NK_set_debug(bool state){
    auto m = NitrokeyManager::instance();
    m->set_debug(state);
}


}