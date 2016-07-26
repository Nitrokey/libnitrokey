#ifndef LIBNITROKEY_NITROKEYMANAGER_H
#define LIBNITROKEY_NITROKEYMANAGER_H

#include "device.h"
#include "log.h"
#include "device_proto.h"
#include "stick10_commands.h"

namespace nitrokey {
    using namespace nitrokey::device;
    using namespace std;
    using namespace nitrokey::proto::stick10;
    using namespace nitrokey::proto;
    using namespace nitrokey::log;

    class NitrokeyManager {
    public:
        static NitrokeyManager *instance();

        bool authorize(const char *pin, const char *temporary_password);
        bool write_HOTP_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint64_t hotp_counter,
                                     const char *temporary_password);
        bool write_TOTP_slot(uint8_t slot_number, const char *slot_name, const char *secret,
                             uint16_t time_window, bool use_8_digits, const char *temporary_password);
        uint32_t get_HOTP_code(uint8_t slot_number);
        uint32_t get_TOTP_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time,
                                       uint8_t last_interval);
        bool set_time(uint64_t time);
        bool get_time();
        bool erase_totp_slot(uint8_t slot_number);
        bool erase_hotp_slot(uint8_t slot_number);
        bool connect();
        bool disconnect();
        void set_debug(bool state);
        string get_status();

        const char * get_totp_slot_name(uint8_t slot_number);
        const char * get_hotp_slot_name(uint8_t slot_number);

        void change_user_PIN(char *current_PIN, char *new_PIN);
        void change_admin_PIN(char *current_PIN, char *new_PIN);

        void enable_password_safe(const char *user_pin);

        void get_password_safe_slot_status();

        uint8_t get_admin_retry_count();
        uint8_t get_user_retry_count();

        void lock_device();

        const char *get_password_safe_slot_name(uint8_t slot_number, const char *temporary_password);
        const char *get_password_safe_slot_password(uint8_t slot_number);
        const char *get_password_safe_slot_login(uint8_t slot_number);

    private:
        NitrokeyManager();
        ~NitrokeyManager();

        static NitrokeyManager *_instance;
        bool connected;
        Device *device;

        bool is_valid_hotp_slot_number(uint8_t slot_number) const;
        bool is_valid_totp_slot_number(uint8_t slot_number) const;
        uint8_t get_internal_slot_number_for_hotp(uint8_t slot_number) const;
        uint8_t get_internal_slot_number_for_totp(uint8_t slot_number) const;
        bool erase_slot(uint8_t slot_number);
        uint8_t *get_slot_name(uint8_t slot_number) const;
    };
}



#endif //LIBNITROKEY_NITROKEYMANAGER_H
