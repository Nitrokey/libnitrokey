#include <cstring>
#include "NK_C_API.h"
using namespace nitrokey;

static uint8_t NK_last_command_status = 0;

template <typename T>
const char* get_with_string_result(T func){
    try {
        return func();
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return "";
    }
}

template <typename T>
auto get_with_result(T func){
    try {
        return func();
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
}

template <typename T>
uint8_t get_without_result(T func){
    try {
        func();
        return 0;
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
}

extern "C"
{
extern uint8_t NK_get_last_command_status(){
    auto _copy = NK_last_command_status;
    NK_last_command_status = 0;
    return _copy;
}

extern int NK_login(const char *admin_pin, const char *temporary_password) {
    auto m = NitrokeyManager::instance();
    try {
        m->connect();
        m->authorize(admin_pin, temporary_password);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    return 0;
}

extern int NK_logout() {
    auto m = NitrokeyManager::instance();
    try {
        m->disconnect();
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    return 0;
}

extern const char * NK_status() {
    auto m = NitrokeyManager::instance();
    try {
        string s = m->get_status();
        return strdup(s.c_str()); //FIXME leak?
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
    }
    return "";
}

extern uint32_t NK_get_hotp_code(uint8_t slot_number){
    auto m = NitrokeyManager::instance();
    try {
        return m->get_HOTP_code(slot_number);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
    }
    return 0;
}

extern uint32_t NK_get_totp_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time,
                                 uint8_t last_interval){
    auto m = NitrokeyManager::instance();
    try {
        return m->get_TOTP_code(slot_number, challenge, last_totp_time, last_interval);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
    }
    return 0;
}

extern int NK_erase_hotp_slot(uint8_t slot_number) {
    auto m = NitrokeyManager::instance();
    try {
        m->erase_hotp_slot(slot_number);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    return 0;
}

extern int NK_erase_totp_slot(uint8_t slot_number) {
    auto m = NitrokeyManager::instance();
    try {
        m->erase_totp_slot(slot_number);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    return 0;
}

extern int NK_write_hotp_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint8_t hotp_counter,
                              const char *temporary_password) {
    auto m = NitrokeyManager::instance();
    try {
        m->write_HOTP_slot(slot_number, slot_name, secret, hotp_counter, temporary_password);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    return 0;
}

extern int NK_write_totp_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint16_t time_window,
                              bool use_8_digits, const char *temporary_password) {
    auto m = NitrokeyManager::instance();
    try {
        m->write_TOTP_slot(slot_number, slot_name, secret, time_window, use_8_digits, temporary_password);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    return 0;
}

extern const char* NK_get_totp_slot_name(uint8_t slot_number){
    auto m = NitrokeyManager::instance();
    try {
        return m->get_totp_slot_name(slot_number);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return "";
    }
}
extern const char* NK_get_hotp_slot_name(uint8_t slot_number){
    auto m = NitrokeyManager::instance();
    try {
        return m->get_hotp_slot_name(slot_number);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return "";
    }
}

extern void NK_set_debug(bool state){
    auto m = NitrokeyManager::instance();
    m->set_debug(state);
}

extern int NK_totp_set_time(uint64_t time){
    auto m = NitrokeyManager::instance();
    try {
        m->set_time(time);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    return 0;
}

extern int NK_totp_get_time(){
    auto m = NitrokeyManager::instance();
    try {
        m->get_time();
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    return 0;
}

extern int NK_change_admin_PIN(char *current_PIN, char *new_PIN){
    auto m = NitrokeyManager::instance();
    try {
        m->change_admin_PIN(current_PIN, new_PIN);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    return 0;
}

extern int NK_change_user_PIN(char *current_PIN, char *new_PIN){
    auto m = NitrokeyManager::instance();
    try {
        m->change_user_PIN(current_PIN, new_PIN);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    return 0;
}

extern int NK_enable_password_safe(const char *user_pin){
    auto m = NitrokeyManager::instance();
    try {
        m->enable_password_safe(user_pin);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    return 0;
}
extern int NK_get_password_safe_slot_status(){
    auto m = NitrokeyManager::instance();
    try {
        m->get_password_safe_slot_status(); //TODO FIXME
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    return 0;
}

extern uint8_t NK_get_user_retry_count(){
    auto m = NitrokeyManager::instance();
    return get_with_result([&](){
        return m->get_user_retry_count();
    });
}

extern uint8_t NK_get_admin_retry_count(){
    auto m = NitrokeyManager::instance();
    return get_with_result([&](){
        return m->get_admin_retry_count();
    });
}

extern int NK_lock_device(){
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->lock_device();
    });
}

extern const char *NK_get_password_safe_slot_name(uint8_t slot_number, const char *temporary_password) {
    auto m = NitrokeyManager::instance();
    return get_with_string_result([&](){
        return m->get_password_safe_slot_name(slot_number, temporary_password);
    });
}

extern const char *NK_get_password_safe_slot_login(uint8_t slot_number, const char *temporary_password) {
    auto m = NitrokeyManager::instance();
    return get_with_string_result([&](){
        return m->get_password_safe_slot_login(slot_number, temporary_password);
    });
}
extern const char *NK_get_password_safe_slot_password(uint8_t slot_number, const char *temporary_password) {
    auto m = NitrokeyManager::instance();
    return get_with_string_result([&](){
        return m->get_password_safe_slot_password(slot_number, temporary_password);
    });
}
extern int NK_write_password_safe_slot(uint8_t slot_number, const char *slot_name, const char *slot_login,
                                       const char *slot_password) {
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->write_password_safe_slot(slot_number, slot_name, slot_login, slot_password);
    });
}

extern int NK_erase_password_safe_slot(uint8_t slot_number) {
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->erase_password_safe_slot(slot_number);
    });
}


}

