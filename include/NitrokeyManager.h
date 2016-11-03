#ifndef LIBNITROKEY_NITROKEYMANAGER_H
#define LIBNITROKEY_NITROKEYMANAGER_H

#include "device.h"
#include "log.h"
#include "device_proto.h"
#include "stick10_commands.h"
#include "stick20_commands.h"
#include <vector>
#include <memory>

namespace nitrokey {
    using namespace nitrokey::device;
    using namespace std;
    using namespace nitrokey::proto::stick10;
    using namespace nitrokey::proto::stick20;
    using namespace nitrokey::proto;
    using namespace nitrokey::log;

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
        uint32_t get_HOTP_code(uint8_t slot_number, const char *user_temporary_password);
        uint32_t get_TOTP_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time, uint8_t last_interval,
                               const char *user_temporary_password);
        bool set_time(uint64_t time);
        bool get_time();
        bool erase_totp_slot(uint8_t slot_number, const char *temporary_password);
        bool erase_hotp_slot(uint8_t slot_number, const char *temporary_password);
        bool connect(const char *device_model);
        bool connect();
        bool disconnect();
        void set_debug(bool state);
        string get_status();
        string get_serial_number();

        const char * get_totp_slot_name(uint8_t slot_number);
        const char * get_hotp_slot_name(uint8_t slot_number);

        void change_user_PIN(char *current_PIN, char *new_PIN);
        void change_admin_PIN(char *current_PIN, char *new_PIN);

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

        void unlock_hidden_volume(const char *hidden_volume_password);

        void set_unencrypted_read_only(const char *user_pin);

        void set_unencrypted_read_write(const char *user_pin);

        void export_firmware(const char *admin_pin);

        void clear_new_sd_card_warning(const char *admin_pin);

        void fill_SD_card_with_random_data(const char *admin_pin);

        void change_update_password(const char *current_update_password, const char *new_update_password);

        void create_hidden_volume(int slot_nr, int start_percent, int end_percent, const char *hidden_volume_password);

        void send_startup(uint64_t seconds_from_epoch);

        const char * get_status_storage();

        const char *get_SD_usage_data();

        int get_progress_bar_value();

        ~NitrokeyManager();
    private:
        NitrokeyManager();

        static shared_ptr <NitrokeyManager> _instance;
        bool connected;
        std::shared_ptr<Device> device;

        bool is_valid_hotp_slot_number(uint8_t slot_number) const;
        bool is_valid_totp_slot_number(uint8_t slot_number) const;
        bool is_valid_password_safe_slot_number(uint8_t slot_number) const;
        uint8_t get_internal_slot_number_for_hotp(uint8_t slot_number) const;
        uint8_t get_internal_slot_number_for_totp(uint8_t slot_number) const;
        bool erase_slot(uint8_t slot_number, const char *temporary_password);
        const char * get_slot_name(uint8_t slot_number);

        template <typename ProCommand, PasswordKind StoKind>
        void change_PIN_general(char *current_PIN, char *new_PIN);

    };
}



#endif //LIBNITROKEY_NITROKEYMANAGER_H
