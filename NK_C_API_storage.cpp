#include "NK_C_API_storage.h"
#include "NK_C_API_helpers.h"

using namespace nitrokey;

#include "nk_strndup.h"


#ifdef __cplusplus
extern "C" {
#endif


// storage commands

NK_C_API int NK_send_startup(uint64_t seconds_from_epoch) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->send_startup(seconds_from_epoch);
  });
}

NK_C_API int NK_unlock_encrypted_volume(const char* user_pin) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->unlock_encrypted_volume(user_pin);
  });
}

NK_C_API int NK_lock_encrypted_volume() {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->lock_encrypted_volume();
  });
}

NK_C_API int NK_unlock_hidden_volume(const char* hidden_volume_password) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->unlock_hidden_volume(hidden_volume_password);
  });
}

NK_C_API int NK_lock_hidden_volume() {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->lock_hidden_volume();
  });
}

NK_C_API int NK_create_hidden_volume(uint8_t slot_nr, uint8_t start_percent, uint8_t end_percent,
                                     const char *hidden_volume_password) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->create_hidden_volume(slot_nr, start_percent, end_percent,
                            hidden_volume_password);
  });
}

NK_C_API int NK_set_unencrypted_read_only(const char *user_pin) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->set_unencrypted_read_only(user_pin);
  });
}

NK_C_API int NK_set_unencrypted_read_write(const char *user_pin) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->set_unencrypted_read_write(user_pin);
  });
}

NK_C_API int NK_set_unencrypted_read_only_admin(const char *admin_pin) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->set_unencrypted_read_only_admin(admin_pin);
  });
}

NK_C_API int NK_set_unencrypted_read_write_admin(const char *admin_pin) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->set_unencrypted_read_write_admin(admin_pin);
  });
}

NK_C_API int NK_set_encrypted_read_only(const char* admin_pin) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->set_encrypted_volume_read_only(admin_pin);
  });
}

NK_C_API int NK_set_encrypted_read_write(const char* admin_pin) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->set_encrypted_volume_read_write(admin_pin);
  });
}

NK_C_API int NK_export_firmware(const char* admin_pin) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->export_firmware(admin_pin);
  });
}

NK_C_API int NK_clear_new_sd_card_warning(const char* admin_pin) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->clear_new_sd_card_warning(admin_pin);
  });
}

NK_C_API int NK_fill_SD_card_with_random_data(const char* admin_pin) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->fill_SD_card_with_random_data(admin_pin);
  });
}

NK_C_API int NK_change_update_password(const char* current_update_password,
                                       const char* new_update_password) {
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->change_update_password(current_update_password, new_update_password);
  });
}

NK_C_API int NK_enable_firmware_update(const char* update_password){
  auto m = NitrokeyManager::instance();
  return get_without_result([&]() {
    m->enable_firmware_update(update_password);
  });
}

NK_C_API char* NK_get_status_storage_as_string() {
  auto m = NitrokeyManager::instance();
  return get_with_string_result([&]() {
    return m->get_status_storage_as_string();
  });
}

NK_C_API int NK_get_status_storage(NK_storage_status* out) {
  if (out == nullptr) {
    return -1;
  }
  auto m = NitrokeyManager::instance();
  auto result = get_with_status([&]() {
    return m->get_status_storage();
  }, proto::stick20::DeviceConfigurationResponsePacket::ResponsePayload());
  auto error_code = std::get<0>(result);
  if (error_code != 0) {
    return error_code;
  }

  auto status = std::get<1>(result);
  out->unencrypted_volume_read_only = status.ReadWriteFlagUncryptedVolume_u8 != 0;
  out->unencrypted_volume_active = status.VolumeActiceFlag_st.unencrypted;
  out->encrypted_volume_read_only = status.ReadWriteFlagCryptedVolume_u8 != 0;
  out->encrypted_volume_active = status.VolumeActiceFlag_st.encrypted;
  out->hidden_volume_read_only = status.ReadWriteFlagHiddenVolume_u8 != 0;
  out->hidden_volume_active = status.VolumeActiceFlag_st.hidden;
  out->firmware_version_major = status.versionInfo.major;
  out->firmware_version_minor = status.versionInfo.minor;
  out->firmware_locked = status.FirmwareLocked_u8 != 0;
  out->serial_number_sd_card = status.ActiveSD_CardID_u32;
  out->serial_number_smart_card = status.ActiveSmartCardID_u32;
  out->user_retry_count = status.UserPwRetryCount;
  out->admin_retry_count = status.AdminPwRetryCount;
  out->new_sd_card_found = status.NewSDCardFound_st.NewCard;
  out->filled_with_random = (status.SDFillWithRandomChars_u8 & 0x01) != 0;
  out->stick_initialized = status.StickKeysNotInitiated == 0;
  return 0;
}

NK_C_API int NK_get_storage_production_info(NK_storage_ProductionTest * out){
  if (out == nullptr) {
    return -1;
  }
  auto m = NitrokeyManager::instance();
  auto result = get_with_status([&]() {
    return m->production_info();
  }, proto::stick20::ProductionTest::ResponsePayload());

  auto error_code = std::get<0>(result);
  if (error_code != 0) {
    return error_code;
  }

  stick20::ProductionTest::ResponsePayload status = std::get<1>(result);
  // Cannot use memcpy without declaring C API struct packed
  // (which is not parsed by Python's CFFI apparently), hence the manual way.
#define a(x) out->x = status.x;
  a(FirmwareVersion_au8[0]);
  a(FirmwareVersion_au8[1]);
  a(FirmwareVersionInternal_u8);
  a(SD_Card_Size_u8);
  a(CPU_CardID_u32);
  a(SmartCardID_u32);
  a(SD_CardID_u32);
  a(SC_UserPwRetryCount);
  a(SC_AdminPwRetryCount);
  a(SD_Card_ManufacturingYear_u8);
  a(SD_Card_ManufacturingMonth_u8);
  a(SD_Card_OEM_u16);
  a(SD_WriteSpeed_u16);
  a(SD_Card_Manufacturer_u8);
#undef a
  return 0;
}

NK_C_API int NK_get_SD_usage_data(struct NK_SD_usage_data* out) {
  if (out == nullptr)
    return -1;
  auto m = NitrokeyManager::instance();
  auto result = get_with_status([&]() {
    return m->get_SD_usage_data();
  }, std::make_pair<uint8_t, uint8_t>(0, 0));
  auto error_code = std::get<0>(result);
  if (error_code != 0)
    return error_code;

  auto data = std::get<1>(result);
  out->write_level_min = std::get<0>(data);
  out->write_level_max = std::get<1>(data);

  return 0;
}

NK_C_API char* NK_get_SD_usage_data_as_string() {
  auto m = NitrokeyManager::instance();
  return get_with_string_result([&]() {
    return m->get_SD_usage_data_as_string();
  });
}

NK_C_API int NK_get_progress_bar_value() {
  auto m = NitrokeyManager::instance();
  return std::get<1>(get_with_status([&]() {
    return m->get_progress_bar_value();
  }, -2));
}

NK_C_API int NK_set_unencrypted_volume_rorw_pin_type_user() {
  auto m = NitrokeyManager::instance();
  return get_with_result([&]() {
    return m->set_unencrypted_volume_rorw_pin_type_user() ? 1 : 0;
  });
}


#ifdef __cplusplus
}
#endif
