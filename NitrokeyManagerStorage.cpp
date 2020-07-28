#include "NitrokeyManagerStorage.h"

//using namespace nitrokey;
namespace nitrokey{

using nitrokey::misc::strcpyT;

//storage commands

void NitrokeyManager::send_startup(uint64_t seconds_from_epoch){
  auto p = get_payload<stick20::SendStartup>();
//      p.set_defaults(); //set current time
  p.localtime = seconds_from_epoch;
  stick20::SendStartup::CommandTransaction::run(device, p);
}

void NitrokeyManager::unlock_encrypted_volume(const char* user_pin){
  misc::execute_password_command<stick20::EnableEncryptedPartition>(device, user_pin);
}

void NitrokeyManager::unlock_hidden_volume(const char* hidden_volume_password) {
  misc::execute_password_command<stick20::EnableHiddenEncryptedPartition>(device, hidden_volume_password);
}

void NitrokeyManager::set_encrypted_volume_read_only(const char* admin_pin) {
  misc::execute_password_command<stick20::SetEncryptedVolumeReadOnly>(device, admin_pin);
}

void NitrokeyManager::set_encrypted_volume_read_write(const char* admin_pin) {
  misc::execute_password_command<stick20::SetEncryptedVolumeReadWrite>(device, admin_pin);
}

//TODO check is encrypted volume unlocked before execution
//if not return library exception
void NitrokeyManager::create_hidden_volume(uint8_t slot_nr, uint8_t start_percent, uint8_t end_percent,
                                           const char *hidden_volume_password) {
  auto p = get_payload<stick20::SetupHiddenVolume>();
  p.SlotNr_u8 = slot_nr;
  p.StartBlockPercent_u8 = start_percent;
  p.EndBlockPercent_u8 = end_percent;
  strcpyT(p.HiddenVolumePassword_au8, hidden_volume_password);
  stick20::SetupHiddenVolume::CommandTransaction::run(device, p);
}

void NitrokeyManager::set_unencrypted_read_only_admin(const char* admin_pin) {
  //from v0.49, v0.52+ it needs Admin PIN
  if (set_unencrypted_volume_rorw_pin_type_user()){
    LOG("set_unencrypted_read_only_admin is not supported for this version of Storage device. "
        "Please update firmware to v0.52+. Doing nothing.", nitrokey::log::Loglevel::WARNING);
    return;
  }
  misc::execute_password_command<stick20::SetUnencryptedVolumeReadOnlyAdmin>(device, admin_pin);
}

void NitrokeyManager::set_unencrypted_read_only(const char *user_pin) {
  //until v0.48 (incl. v0.50 and v0.51) User PIN was sufficient
  LOG("set_unencrypted_read_only is deprecated. Use set_unencrypted_read_only_admin instead.",
      nitrokey::log::Loglevel::WARNING);
  if (!set_unencrypted_volume_rorw_pin_type_user()){
    LOG("set_unencrypted_read_only is not supported for this version of Storage device. Doing nothing.",
        nitrokey::log::Loglevel::WARNING);
    return;
  }
  misc::execute_password_command<stick20::SendSetReadonlyToUncryptedVolume>(device, user_pin);
}

void NitrokeyManager::set_unencrypted_read_write_admin(const char* admin_pin) {
  //from v0.49, v0.52+ it needs Admin PIN
  if (set_unencrypted_volume_rorw_pin_type_user()){
    LOG("set_unencrypted_read_write_admin is not supported for this version of Storage device. "
        "Please update firmware to v0.52+. Doing nothing.", nitrokey::log::Loglevel::WARNING);
    return;
  }
  misc::execute_password_command<stick20::SetUnencryptedVolumeReadWriteAdmin>(device, admin_pin);
}

void NitrokeyManager::set_unencrypted_read_write(const char *user_pin) {
  //until v0.48 (incl. v0.50 and v0.51) User PIN was sufficient
  LOG("set_unencrypted_read_write is deprecated. Use set_unencrypted_read_write_admin instead.",
      nitrokey::log::Loglevel::WARNING);
  if (!set_unencrypted_volume_rorw_pin_type_user()){
    LOG("set_unencrypted_read_write is not supported for this version of Storage device. Doing nothing.",
        nitrokey::log::Loglevel::WARNING);
    return;
  }
  misc::execute_password_command<stick20::SendSetReadwriteToUncryptedVolume>(device, user_pin);
}

bool NitrokeyManager::set_unencrypted_volume_rorw_pin_type_user(){
  auto minor_firmware_version = get_minor_firmware_version();
  return minor_firmware_version <= 48 || minor_firmware_version == 50 || minor_firmware_version == 51;
}

void NitrokeyManager::export_firmware(const char* admin_pin) {
  misc::execute_password_command<stick20::ExportFirmware>(device, admin_pin);
}

void NitrokeyManager::enable_firmware_update(const char* firmware_pin) {
  misc::execute_password_command<stick20::EnableFirmwareUpdate>(device, firmware_pin);
}

void NitrokeyManager::clear_new_sd_card_warning(const char* admin_pin) {
  misc::execute_password_command<stick20::SendClearNewSdCardFound>(device, admin_pin);
}

void NitrokeyManager::fill_SD_card_with_random_data(const char* admin_pin) {
  auto p = get_payload<stick20::FillSDCardWithRandomChars>();
  p.set_defaults();
  strcpyT(p.admin_pin, admin_pin);
  stick20::FillSDCardWithRandomChars::CommandTransaction::run(device, p);
}

void NitrokeyManager::change_update_password(const char* current_update_password, const char* new_update_password) {
  auto p = get_payload<stick20::ChangeUpdatePassword>();
  strcpyT(p.current_update_password, current_update_password);
  strcpyT(p.new_update_password, new_update_password);
  stick20::ChangeUpdatePassword::CommandTransaction::run(device, p);
}

char * NitrokeyManager::get_status_storage_as_string(){
  auto p = stick20::GetDeviceStatus::CommandTransaction::run(device);
  return strndup(p.data().dissect().c_str(), max_string_field_length);
}

stick20::DeviceConfigurationResponsePacket::ResponsePayload NitrokeyManager::get_status_storage(){
  auto p = stick20::GetDeviceStatus::CommandTransaction::run(device);
  return p.data();
}

char * NitrokeyManager::get_SD_usage_data_as_string(){
  auto p = stick20::GetSDCardOccupancy::CommandTransaction::run(device);
  return strndup(p.data().dissect().c_str(), max_string_field_length);
}

std::pair<uint8_t,uint8_t> NitrokeyManager::get_SD_usage_data(){
  auto p = stick20::GetSDCardOccupancy::CommandTransaction::run(device);
  return std::make_pair(p.data().WriteLevelMin, p.data().WriteLevelMax);
}

int NitrokeyManager::get_progress_bar_value(){
  try{
    stick20::GetDeviceStatus::CommandTransaction::run(device);
    return -1;
  }
  catch (LongOperationInProgressException &e){
    return e.progress_bar_value;
  }
}


void NitrokeyManager::lock_encrypted_volume() {
  misc::execute_password_command<stick20::DisableEncryptedPartition>(device, "");
}

void NitrokeyManager::lock_hidden_volume() {
  misc::execute_password_command<stick20::DisableHiddenEncryptedPartition>(device, "");
}

uint8_t NitrokeyManager::get_SD_card_size() {
  auto data = stick20::ProductionTest::CommandTransaction::run(device);
  return data.data().SD_Card_Size_u8;
}


void NitrokeyManager::wink(){
  stick20::Wink::CommandTransaction::run(device);
};

stick20::ProductionTest::ResponsePayload NitrokeyManager::production_info(){
  auto data = stick20::ProductionTest::CommandTransaction::run(device);
  return data.data();
};

}