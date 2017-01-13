#include <cstring>
#include "NK_C_API.h"
#include "include/LibraryException.h"

using namespace nitrokey;

static uint8_t NK_last_command_status = 0;

template <typename T>
T* duplicate_vector_and_clear(std::vector<T> &v){
    auto d = new T[v.size()];
    std::copy(v.begin(), v.end(), d);
    std::fill(v.begin(), v.end(), 0);
    return d;
}

template <typename T>
uint8_t * get_with_array_result(T func){
    NK_last_command_status = 0;
    try {
        return func();
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
    }
    catch (LibraryException & libraryException){
        NK_last_command_status = libraryException.exception_id();
    }
    return nullptr;
}

template <typename T>
const char* get_with_string_result(T func){
    NK_last_command_status = 0;
    try {
        return func();
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
    }
    catch (LibraryException & libraryException){
        NK_last_command_status = libraryException.exception_id();
    }
    return "";
}

template <typename T>
auto get_with_result(T func){
    NK_last_command_status = 0;
    try {
        return func();
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
    }
    catch (LibraryException & libraryException){
        NK_last_command_status = libraryException.exception_id();
    }
    return static_cast<decltype(func())>(0);
}

template <typename T>
uint8_t get_without_result(T func){
    NK_last_command_status = 0;
    try {
        func();
        return 0;
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
    }
    catch (LibraryException & libraryException){
        NK_last_command_status = libraryException.exception_id();
    }
    return NK_last_command_status;
}

extern "C"
{
extern uint8_t NK_get_last_command_status(){
    auto _copy = NK_last_command_status;
    NK_last_command_status = 0;
    return _copy;
}

extern int NK_login(const char *device_model) {
    auto m = NitrokeyManager::instance();
    try {
        NK_last_command_status = 0;
        return m->connect(device_model);
    }
    catch (CommandFailedException & commandFailedException){
        NK_last_command_status = commandFailedException.last_command_status;
        return commandFailedException.last_command_status;
    }
    catch (std::runtime_error &e){
        cerr << e.what() << endl;
        return 0;
    }
    return 0;
}

extern int NK_logout() {
    auto m = NitrokeyManager::instance();
    return get_without_result( [&](){
        m->disconnect();
    });
}

extern int NK_first_authenticate(const char* admin_password, const char* admin_temporary_password){
    auto m = NitrokeyManager::instance();
    return get_without_result( [&](){
        return m->first_authenticate(admin_password, admin_temporary_password);
    });
}


extern int NK_user_authenticate(const char* user_password, const char* user_temporary_password){
    auto m = NitrokeyManager::instance();
    return get_without_result( [&](){
        m->user_authenticate(user_password, user_temporary_password);
    });
}

extern int NK_factory_reset(const char* admin_password){
    auto m = NitrokeyManager::instance();
    return get_without_result( [&](){
        m->factory_reset(admin_password);
    });
}
extern int NK_build_aes_key(const char* admin_password){
    auto m = NitrokeyManager::instance();
    return get_without_result( [&](){
        m->build_aes_key(admin_password);
    });
}

extern int NK_unlock_user_password(const char *admin_password, const char *new_user_password) {
    auto m = NitrokeyManager::instance();
    return get_without_result( [&](){
        m->unlock_user_password(admin_password, new_user_password);
    });
}

extern int NK_write_config(uint8_t numlock, uint8_t capslock, uint8_t scrolllock, bool enable_user_password,
                           bool delete_user_password,
                           const char *admin_temporary_password) {
    auto m = NitrokeyManager::instance();
    return get_without_result( [&](){
        return m->write_config(numlock, capslock, scrolllock, enable_user_password, delete_user_password, admin_temporary_password);
    });
}


extern uint8_t* NK_read_config(){
    auto m = NitrokeyManager::instance();
    return get_with_array_result( [&](){
        auto v = m->read_config();
        return duplicate_vector_and_clear(v);
    });
}


void clear_string(std::string &s){
    std::fill(s.begin(), s.end(), ' ');
}

extern const char * NK_status() {
    auto m = NitrokeyManager::instance();
    return get_with_string_result([&](){
        string && s = m->get_status_as_string();
        char * rs = strdup(s.c_str());
        clear_string(s);
        return rs;
    });
}

extern const char * NK_device_serial_number(){
    auto m = NitrokeyManager::instance();
    return get_with_string_result([&](){
        string && s = m->get_serial_number();
        char * rs = strdup(s.c_str());
        clear_string(s);
        return rs;
    });
}

extern uint32_t NK_get_hotp_code(uint8_t slot_number) {
    return NK_get_hotp_code_PIN(slot_number, "");
}

extern uint32_t NK_get_hotp_code_PIN(uint8_t slot_number, const char* user_temporary_password){
    auto m = NitrokeyManager::instance();
    return get_with_result([&](){
        return m->get_HOTP_code(slot_number, user_temporary_password);
    });
}

extern uint32_t NK_get_totp_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time,
                                 uint8_t last_interval){
    return NK_get_totp_code_PIN(slot_number, challenge, last_totp_time, last_interval, "");
}

extern uint32_t NK_get_totp_code_PIN(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time,
                                 uint8_t last_interval, const char* user_temporary_password){
    auto m = NitrokeyManager::instance();
    return get_with_result([&](){
        return m->get_TOTP_code(slot_number, challenge, last_totp_time, last_interval, user_temporary_password);
    });
}

extern int NK_erase_hotp_slot(uint8_t slot_number, const char *temporary_password) {
    auto m = NitrokeyManager::instance();
    return get_without_result([&]{
        m->erase_hotp_slot(slot_number, temporary_password);
    });
}

extern int NK_erase_totp_slot(uint8_t slot_number, const char *temporary_password) {
    auto m = NitrokeyManager::instance();
    return get_without_result([&]{
        m->erase_totp_slot(slot_number, temporary_password);
    });
}

extern int NK_write_hotp_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint64_t hotp_counter,
                              bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                              const char *temporary_password) {
    auto m = NitrokeyManager::instance();
    return get_without_result([&]{
        m->write_HOTP_slot(slot_number, slot_name, secret, hotp_counter, use_8_digits, use_enter, use_tokenID, token_ID,
                           temporary_password);
    });
}

extern int NK_write_totp_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint16_t time_window,
                              bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                              const char *temporary_password) {
    auto m = NitrokeyManager::instance();
    return get_without_result([&]{
        m->write_TOTP_slot(slot_number, slot_name, secret, time_window, use_8_digits, use_enter, use_tokenID, token_ID,
                           temporary_password);
    });
}

extern const char* NK_get_totp_slot_name(uint8_t slot_number){
    auto m = NitrokeyManager::instance();
    return get_with_string_result([&]() {
        const auto slot_name = m->get_totp_slot_name(slot_number);
        return slot_name;
    });
}
extern const char* NK_get_hotp_slot_name(uint8_t slot_number){
    auto m = NitrokeyManager::instance();
    return get_with_string_result([&]() {
        const auto slot_name = m->get_hotp_slot_name(slot_number);
        return slot_name;
    });
}

extern void NK_set_debug(bool state){
    auto m = NitrokeyManager::instance();
    m->set_debug(state);
}

extern int NK_totp_set_time(uint64_t time){
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->set_time(time);
    });
}

extern int NK_totp_get_time(){
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->get_time(); // FIXME check how that should work
    });
}

extern int NK_change_admin_PIN(char *current_PIN, char *new_PIN){
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->change_admin_PIN(current_PIN, new_PIN);
    });
}

extern int NK_change_user_PIN(char *current_PIN, char *new_PIN){
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->change_user_PIN(current_PIN, new_PIN);
    });
}

extern int NK_enable_password_safe(const char *user_pin){
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->enable_password_safe(user_pin);
    });
}
extern uint8_t * NK_get_password_safe_slot_status(){
    auto m = NitrokeyManager::instance();
    return get_with_array_result( [&](){
        auto slot_status = m->get_password_safe_slot_status();
        return duplicate_vector_and_clear(slot_status);
    });

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

extern const char *NK_get_password_safe_slot_name(uint8_t slot_number) {
    auto m = NitrokeyManager::instance();
    return get_with_string_result([&](){
        return m->get_password_safe_slot_name(slot_number);
    });
}

extern const char *NK_get_password_safe_slot_login(uint8_t slot_number) {
    auto m = NitrokeyManager::instance();
    return get_with_string_result([&](){
        return m->get_password_safe_slot_login(slot_number);
    });
}
extern const char *NK_get_password_safe_slot_password(uint8_t slot_number) {
    auto m = NitrokeyManager::instance();
    return get_with_string_result([&](){
        return m->get_password_safe_slot_password(slot_number);
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

extern int NK_is_AES_supported(const char *user_password) {
    auto m = NitrokeyManager::instance();
    return get_with_result([&](){
       return (uint8_t) m->is_AES_supported(user_password);
    });
}

extern int NK_login_auto() {
    auto m = NitrokeyManager::instance();
    return get_with_result([&](){
        return (uint8_t) m->connect();
    });
}

// storage commands

extern int NK_send_startup(uint64_t seconds_from_epoch){
  auto m = NitrokeyManager::instance();
  return get_without_result([&](){
      m->send_startup(seconds_from_epoch);
  });
}

extern int NK_unlock_encrypted_volume(const char* user_pin){
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->unlock_encrypted_volume(user_pin);
    });
}

extern int NK_unlock_hidden_volume(const char* hidden_volume_password){
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->unlock_hidden_volume(hidden_volume_password);
    });
}

extern int NK_create_hidden_volume(uint8_t slot_nr, uint8_t start_percent, uint8_t end_percent,
                                   const char *hidden_volume_password){
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->create_hidden_volume( slot_nr,  start_percent,  end_percent,
        hidden_volume_password);
    });
}

extern int NK_set_unencrypted_read_only(const char* user_pin){
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->set_unencrypted_read_only(user_pin);
    });
}

extern int NK_set_unencrypted_read_write(const char* user_pin){
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->set_unencrypted_read_write(user_pin);
    });
}

extern int NK_export_firmware(const char* admin_pin) {
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->export_firmware(admin_pin) ;
    });
}

extern int NK_clear_new_sd_card_warning(const char* admin_pin) {
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->clear_new_sd_card_warning(admin_pin);
    });
}

extern int NK_fill_SD_card_with_random_data(const char* admin_pin) {
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->fill_SD_card_with_random_data(admin_pin);
    });
}

extern int NK_change_update_password(const char* current_update_password,
                                     const char* new_update_password) {
    auto m = NitrokeyManager::instance();
    return get_without_result([&](){
        m->change_update_password(current_update_password, new_update_password);
    });
}

extern const char* NK_get_status_storage_as_string() {
  auto m = NitrokeyManager::instance();
  return get_with_string_result([&](){
      return m->get_status_storage_as_string();
  });
}

extern const char* NK_get_SD_usage_data_as_string() {
  auto m = NitrokeyManager::instance();
  return get_with_string_result([&](){
      return m->get_SD_usage_data_as_string();
  });
}

extern int NK_get_progress_bar_value() {
  auto m = NitrokeyManager::instance();
  return get_with_result([&](){
      return m->get_progress_bar_value();
  });
}

extern int NK_get_major_firmware_version(){
  auto m = NitrokeyManager::instance();
  return get_with_result([&](){
      return m->get_minor_firmware_version();
  });
}


}

