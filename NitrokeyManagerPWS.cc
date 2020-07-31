#include "NitrokeyManagerPWS.h"
#include "NitrokeyManagerOTP.h"
#include "libnitrokey/LibraryException.h"
#include "libnitrokey/NitrokeyManager.h"
#include "libnitrokey/cxx_semantics.h"
#include "libnitrokey/misc.h"
#include <algorithm>
#include <cstring>
#include <functional>
#include <mutex>
#include <stick10_commands.h>
#include <stick20_commands.h>
#include <unordered_map>
void nitrokey::NitrokeyManager::enable_password_safe(const char *user_pin) {
    //The following command will cancel enabling PWS if it is not supported
    auto a = get_payload<IsAESSupported>();
    misc::strcpyT(a.user_password, user_pin);
    IsAESSupported::CommandTransaction::run(device, a);

    auto p = get_payload<EnablePasswordSafe>();
    misc::strcpyT(p.user_password, user_pin);
    EnablePasswordSafe::CommandTransaction::run(device, p);
}
std::vector <uint8_t> nitrokey::NitrokeyManager::get_password_safe_slot_status() {
    auto responsePayload = GetPasswordSafeSlotStatus::CommandTransaction::run(device);
    vector<uint8_t> v = vector<uint8_t>(responsePayload.data().password_safe_status,
                                        responsePayload.data().password_safe_status
                                        + sizeof(responsePayload.data().password_safe_status));
    return v;
}
char * nitrokey::NitrokeyManager::get_password_safe_slot_name(uint8_t slot_number) {
    if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
    auto p = get_payload<GetPasswordSafeSlotName>();
    p.slot_number = slot_number;
    auto response = GetPasswordSafeSlotName::CommandTransaction::run(device, p);
    return strndup((const char *) response.data().slot_name, max_string_field_length);
}
bool nitrokey::NitrokeyManager::is_valid_password_safe_slot_number(uint8_t slot_number) const { return slot_number < 16;
}
char *
nitrokey::NitrokeyManager::get_password_safe_slot_login(uint8_t slot_number) {
    if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
    auto p = get_payload<GetPasswordSafeSlotLogin>();
    p.slot_number = slot_number;
    auto response = GetPasswordSafeSlotLogin::CommandTransaction::run(device, p);
    return strndup((const char *) response.data().slot_login, max_string_field_length);
}
char * nitrokey::NitrokeyManager::get_password_safe_slot_password(uint8_t slot_number) {
    if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
    auto p = get_payload<GetPasswordSafeSlotPassword>();
    p.slot_number = slot_number;
    auto response = GetPasswordSafeSlotPassword::CommandTransaction::run(device, p);
    return strndup((const char *) response.data().slot_password, max_string_field_length); //FIXME use secure way
}
void nitrokey::NitrokeyManager::write_password_safe_slot(uint8_t slot_number, const char *slot_name, const char *slot_login,
                                                   const char *slot_password) {
    if (!is_valid_password_safe_slot_number(slot_number))
      throw InvalidSlotException(slot_number);
    auto p = get_payload<SetPasswordSafeSlotData>();
    p.slot_number = slot_number;
    misc::strcpyT(p.slot_name, slot_name);
    misc::strcpyT(p.slot_password, slot_password);
    SetPasswordSafeSlotData::CommandTransaction::run(device, p);

    auto p2 = get_payload<SetPasswordSafeSlotData2>();
    p2.slot_number = slot_number;
    misc::strcpyT(p2.slot_login_name, slot_login);
    SetPasswordSafeSlotData2::CommandTransaction::run(device, p2);
}
void nitrokey::NitrokeyManager::erase_password_safe_slot(uint8_t slot_number) {
    if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
    auto p = get_payload<ErasePasswordSafeSlot>();
    p.slot_number = slot_number;
    ErasePasswordSafeSlot::CommandTransaction::run(device, p);
}