/*
 * Copyright (c) 2015-2018 Nitrokey UG
 *
 * This file is part of libnitrokey.
 *
 * libnitrokey is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * libnitrokey is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libnitrokey. If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0
 */

#include "NK_C_API.h"
#include <iostream>
#include <tuple>
#include "libnitrokey/NitrokeyManager.h"
#include <cstring>
#include "libnitrokey/LibraryException.h"
#include "libnitrokey/cxx_semantics.h"
#include "libnitrokey/stick20_commands.h"
#include "libnitrokey/device_proto.h"
#include "libnitrokey/version.h"


#include "nk_strndup.h"

using namespace nitrokey;

const uint8_t NK_PWS_SLOT_COUNT = PWS_SLOT_COUNT;
uint8_t NK_last_command_status = 0;

#include "NK_C_API_helpers.h"
#include "NitrokeyManagerOTP.h"
#include "NitrokeyManagerPWS.h"

#ifdef __cplusplus
extern "C" {
#endif

	NK_C_API uint8_t NK_get_last_command_status() {
		auto _copy = NK_last_command_status;
		NK_last_command_status = 0;
		return _copy;
	}

	NK_C_API int NK_login(const char *device_model) {
		auto m = NitrokeyManager::instance();
		try {
			NK_last_command_status = 0;
			return m->connect(device_model);
		}
		catch (CommandFailedException & commandFailedException) {
			NK_last_command_status = commandFailedException.last_command_status;
			return commandFailedException.last_command_status;
		}
    catch (const DeviceCommunicationException &deviceException){
      NK_last_command_status = 256-deviceException.getType();
      cerr << deviceException.what() << endl;
      return 0;
    }
		catch (std::runtime_error &e) {
			cerr << e.what() << endl;
			return 0;
		}
		return 0;
	}

        NK_C_API int NK_login_enum(NK_device_model device_model) {
                const char *model_string;
                switch (device_model) {
                    case NK_PRO:
                        model_string = "P";
                        break;
                    case NK_STORAGE:
                        model_string = "S";
                        break;
                    case NK_LIBREM:
                        model_string = "L";
                        break;
                    case NK_DISCONNECTED:
                    default:
                        /* no such enum value -- return error code */
                        return 0;
                }
                return NK_login(model_string);
        }

	NK_C_API int NK_logout() {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			m->disconnect();
		});
	}

	NK_C_API int NK_first_authenticate(const char* admin_password, const char* admin_temporary_password) {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			return m->first_authenticate(admin_password, admin_temporary_password);
		});
	}


	NK_C_API int NK_user_authenticate(const char* user_password, const char* user_temporary_password) {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			m->user_authenticate(user_password, user_temporary_password);
		});
	}

	NK_C_API int NK_factory_reset(const char* admin_password) {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			m->factory_reset(admin_password);
		});
	}
	NK_C_API int NK_build_aes_key(const char* admin_password) {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			m->build_aes_key(admin_password);
		});
	}

	NK_C_API int NK_unlock_user_password(const char *admin_password, const char *new_user_password) {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			m->unlock_user_password(admin_password, new_user_password);
		});
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


	NK_C_API enum NK_device_model NK_get_device_model() {
		auto m = NitrokeyManager::instance();
		try {
			auto model = m->get_connected_device_model();
			switch (model) {
				case DeviceModel::PRO:
				    return NK_PRO;
				case DeviceModel::STORAGE:
				    return NK_STORAGE;
				case DeviceModel::LIBREM:
				    return NK_LIBREM;
				default:
				    /* unknown or not connected device */
				    return NK_device_model::NK_DISCONNECTED;
			}
		} catch (const DeviceNotConnected& e) {
			return NK_device_model::NK_DISCONNECTED;
		}
}


	void clear_string(std::string &s) {
		std::fill(s.begin(), s.end(), ' ');
	}


	NK_C_API char * NK_status() {
		return NK_get_status_as_string();
	}

	NK_C_API char * NK_get_status_as_string() {
		auto m = NitrokeyManager::instance();
		return get_with_string_result([&]() {
			string && s = m->get_status_as_string();
			char * rs = strndup(s.c_str(), MAXIMUM_STR_REPLY_LENGTH);
			clear_string(s);
			return rs;
		});
	}

	NK_C_API int NK_get_status(struct NK_status* out) {
		if (out == nullptr) {
			return -1;
		}
		auto m = NitrokeyManager::instance();
		auto result = get_with_status([&]() {
			return m->get_status();
		}, proto::stick10::GetStatus::ResponsePayload());
		auto error_code = std::get<0>(result);
		if (error_code != 0) {
			return error_code;
		}

		auto status = std::get<1>(result);
		out->firmware_version_major = status.firmware_version_st.major;
		out->firmware_version_minor = status.firmware_version_st.minor;
		out->serial_number_smart_card = status.card_serial_u32;
		out->config_numlock = status.numlock;
		out->config_capslock = status.capslock;
		out->config_scrolllock = status.scrolllock;
		out->otp_user_password = status.enable_user_password != 0;
		return 0;
	}

	NK_C_API char * NK_device_serial_number() {
		auto m = NitrokeyManager::instance();
		return get_with_string_result([&]() {
			string && s = m->get_serial_number();
			char * rs = strndup(s.c_str(), max_string_field_length);
			clear_string(s);
			return rs;
		});
	}

	NK_C_API uint32_t NK_device_serial_number_as_u32() {
		auto m = NitrokeyManager::instance();
		return get_with_result([&]() {
                        return m->get_serial_number_as_u32();
		});
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

	NK_C_API void NK_set_debug(bool state) {
		auto m = NitrokeyManager::instance();
		m->set_debug(state);
	}


	NK_C_API void NK_set_debug_level(const int level) {
		auto m = NitrokeyManager::instance();
		m->set_loglevel(level);
	}

	NK_C_API unsigned int NK_get_major_library_version() {
		return get_major_library_version();
	}

	NK_C_API unsigned int NK_get_minor_library_version() {
		return get_minor_library_version();
	}

	NK_C_API const char* NK_get_library_version() {
		return get_library_version();
	}

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

	NK_C_API int NK_change_admin_PIN(const char *current_PIN, const char *new_PIN) {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			m->change_admin_PIN(current_PIN, new_PIN);
		});
	}

	NK_C_API int NK_change_user_PIN(const char *current_PIN, const char *new_PIN) {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			m->change_user_PIN(current_PIN, new_PIN);
		});
	}

	NK_C_API int NK_enable_password_safe(const char *user_pin) {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			m->enable_password_safe(user_pin);
		});
	}
	NK_C_API uint8_t * NK_get_password_safe_slot_status() {
		auto m = NitrokeyManager::instance();
		return get_with_array_result([&]() {
			auto slot_status = m->get_password_safe_slot_status();
			return duplicate_vector_and_clear(slot_status);
		});

	}

        NK_C_API void NK_free_password_safe_slot_status(uint8_t* status) {
                delete[] status;
        }

	NK_C_API uint8_t NK_get_user_retry_count() {
		auto m = NitrokeyManager::instance();
		return get_with_result([&]() {
			return m->get_user_retry_count();
		});
	}

	NK_C_API uint8_t NK_get_admin_retry_count() {
		auto m = NitrokeyManager::instance();
		return get_with_result([&]() {
			return m->get_admin_retry_count();
		});
	}

	NK_C_API int NK_lock_device() {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			m->lock_device();
		});
	}

	NK_C_API char *NK_get_password_safe_slot_name(uint8_t slot_number) {
		auto m = NitrokeyManager::instance();
		return get_with_string_result([&]() {
			return m->get_password_safe_slot_name(slot_number);
		});
	}

	NK_C_API char *NK_get_password_safe_slot_login(uint8_t slot_number) {
		auto m = NitrokeyManager::instance();
		return get_with_string_result([&]() {
			return m->get_password_safe_slot_login(slot_number);
		});
	}
	NK_C_API char *NK_get_password_safe_slot_password(uint8_t slot_number) {
		auto m = NitrokeyManager::instance();
		return get_with_string_result([&]() {
			return m->get_password_safe_slot_password(slot_number);
		});
	}
	NK_C_API int NK_write_password_safe_slot(uint8_t slot_number, const char *slot_name, const char *slot_login,
		const char *slot_password) {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			m->write_password_safe_slot(slot_number, slot_name, slot_login, slot_password);
		});
	}

	NK_C_API int NK_erase_password_safe_slot(uint8_t slot_number) {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			m->erase_password_safe_slot(slot_number);
		});
	}

	NK_C_API int NK_is_AES_supported(const char *user_password) {
		auto m = NitrokeyManager::instance();
		return get_with_result([&]() {
			return (uint8_t)m->is_AES_supported(user_password);
		});
	}

	NK_C_API int NK_login_auto() {
		auto m = NitrokeyManager::instance();
		return get_with_result([&]() {
			return (uint8_t)m->connect();
		});
	}

	NK_C_API uint8_t NK_get_major_firmware_version() {
		auto m = NitrokeyManager::instance();
		return get_with_result([&]() {
			return m->get_major_firmware_version();
		});
	}

  NK_C_API uint8_t NK_get_minor_firmware_version() {
		auto m = NitrokeyManager::instance();
		return get_with_result([&]() {
			return m->get_minor_firmware_version();
		});
	}


	NK_C_API char* NK_list_devices_by_cpuID() {
		auto nm = NitrokeyManager::instance();
		return get_with_string_result([&]() {
			auto v = nm->list_devices_by_cpuID();
			std::string res;
			for (const auto a : v){
				res += a+";";
			}
			if (res.size()>0) res.pop_back(); // remove last delimiter char
			return strndup(res.c_str(), MAXIMUM_STR_REPLY_LENGTH);
		});
	}

	bool copy_device_info(const DeviceInfo& source, NK_device_info* target) {
		switch (source.m_deviceModel) {
		case DeviceModel::PRO:
			target->model = NK_PRO;
			break;
		case DeviceModel::STORAGE:
			target->model = NK_STORAGE;
			break;
		case DeviceModel::LIBREM:
			target->model = NK_LIBREM;
			break;
		default:
			return false;
		}

		target->path = strndup(source.m_path.c_str(), MAXIMUM_STR_REPLY_LENGTH);
		target->serial_number = strndup(source.m_serialNumber.c_str(), MAXIMUM_STR_REPLY_LENGTH);
		target->next = nullptr;

		return target->path && target->serial_number;
	}

	NK_C_API struct NK_device_info* NK_list_devices() {
		auto nm = NitrokeyManager::instance();
		return get_with_result([&]() -> NK_device_info* {
			auto v = nm->list_devices();
			if (v.empty())
				return nullptr;

			auto result = new NK_device_info();
			auto ptr = result;
			auto first = v.begin();
			if (!copy_device_info(*first, ptr)) {
				NK_free_device_info(result);
				return nullptr;
			}
			v.erase(first);

			for (auto& info : v) {
				ptr->next = new NK_device_info();
				ptr = ptr->next;

				if (!copy_device_info(info, ptr)) {
					NK_free_device_info(result);
					return nullptr;
				}
			}
			return result;
		});
	}

	NK_C_API void NK_free_device_info(struct NK_device_info* device_info) {
		if (!device_info)
			return;

		if (device_info->next)
			NK_free_device_info(device_info->next);

		free(device_info->path);
		free(device_info->serial_number);
		delete device_info;
	}

	NK_C_API int NK_connect_with_ID(const char* id) {
		auto m = NitrokeyManager::instance();
		return get_with_result([&]() {
			return m->connect_with_ID(id) ? 1 : 0;
		});
	}

	NK_C_API int NK_connect_with_path(const char* path) {
		auto m = NitrokeyManager::instance();
		return get_with_result([&]() {
			return m->connect_with_path(path) ? 1 : 0;
		});
	 }


	NK_C_API int NK_wink() {
		auto m = NitrokeyManager::instance();
		return get_without_result([&]() {
			return m->wink();
		});
	}

  NK_C_API int NK_enable_firmware_update_pro(const char* update_password){
    auto m = NitrokeyManager::instance();
    return get_without_result([&]() {
      m->enable_firmware_update_pro(update_password);
  });
}

  NK_C_API int NK_change_firmware_password_pro(const char *current_firmware_password, const char *new_firmware_password) {
    auto m = NitrokeyManager::instance();
    return get_without_result([&]() {
      m->change_firmware_update_password_pro(current_firmware_password,
                                             new_firmware_password);
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


#ifdef __cplusplus
}
#endif
