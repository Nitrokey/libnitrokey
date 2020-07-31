
#include "NK_C_API.h"
#include "NK_C_API_helpers.h"
#include "NitrokeyManagerOTP.h"
#include "NitrokeyManagerPWS.h"
#include "libnitrokey/LibraryException.h"
#include "libnitrokey/NitrokeyManager.h"
#include "libnitrokey/cxx_semantics.h"
#include "libnitrokey/device_proto.h"
#include "libnitrokey/stick20_commands.h"
#include "libnitrokey/version.h"
#include <cstring>
#include <iostream>
#include <tuple>

#include "nk_strndup.h"

using namespace nitrokey;


#ifdef __cplusplus
extern "C" {
#endif


NK_C_API int NK_totp_set_time(uint64_t time) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->set_time(time);
  });
}

NK_C_API int NK_totp_set_time_soft(uint64_t time) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->set_time_soft(time);
  });
}

NK_C_API int NK_totp_get_time() {
  return 0;
}


NK_C_API char * NK_get_hotp_code(uint8_t slot_number) {
  return NK_get_hotp_code_PIN(slot_number, "");
}

NK_C_API char * NK_get_hotp_code_PIN(uint8_t slot_number, const char *user_temporary_password) {
  auto m = NitrokeyManager::instance();
  return get_with_string_result([&]() {
    string && s = m->get_HOTP_code(slot_number, user_temporary_password);
    char * rs = strndup(s.c_str(), max_string_field_length);
    clear_string(s);
    return rs;
  });
}

NK_C_API char * NK_get_totp_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time,
                                 uint8_t last_interval) {
  return NK_get_totp_code_PIN(slot_number, challenge, last_totp_time, last_interval, "");
}

NK_C_API char * NK_get_totp_code_PIN(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time,
                                     uint8_t last_interval, const char *user_temporary_password) {
  auto m = NitrokeyManager::instance();
  return get_with_string_result([&]() {
    string && s = m->get_TOTP_code(slot_number, challenge, last_totp_time, last_interval, user_temporary_password);
    char * rs = strndup(s.c_str(), max_string_field_length);
    clear_string(s);
    return rs;
  });
}

NK_C_API int NK_erase_hotp_slot(uint8_t slot_number, const char *temporary_password) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&] {
    m->erase_hotp_slot(slot_number, temporary_password);
  });
}

NK_C_API int NK_erase_totp_slot(uint8_t slot_number, const char *temporary_password) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&] {
    m->erase_totp_slot(slot_number, temporary_password);
  });
}

NK_C_API int NK_write_hotp_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint64_t hotp_counter,
                                bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                                const char *temporary_password) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&] {
    m->write_HOTP_slot(slot_number, slot_name, secret, hotp_counter, use_8_digits, use_enter, use_tokenID, token_ID,
                       temporary_password);
  });
}

NK_C_API int NK_write_totp_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint16_t time_window,
                                bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                                const char *temporary_password) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&] {
    m->write_TOTP_slot(slot_number, slot_name, secret, time_window, use_8_digits, use_enter, use_tokenID, token_ID,
                       temporary_password);
  });
}

NK_C_API char* NK_get_totp_slot_name(uint8_t slot_number) {
  auto m = NitrokeyManager::instance();
  return get_with_string_result([&]() {
    const auto slot_name = m->get_totp_slot_name(slot_number);
    return slot_name;
  });
}
NK_C_API char* NK_get_hotp_slot_name(uint8_t slot_number) {
  auto m = NitrokeyManager::instance();
  return get_with_string_result([&]() {
    const auto slot_name = m->get_hotp_slot_name(slot_number);
    return slot_name;
  });
}


NK_C_API int NK_read_HOTP_slot(const uint8_t slot_num, struct ReadSlot_t* out){
  if (out == nullptr)
    return -1;
  auto m = NitrokeyManager::instance();
  auto result = get_with_status([&]() {
    return m->get_HOTP_slot_data(slot_num);
  }, stick10::ReadSlot::ResponsePayload() );
  auto error_code = std::get<0>(result);
  if (error_code != 0) {
    return error_code;
  }
#define a(x) out->x = read_slot.x
  stick10::ReadSlot::ResponsePayload read_slot = std::get<1>(result);
  a(_slot_config);
  a(slot_counter);
#undef a
#define m(x) memmove(out->x, read_slot.x, sizeof(read_slot.x))
  m(slot_name);
  m(slot_token_id);
#undef m
  return 0;
}


NK_C_API int NK_write_config(uint8_t numlock, uint8_t capslock, uint8_t scrolllock, bool enable_user_password,
                             bool delete_user_password,
                             const char *admin_temporary_password) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    return m->write_config(numlock, capslock, scrolllock, enable_user_password, delete_user_password, admin_temporary_password);
  });
}

NK_C_API int NK_write_config_struct(struct NK_config config,
                                    const char *admin_temporary_password) {
  return NK_write_config(config.numlock, config.capslock, config.scrolllock, config.enable_user_password,
                         config.disable_user_password, admin_temporary_password);
}


NK_C_API uint8_t* NK_read_config() {
  auto m = NitrokeyManager::instance();
  return get_with_array_result([&]() {
    auto v = m->read_config();
    return duplicate_vector_and_clear(v);
  });
}

NK_C_API void NK_free_config(uint8_t* config) {
  delete[] config;
}

NK_C_API int NK_read_config_struct(struct NK_config* out) {
  if (out == nullptr) {
    return -1;
  }
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    auto v = m->read_config();
    out->numlock = v[0];
    out->capslock = v[1];
    out->scrolllock = v[2];
    out->enable_user_password = v[3];
    out->disable_user_password = v[4];
  });
}



#ifdef __cplusplus
}
#endif
