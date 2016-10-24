#ifndef STICK20_COMMANDS_H
#define STICK20_COMMANDS_H

#include "inttypes.h"
#include "command.h"
#include <string>
#include <sstream>
#include "device_proto.h"


namespace nitrokey {
    namespace proto {

/*
*	STICK20 protocol command ids
*	a superset (almost) of STICK10
*/
#define print_to_ss(x) ( ss << " #x:\t" << (x) << std::endl );
        namespace stick20 {

            enum class PasswordKind : uint8_t {
                User = 'P',
                Admin = 'A'
            };

            class ChangeAdminUserPin20Current : Command<CommandID::STICK20_CMD_SEND_PASSWORD> {
            public:
                struct CommandPayload {
                    uint8_t kind;
                    uint8_t old_pin[20];

                    std::string dissect() const {
                      std::stringstream ss;
                      print_to_ss( kind );
                      ss << " old_pin:\t" << old_pin << std::endl;
                      return ss.str();
                    }

                    void set_kind(PasswordKind k) {
                      kind = (uint8_t) k;
                    }
                } __packed;

                typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                    CommandTransaction;
            };


            class ChangeAdminUserPin20New : Command<CommandID::STICK20_CMD_SEND_NEW_PASSWORD> {
            public:

                struct CommandPayload {
                    uint8_t kind;
                    uint8_t new_pin[20];

                    std::string dissect() const {
                      std::stringstream ss;
                      print_to_ss( kind );
                      ss << " new_pin:\t" << new_pin << std::endl;
                      return ss.str();
                    }

                    void set_kind(PasswordKind k) {
                      kind = (uint8_t) k;
                    }

                } __packed;

                typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                    CommandTransaction;
            };


            class UnlockUserPassword : Command<CommandID::UNLOCK_USER_PASSWORD> {
            public:
                struct CommandPayload {
                    uint8_t kind;
                    uint8_t user_new_password[20];

                    std::string dissect() const {
                      std::stringstream ss;
                      print_to_ss( kind );
                      ss << " user_new_password:\t" << user_new_password << std::endl;
                      return ss.str();
                    }

                    void set_kind(PasswordKind k) {
                      kind = (uint8_t) k;
                    }
                } __packed;

                typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                    CommandTransaction;
            };

            class EnableEncryptedPartition : public PasswordCommand<CommandID::ENABLE_CRYPTED_PARI> {};
            class DisableEncryptedPartition : public PasswordCommand<CommandID::DISABLE_CRYPTED_PARI> {};
            class EnableHiddenEncryptedPartition : public PasswordCommand<CommandID::ENABLE_HIDDEN_CRYPTED_PARI> {};
            class DisableHiddenEncryptedPartition : public PasswordCommand<CommandID::DISABLE_CRYPTED_PARI> {};
            class EnableFirmwareUpdate : public PasswordCommand<CommandID::ENABLE_FIRMWARE_UPDATE> {};

            class UpdatePassword : Command<CommandID::CHANGE_UPDATE_PIN> {
            public:
                struct CommandPayload {
                    uint8_t old_password[15];
                    uint8_t new_password[15];
                    std::string dissect() const {
                      std::stringstream ss;
                      print_to_ss( old_password );
                      print_to_ss( new_password );
                      return ss.str();
                    }
                };

                typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                    CommandTransaction;
            };

            class ExportFirmware : public PasswordCommand<CommandID::EXPORT_FIRMWARE_TO_FILE> {};

            class CreateNewKeys : Command<CommandID::GENERATE_NEW_KEYS> {
            public:
                struct CommandPayload {
                    uint8_t kind;
                    uint8_t admin_password[30]; //CS20_MAX_PASSWORD_LEN
                    std::string dissect() const {
                      std::stringstream ss;
                      print_to_ss( kind );
                      ss << " admin_password:\t" << admin_password << std::endl;
                      return ss.str();
                    }

                    void setKindPrefixed() {
                      kind = 'P';
                    }
                } __packed;

                typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                    CommandTransaction;
            };

            class FillSDCardWithRandomChars : public PasswordCommand<CommandID::FILL_SD_CARD_WITH_RANDOM_CHARS> {};

            class SetupHiddenVolume : Command<CommandID::SEND_HIDDEN_VOLUME_SETUP> {
            public:
                typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                    CommandTransaction;
            };

#define d(x) ss << " "#x":\t" << (int)x << std::endl;

            class GetDeviceStatus : Command<CommandID::GET_DEVICE_STATUS> {
            public:
                static const int OUTPUT_CMD_RESULT_STICK20_STATUS_START = 20 + 1;
                static const int payload_absolute_begin = 8;
                static const int padding_size = OUTPUT_CMD_RESULT_STICK20_STATUS_START - payload_absolute_begin;

                struct ResponsePayload {
                    uint8_t _padding[padding_size]; //TODO confirm padding in Storage firmware
                    //data starts from 21st byte of packet -> 13th byte of payload
                    uint8_t command_counter;
                    uint8_t last_command;
                    uint8_t status;
                    uint8_t progress_bar_value;

                    bool isValid() const { return true; }

                    std::string dissect() const {
                      std::stringstream ss;
                      d(command_counter);
                      d(last_command);
                      d(status);
                      d(progress_bar_value);
                      ss << "_padding:\t"
                         << ::nitrokey::misc::hexdump((const char *) (_padding),
                                                      sizeof _padding);
                      return ss.str();
                    }
                } __packed;

                typedef Transaction<command_id(), struct EmptyPayload, struct ResponsePayload>
                    CommandTransaction;
            };

#undef d


// TODO fix original nomenclature
            class SendSetReadonlyToUncryptedVolume : public PasswordCommand<CommandID::ENABLE_READONLY_UNCRYPTED_LUN> {};
            class SendSetReadwriteToUncryptedVolume : public PasswordCommand<CommandID::ENABLE_READWRITE_UNCRYPTED_LUN> {};
            class SendClearNewSdCardFound : public PasswordCommand<CommandID::CLEAR_NEW_SD_CARD_FOUND> {};

            class SendStartup : Command<CommandID::SEND_STARTUP> {
            public:
                struct CommandPayload {
                    uint64_t localtime;  // POSIX
                    std::string dissect() const {
                      std::stringstream ss;
//                      ss << " admin_password:\t" << admin_password << std::endl;
                      return ss.str();
                    }
                };

                typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                    CommandTransaction;
            };

            class SendHiddenVolumeSetup : Command<CommandID::SEND_HIDDEN_VOLUME_SETUP> {
            public:
                struct CommandPayload {
                    // TODO HiddenVolumeSetup_tst type
                    std::string dissect() const {
                      std::stringstream ss;
//                      ss << " admin_password:\t" << admin_password << std::endl;
                      return ss.str();
                    }
                };

                typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                    CommandTransaction;
            };

            class LockFirmware : public PasswordCommand<CommandID::SEND_LOCK_STICK_HARDWARE> {};

            class ProductionTest : Command<CommandID::PRODUCTION_TEST> {
            public:
                typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                    CommandTransaction;
            };
        }
    }
}

#undef print_to_ss

#endif
