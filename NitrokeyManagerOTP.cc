#include "NitrokeyManagerOTP.h"
using nitrokey::misc::strcpyT;

std::string getFilledOTPCode(uint32_t code, bool use_8_digits){
  std::stringstream s;
  s << std::right << std::setw(use_8_digits ? 8 : 6) << std::setfill('0') << code;
  return s.str();
}
std::string nitrokey::NitrokeyManager::get_HOTP_code(uint8_t slot_number, const char *user_temporary_password) {
  if (!is_valid_hotp_slot_number(slot_number)) throw InvalidSlotException(slot_number);

  if (is_authorization_command_supported()){
    auto gh = get_payload<GetHOTP>();
    gh.slot_number = get_internal_slot_number_for_hotp(slot_number);
    if(user_temporary_password != nullptr && strlen(user_temporary_password)!=0){ //FIXME use string instead of strlen
        authorize_packet<GetHOTP, UserAuthorize>(gh, user_temporary_password, device);
    }
    auto resp = GetHOTP::CommandTransaction::run(device, gh);
    return getFilledOTPCode(resp.data().code, resp.data().use_8_digits);
  } else {
    auto gh = get_payload<stick10_08::GetHOTP>();
    gh.slot_number = get_internal_slot_number_for_hotp(slot_number);
    if(user_temporary_password != nullptr && strlen(user_temporary_password)!=0) {
      strcpyT(gh.temporary_user_password, user_temporary_password);
    }
    auto resp = stick10_08::GetHOTP::CommandTransaction::run(device, gh);
    return getFilledOTPCode(resp.data().code, resp.data().use_8_digits);
  }
  return "";
}
bool nitrokey::NitrokeyManager::is_internal_hotp_slot_number(uint8_t slot_number) const { return slot_number < 0x20; }
bool nitrokey::NitrokeyManager::is_valid_hotp_slot_number(uint8_t slot_number) const { return slot_number < 3; }
bool nitrokey::NitrokeyManager::is_valid_totp_slot_number(uint8_t slot_number) const { return slot_number < 0x10-1; }
uint8_t nitrokey::NitrokeyManager::get_internal_slot_number_for_totp(uint8_t slot_number) const { return (uint8_t) (0x20 + slot_number);
}
uint8_t nitrokey::NitrokeyManager::get_internal_slot_number_for_hotp(uint8_t slot_number) const { return (uint8_t) (0x10 + slot_number); }
std::string nitrokey::NitrokeyManager::get_TOTP_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time,
                                      uint8_t last_interval,
                                      const char *user_temporary_password) {
    if(!is_valid_totp_slot_number(slot_number)) throw InvalidSlotException(slot_number);
    slot_number = get_internal_slot_number_for_totp(slot_number);

    if (is_authorization_command_supported()){
      auto gt = get_payload<GetTOTP>();
      gt.slot_number = slot_number;
      gt.challenge = challenge;
      gt.last_interval = last_interval;
      gt.last_totp_time = last_totp_time;

      if(user_temporary_password != nullptr && strlen(user_temporary_password)!=0){ //FIXME use string instead of strlen
          authorize_packet<GetTOTP, UserAuthorize>(gt, user_temporary_password, device);
      }
      auto resp = GetTOTP::CommandTransaction::run(device, gt);
      return getFilledOTPCode(resp.data().code, resp.data().use_8_digits);
    } else {
      auto gt = get_payload<stick10_08::GetTOTP>();
      strcpyT(gt.temporary_user_password, user_temporary_password);
      gt.slot_number = slot_number;
      auto resp = stick10_08::GetTOTP::CommandTransaction::run(device, gt);
      return getFilledOTPCode(resp.data().code, resp.data().use_8_digits);
    }
  return "";
}
bool nitrokey::NitrokeyManager::erase_slot(uint8_t slot_number, const char *temporary_password) {
  if (is_authorization_command_supported()){
    auto p = get_payload<EraseSlot>();
    p.slot_number = slot_number;
    authorize_packet<EraseSlot, Authorize>(p, temporary_password, device);
    auto resp = EraseSlot::CommandTransaction::run(device,p);
  } else {
    auto p = get_payload<stick10_08::EraseSlot>();
    p.slot_number = slot_number;
    strcpyT(p.temporary_admin_password, temporary_password);
    auto resp = stick10_08::EraseSlot::CommandTransaction::run(device,p);
  }
    return true;
}
bool nitrokey::NitrokeyManager::erase_hotp_slot(uint8_t slot_number, const char *temporary_password) {
    if (!is_valid_hotp_slot_number(slot_number)) throw InvalidSlotException(slot_number);
    slot_number = get_internal_slot_number_for_hotp(slot_number);
    return erase_slot(slot_number, temporary_password);
}
bool nitrokey::NitrokeyManager::erase_totp_slot(uint8_t slot_number, const char *temporary_password) {
    if (!is_valid_totp_slot_number(slot_number)) throw InvalidSlotException(slot_number);
    slot_number = get_internal_slot_number_for_totp(slot_number);
    return erase_slot(slot_number, temporary_password);
}
template <typename T, typename U>
void vector_copy_ranged(T& dest, std::vector<U> &vec, std::size_t begin, std::size_t elements_to_copy){
    const std::size_t d_size = sizeof(dest);
  if(d_size < elements_to_copy){
      throw TargetBufferSmallerThanSource(elements_to_copy, d_size);
    }
    std::fill(dest, dest+d_size, 0);
    std::copy(vec.begin() + begin, vec.begin() +begin + elements_to_copy, dest);
}
template <typename T, typename U>
void vector_copy(T& dest, std::vector<U> &vec){
    const std::size_t d_size = sizeof(dest);
    if(d_size < vec.size()){
        throw TargetBufferSmallerThanSource(vec.size(), d_size);
    }
    std::fill(dest, dest+d_size, 0);
    std::copy(vec.begin(), vec.end(), dest);
}
bool nitrokey::NitrokeyManager::write_HOTP_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint64_t hotp_counter,
                                      bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                                      const char *temporary_password) {
    if (!is_valid_hotp_slot_number(slot_number)) throw InvalidSlotException(slot_number);

  int internal_slot_number = get_internal_slot_number_for_hotp(slot_number);
  if (is_authorization_command_supported()){
    write_HOTP_slot_authorize(internal_slot_number, slot_name, secret, hotp_counter, use_8_digits, use_enter, use_tokenID,
                token_ID, temporary_password);
  } else {
    write_OTP_slot_no_authorize(internal_slot_number, slot_name, secret, hotp_counter, use_8_digits, use_enter, use_tokenID,
                                token_ID, temporary_password);
  }
  return true;
}
void nitrokey::NitrokeyManager::write_HOTP_slot_authorize(uint8_t slot_number, const char *slot_name, const char *secret,
                                                uint64_t hotp_counter, bool use_8_digits, bool use_enter,
                                                bool use_tokenID, const char *token_ID, const char *temporary_password) {
  auto payload = get_payload<WriteToHOTPSlot>();
  payload.slot_number = slot_number;
  auto secret_bin = misc::hex_string_to_byte(secret);
  vector_copy(payload.slot_secret, secret_bin);
  strcpyT(payload.slot_name, slot_name);
  strcpyT(payload.slot_token_id, token_ID);
  switch (device->get_device_model() ){
    case DeviceModel::LIBREM:
    case DeviceModel::PRO: {
      payload.slot_counter = hotp_counter;
      break;
    }
    case DeviceModel::STORAGE: {
      string counter = to_string(hotp_counter);
      strcpyT(payload.slot_counter_s, counter.c_str());
      break;
    }
    default:
      LOG(string(__FILE__) + to_string(__LINE__) +
                      string(__FUNCTION__) + string(" Unhandled device model for HOTP")
          , Loglevel::DEBUG);
      break;
  }
  payload.use_8_digits = use_8_digits;
  payload.use_enter = use_enter;
  payload.use_tokenID = use_tokenID;

  authorize_packet<WriteToHOTPSlot, Authorize>(payload, temporary_password, device);

  auto resp = WriteToHOTPSlot::CommandTransaction::run(device, payload);
}
bool nitrokey::NitrokeyManager::write_TOTP_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint16_t time_window,
                                          bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                                          const char *temporary_password) {
    if (!is_valid_totp_slot_number(slot_number)) throw InvalidSlotException(slot_number);
    int internal_slot_number = get_internal_slot_number_for_totp(slot_number);

  if (is_authorization_command_supported()){
  write_TOTP_slot_authorize(internal_slot_number, slot_name, secret, time_window, use_8_digits, use_enter, use_tokenID,
                            token_ID, temporary_password);
  } else {
    write_OTP_slot_no_authorize(internal_slot_number, slot_name, secret, time_window, use_8_digits, use_enter, use_tokenID,
                                token_ID, temporary_password);
  }

  return true;
}
void nitrokey::NitrokeyManager::write_OTP_slot_no_authorize(uint8_t internal_slot_number, const char *slot_name,
                                                  const char *secret,
                                                  uint64_t counter_or_interval, bool use_8_digits, bool use_enter,
                                                  bool use_tokenID, const char *token_ID,
                                                  const char *temporary_password) const {

  auto payload2 = get_payload<stick10_08::SendOTPData>();
  strcpyT(payload2.temporary_admin_password, temporary_password);
  strcpyT(payload2.data, slot_name);
  payload2.setTypeName();
  stick10_08::SendOTPData::CommandTransaction::run(device, payload2);

  payload2.setTypeSecret();
  payload2.id = 0;
  auto secret_bin = misc::hex_string_to_byte(secret);
  auto remaining_secret_length = secret_bin.size();
  const auto maximum_OTP_secret_size = 40;
  if(remaining_secret_length > maximum_OTP_secret_size){
    throw TargetBufferSmallerThanSource(remaining_secret_length, maximum_OTP_secret_size);
  }

  while (remaining_secret_length>0){
    const auto bytesToCopy = std::min(sizeof(payload2.data), remaining_secret_length);
    const auto start = secret_bin.size() - remaining_secret_length;
    memset(payload2.data, 0, sizeof(payload2.data));
    vector_copy_ranged(payload2.data, secret_bin, start, bytesToCopy);
    stick10_08::SendOTPData::CommandTransaction::run(device, payload2);
    remaining_secret_length -= bytesToCopy;
    payload2.id++;
  }

  auto payload = get_payload<stick10_08::WriteToOTPSlot>();
  strcpyT(payload.temporary_admin_password, temporary_password);
  strcpyT(payload.slot_token_id, token_ID);
  payload.use_8_digits = use_8_digits;
  payload.use_enter = use_enter;
  payload.use_tokenID = use_tokenID;
  payload.slot_counter_or_interval = counter_or_interval;
  payload.slot_number = internal_slot_number;
  stick10_08::WriteToOTPSlot::CommandTransaction::run(device, payload);
}
void nitrokey::NitrokeyManager::write_TOTP_slot_authorize(uint8_t slot_number, const char *slot_name, const char *secret,
                                                uint16_t time_window, bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID, const char *temporary_password) {
  auto payload = get_payload<WriteToTOTPSlot>();
  payload.slot_number = slot_number;
  auto secret_bin = misc::hex_string_to_byte(secret);
  vector_copy(payload.slot_secret, secret_bin);
  strcpyT(payload.slot_name, slot_name);
  strcpyT(payload.slot_token_id, token_ID);
  payload.slot_interval = time_window; //FIXME naming
  payload.use_8_digits = use_8_digits;
  payload.use_enter = use_enter;
  payload.use_tokenID = use_tokenID;

  authorize_packet<WriteToTOTPSlot, Authorize>(payload, temporary_password, device);

  auto resp = WriteToTOTPSlot::CommandTransaction::run(device, payload);
}
char * nitrokey::NitrokeyManager::get_totp_slot_name(uint8_t slot_number) {
    if (!is_valid_totp_slot_number(slot_number)) throw InvalidSlotException(slot_number);
    slot_number = get_internal_slot_number_for_totp(slot_number);
    return get_slot_name(slot_number);
}
char *nitrokey::NitrokeyManager::get_hotp_slot_name(uint8_t slot_number) {
    if (!is_valid_hotp_slot_number(slot_number)) throw InvalidSlotException(slot_number);
    slot_number = get_internal_slot_number_for_hotp(slot_number);
    return get_slot_name(slot_number);
}
char *nitrokey::NitrokeyManager::get_slot_name(uint8_t slot_number)  {
    auto payload = get_payload<GetSlotName>();
    payload.slot_number = slot_number;
    auto resp = GetSlotName::CommandTransaction::run(device, payload);
    return strndup((const char *) resp.data().slot_name, max_string_field_length);
}
bool nitrokey::NitrokeyManager::set_time(uint64_t time) {
    auto p = get_payload<SetTime>();
    p.reset = 1;
    p.time = time;
    SetTime::CommandTransaction::run(device, p);
    return false;
}
void nitrokey::NitrokeyManager::set_time_soft(uint64_t time) {
    auto p = get_payload<SetTime>();
    p.reset = 0;
    p.time = time;
    SetTime::CommandTransaction::run(device, p);
}
bool nitrokey::NitrokeyManager::get_time(uint64_t time) {
    set_time_soft(time);
  return true;
}
void nitrokey::NitrokeyManager::write_config(uint8_t numlock, uint8_t capslock, uint8_t scrolllock, bool enable_user_password,
                                   bool delete_user_password, const char *admin_temporary_password) {
    auto p = get_payload<stick10_08::WriteGeneralConfig>();
    p.numlock = numlock;
    p.capslock = capslock;
    p.scrolllock = scrolllock;
    p.enable_user_password = static_cast<uint8_t>(enable_user_password ? 1 : 0);
    p.delete_user_password = static_cast<uint8_t>(delete_user_password ? 1 : 0);
    if (is_authorization_command_supported()){
      authorize_packet<stick10_08::WriteGeneralConfig, Authorize>(p, admin_temporary_password, device);
    } else {
      strcpyT(p.temporary_admin_password, admin_temporary_password);
    }
    stick10_08::WriteGeneralConfig::CommandTransaction::run(device, p);
}
std::vector<uint8_t> nitrokey::NitrokeyManager::read_config() {
    auto responsePayload = GetStatus::CommandTransaction::run(device);
    vector<uint8_t> v = vector<uint8_t>(responsePayload.data().general_config,
                                        responsePayload.data().general_config+sizeof(responsePayload.data().general_config));
    return v;
}
bool nitrokey::NitrokeyManager::is_320_OTP_secret_supported(){
  // 320 bit OTP secret is supported by version bigger or equal to:
    auto m = unordered_map<DeviceModel , int, EnumClassHash>({
                                           {DeviceModel::PRO, 8},
                                           {DeviceModel::LIBREM, 8},
                                           {DeviceModel::STORAGE, 54},
     });
    return get_minor_firmware_version() >= m[device->get_device_model()];
}
std::string nitrokey::NitrokeyManager::get_TOTP_code(uint8_t slot_number, const char *user_temporary_password) {
  return get_TOTP_code(slot_number, 0, 0, 0, user_temporary_password);
} /**
   * Returns ReadSlot structure, describing OTP slot configuration. Always
   * return binary counter - does the necessary conversion, if needed, to unify
   * the behavior across Pro and Storage.
   * @private For internal use only
   * @param slot_number which OTP slot to use (usual format)
   * @return ReadSlot structure
   */
nitrokey::proto::stick10::ReadSlot::ResponsePayload
nitrokey::NitrokeyManager::get_OTP_slot_data(const uint8_t slot_number) {
  auto p = get_payload<ReadSlot>();
  p.slot_number = slot_number;
  p.data_format = ReadSlot::CounterFormat::BINARY; // ignored for devices other than Storage v0.54+
  auto data = stick10::ReadSlot::CommandTransaction::run(device, p);

  auto &payload = data.data();

  // if fw <=v0.53 and asked binary - do the conversion from ASCII
  if (device->get_device_model() == DeviceModel::STORAGE && get_minor_firmware_version() <= 53
       && is_internal_hotp_slot_number(slot_number))
  {
    //convert counter from string to ull
    auto counter_s = string(payload.slot_counter_s, payload.slot_counter_s + sizeof(payload.slot_counter_s));
    payload.slot_counter = stoull(counter_s);
  }

  return payload;
}
  nitrokey::proto::stick10::ReadSlot::ResponsePayload nitrokey::NitrokeyManager::get_TOTP_slot_data(const uint8_t slot_number) {
    return get_OTP_slot_data(get_internal_slot_number_for_totp(slot_number));
  }
  nitrokey::proto::stick10::ReadSlot::ResponsePayload nitrokey::NitrokeyManager::get_HOTP_slot_data(const uint8_t slot_number) {
    return get_OTP_slot_data(get_internal_slot_number_for_hotp(slot_number));
  }