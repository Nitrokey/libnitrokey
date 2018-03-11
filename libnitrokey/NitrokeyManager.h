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

#ifndef LIBNITROKEY_NITROKEYMANAGER_H
#define LIBNITROKEY_NITROKEYMANAGER_H

#include "device.h"
#include "log.h"
#include "device_proto.h"
#include "stick10_commands.h"
#include "stick10_commands_0.8.h"
#include "stick20_commands.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace nitrokey {
    using namespace nitrokey::device;
    using namespace std;
    using namespace nitrokey::proto::stick10;
    using namespace nitrokey::proto::stick20;
    using namespace nitrokey::proto;
    using namespace nitrokey::log;


#ifdef __WIN32
char * strndup(const char* str, size_t maxlen);
#endif

    class NitrokeyManager {
    public:
        static shared_ptr <NitrokeyManager> instance();

        bool first_authenticate(const char *pin, const char *temporary_password);
        bool write_HOTP_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint64_t hotp_counter,
                             bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                             const char *temporary_password);
        bool write_TOTP_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint16_t time_window,
                                     bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                                     const char *temporary_password);
        string get_HOTP_code(uint8_t slot_number, const char *user_temporary_password);
        string get_TOTP_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time,
                             uint8_t last_interval,
                             const char *user_temporary_password);
        string get_TOTP_code(uint8_t slot_number, const char *user_temporary_password);
        stick10::ReadSlot::ResponsePayload get_TOTP_slot_data(const uint8_t slot_number);
        stick10::ReadSlot::ResponsePayload get_HOTP_slot_data(const uint8_t slot_number);

        bool set_time(uint64_t time);
        bool get_time(uint64_t time = 0);
        bool erase_totp_slot(uint8_t slot_number, const char *temporary_password);
        bool erase_hotp_slot(uint8_t slot_number, const char *temporary_password);
        std::vector<std::string> list_devices();
        std::vector<std::string> list_devices_by_cpuID();

        /**
         * Connect to the device using unique smartcard:datacard id.
         * Needs list_device_by_cpuID() run first
         * @param id Current ID of the target device
         * @return true on success, false on failure
         */
        bool connect_with_ID(const std::string id);
        bool connect_with_path (std::string path);
        bool connect(const char *device_model);
        bool connect();
        bool disconnect();
        bool is_connected() throw() ;
        bool could_current_device_be_enumerated();
      bool set_default_commands_delay(int delay);

      DeviceModel get_connected_device_model() const;
          void set_debug(bool state);
        stick10::GetStatus::ResponsePayload get_status();
        string get_status_as_string();
        string get_serial_number();

        const char * get_totp_slot_name(uint8_t slot_number);
        const char * get_hotp_slot_name(uint8_t slot_number);

        void change_user_PIN(const char *current_PIN, const char *new_PIN);
        void change_admin_PIN(const char *current_PIN, const char *new_PIN);

        void enable_password_safe(const char *user_pin);

        vector <uint8_t> get_password_safe_slot_status();

        uint8_t get_admin_retry_count();
        uint8_t get_user_retry_count();

        void lock_device();

        const char *get_password_safe_slot_name(uint8_t slot_number);
        const char *get_password_safe_slot_password(uint8_t slot_number);
        const char *get_password_safe_slot_login(uint8_t slot_number);

        void
    write_password_safe_slot(uint8_t slot_number, const char *slot_name, const char *slot_login,
                                 const char *slot_password);

        void erase_password_safe_slot(uint8_t slot_number);

        void user_authenticate(const char *user_password, const char *temporary_password);

        void factory_reset(const char *admin_password);

        void build_aes_key(const char *admin_password);

        void unlock_user_password(const char *admin_password, const char *new_user_password);

        void write_config(uint8_t numlock, uint8_t capslock, uint8_t scrolllock, bool enable_user_password,
                          bool delete_user_password, const char *admin_temporary_password);

        vector<uint8_t> read_config();

        bool is_AES_supported(const char *user_password);

        void unlock_encrypted_volume(const char *user_password);
        void lock_encrypted_volume();

        void unlock_hidden_volume(const char *hidden_volume_password);
        void lock_hidden_volume();

        /**
         * Sets unencrypted volume read-only.
         * Works until v0.48 (incl. v0.50), where User PIN was sufficient
         * Does nothing otherwise.
         * @param user_pin User PIN
         */
        void set_unencrypted_read_only(const char *user_pin);

        /**
         * Sets unencrypted volume read-only.
         * Works from v0.49 (except v0.50) accepts Admin PIN
         * Does nothing otherwise.
         * @param admin_pin Admin PIN
         */
        void set_unencrypted_read_only_admin(const char *admin_pin);

        /**
         * Sets unencrypted volume read-write.
         * Works until v0.48 (incl. v0.50), where User PIN was sufficient
         * Does nothing otherwise.
         * @param user_pin User PIN
         */
        void set_unencrypted_read_write(const char *user_pin);

        /**
         * Sets unencrypted volume read-write.
         * Works from v0.49 (except v0.50) accepts Admin PIN
         * Does nothing otherwise.
         * @param admin_pin Admin PIN
         */
        void set_unencrypted_read_write_admin(const char *admin_pin);

        void export_firmware(const char *admin_pin);
        void enable_firmware_update(const char *firmware_pin);

        void clear_new_sd_card_warning(const char *admin_pin);

        void fill_SD_card_with_random_data(const char *admin_pin);

        uint8_t get_SD_card_size();

        void change_update_password(const char *current_update_password, const char *new_update_password);

        void create_hidden_volume(uint8_t slot_nr, uint8_t start_percent, uint8_t end_percent,
                                  const char *hidden_volume_password);

        void send_startup(uint64_t seconds_from_epoch);

        const char * get_status_storage_as_string();
        stick20::DeviceConfigurationResponsePacket::ResponsePayload get_status_storage();

        const char *get_SD_usage_data_as_string();
        std::pair<uint8_t,uint8_t> get_SD_usage_data();


        int get_progress_bar_value();

        ~NitrokeyManager();
        bool is_authorization_command_supported();
        bool is_320_OTP_secret_supported();


      template <typename S, typename A, typename T>
        void authorize_packet(T &package, const char *admin_temporary_password, shared_ptr<Device> device);
        int get_minor_firmware_version();

        explicit NitrokeyManager();
        void set_log_function(std::function<void(std::string)> log_function);
    private:

        static shared_ptr <NitrokeyManager> _instance;
        std::shared_ptr<Device> device;
        std::string current_device_id;
    public:
        const string get_current_device_id() const;

    private:
        std::unordered_map<std::string, shared_ptr<Device> > connected_devices;
        std::unordered_map<std::string, shared_ptr<Device> > connected_devices_byID;


        stick10::ReadSlot::ResponsePayload get_OTP_slot_data(const uint8_t slot_number);
      bool is_valid_hotp_slot_number(uint8_t slot_number) const;
        bool is_valid_totp_slot_number(uint8_t slot_number) const;
        bool is_valid_password_safe_slot_number(uint8_t slot_number) const;
        uint8_t get_internal_slot_number_for_hotp(uint8_t slot_number) const;
        uint8_t get_internal_slot_number_for_totp(uint8_t slot_number) const;
        bool erase_slot(uint8_t slot_number, const char *temporary_password);
        const char * get_slot_name(uint8_t slot_number);

        template <typename ProCommand, PasswordKind StoKind>
        void change_PIN_general(const char *current_PIN, const char *new_PIN);

        void write_HOTP_slot_authorize(uint8_t slot_number, const char *slot_name, const char *secret, uint64_t hotp_counter,
                                   bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                                   const char *temporary_password);

        void write_TOTP_slot_authorize(uint8_t slot_number, const char *slot_name, const char *secret, uint16_t time_window,
                                   bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                                   const char *temporary_password);

        void write_OTP_slot_no_authorize(uint8_t internal_slot_number, const char *slot_name, const char *secret,
                                         uint64_t counter_or_interval,
                                         bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                                         const char *temporary_password) const;
      bool _disconnect_no_lock();

    public:
      bool set_current_device_speed(int retry_delay, int send_receive_delay);
      void set_loglevel(Loglevel loglevel);

      void set_loglevel(int loglevel);

      /**
       * Sets encrypted volume read-only.
       * Supported from future versions of Storage.
       * @param admin_pin Admin PIN
       */
      void set_encrypted_volume_read_only(const char *admin_pin);

      /**
       * Sets encrypted volume read-write.
       * Supported from future versions of Storage.
       * @param admin_pin Admin PIN
       */
      void set_encrypted_volume_read_write(const char *admin_pin);

      int get_major_firmware_version();

      bool is_smartcard_in_use();

      /**
       * Function to determine unencrypted volume PIN type
       * @param minor_firmware_version
       * @return Returns true, if set unencrypted volume ro/rw pin type is User, false otherwise.
       */
      bool set_unencrypted_volume_rorw_pin_type_user();
    };
}



#endif //LIBNITROKEY_NITROKEYMANAGER_H
