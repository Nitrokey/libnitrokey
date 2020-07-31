
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
#include <tuple>

#include "nk_strndup.h"

using namespace nitrokey;
const uint8_t NK_PWS_SLOT_COUNT = PWS_SLOT_COUNT;


#ifdef __cplusplus
extern "C" {
#endif

NK_C_API int NK_enable_password_safe(const char *user_pin) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() { m->enable_password_safe(user_pin); });
}
NK_C_API uint8_t *NK_get_password_safe_slot_status() {
  auto m = NitrokeyManager::instance();
  return get_with_array_result([&]() {
    auto slot_status = m->get_password_safe_slot_status();
    return duplicate_vector_and_clear(slot_status);
  });
}

NK_C_API void NK_free_password_safe_slot_status(uint8_t *status) {
  delete[] status;
}

NK_C_API char *NK_get_password_safe_slot_name(uint8_t slot_number) {
  auto m = NitrokeyManager::instance();
  return get_with_string_result(
      [&]() { return m->get_password_safe_slot_name(slot_number); });
}

NK_C_API char *NK_get_password_safe_slot_login(uint8_t slot_number) {
  auto m = NitrokeyManager::instance();
  return get_with_string_result(
      [&]() { return m->get_password_safe_slot_login(slot_number); });
}
NK_C_API char *NK_get_password_safe_slot_password(uint8_t slot_number) {
  auto m = NitrokeyManager::instance();
  return get_with_string_result(
      [&]() { return m->get_password_safe_slot_password(slot_number); });
}
NK_C_API int NK_write_password_safe_slot(uint8_t slot_number,
                                         const char *slot_name,
                                         const char *slot_login,
                                         const char *slot_password) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->write_password_safe_slot(slot_number, slot_name, slot_login,
                                slot_password);
  });
}

NK_C_API int NK_erase_password_safe_slot(uint8_t slot_number) {
  auto m = NitrokeyManager::instance();
  return get_without_result(
      [&]() { m->erase_password_safe_slot(slot_number); });
}

#ifdef __cplusplus
}
#endif
