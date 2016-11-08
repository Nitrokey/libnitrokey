//
// Created by sz on 08.11.16.
//

#ifndef LIBNITROKEY_STICK10_COMMANDS_0_8_H
#define LIBNITROKEY_STICK10_COMMANDS_0_8_H

#include <bitset>
#include <iomanip>
#include <string>
#include <sstream>
#include "inttypes.h"
#include "command.h"
#include "device_proto.h"

namespace nitrokey {
    namespace proto {

/*
 *	Stick10 protocol definition
 */
        namespace stick10_08 {

            class EraseSlot : Command<CommandID::ERASE_SLOT> {
            public:
                struct CommandPayload {
                    uint8_t slot_number;
                    uint8_t temporary_admin_password[25];

                    bool isValid() const { return !(slot_number & 0xF0); }
                    std::string dissect() const {
                      std::stringstream ss;
                      ss << "slot_number:\t" << (int)(slot_number) << std::endl;
                      return ss.str();
                    }
                } __packed;

                typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                    CommandTransaction;
            };

            class WriteToHOTPSlot : Command<CommandID::WRITE_TO_SLOT> {
                //admin auth
            public:
                struct CommandPayload {
                    uint8_t temporary_admin_password[25];
                    uint8_t slot_secret[20];
                    union {
                        uint8_t _slot_config;
                        struct {
                            bool use_8_digits   : 1;
                            bool use_enter      : 1;
                            bool use_tokenID    : 1;
                        };
                    };
                    union {
                        uint8_t slot_token_id[13]; /** OATH Token Identifier */
                        struct { /** @see https://openauthentication.org/token-specs/ */
                            uint8_t omp[2];
                            uint8_t tt[2];
                            uint8_t mui[8];
                            uint8_t keyboard_layout; //disabled feature in nitroapp as of 20160805
                        } slot_token_fields;
                    };

                    bool isValid() const { return true; }

                    std::string dissect() const {
                      std::stringstream ss;
                      ss << "temporary_admin_password:\t" << temporary_admin_password << std::endl;
                      ss << "slot_secret:" << std::endl
                         << ::nitrokey::misc::hexdump((const char *) (&slot_secret), sizeof slot_secret);
                      ss << "slot_config:\t" << std::bitset<8>((int) _slot_config) << std::endl;
                      ss << "\tuse_8_digits(0):\t" << use_8_digits << std::endl;
                      ss << "\tuse_enter(1):\t" << use_enter << std::endl;
                      ss << "\tuse_tokenID(2):\t" << use_tokenID << std::endl;

                      ss << "slot_token_id:\t";
                      for (auto i : slot_token_id)
                        ss << std::hex << std::setw(2) << std::setfill('0') << (int) i << " ";
                      ss << std::endl;

                      return ss.str();
                    }
                } __packed;

                typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                    CommandTransaction;
            };

            class WriteToHOTPSlot_2 : Command<CommandID::WRITE_TO_SLOT_2> {
            public:
                struct CommandPayload {
                    uint8_t temporary_admin_password[25];
                    uint8_t slot_number;
                    uint8_t slot_name[15];
                    union {
                        uint64_t slot_counter;
                        uint8_t slot_counter_s[8];
                    } __packed;

                    bool isValid() const { return !(slot_number & 0xF0); }

                    std::string dissect() const {
                      std::stringstream ss;
                      ss << "temporary_admin_password:\t" << temporary_admin_password << std::endl;
                      ss << "slot_number:\t" << (int) (slot_number) << std::endl;
                      ss << "slot_name:\t" << slot_name << std::endl;
                      ss << "slot_counter:\t[" << (int) slot_counter << "]\t"
                         << ::nitrokey::misc::hexdump((const char *) (&slot_counter), sizeof slot_counter, false);

                      return ss.str();
                    }
                } __packed;

                typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                    CommandTransaction;
            };


            class GetTOTP : Command<CommandID::GET_CODE> {
                //user auth
            public:
                struct CommandPayload {
                    uint8_t slot_number;
                    uint64_t challenge;
                    uint64_t last_totp_time;
                    uint8_t last_interval;
                    uint8_t user_temporary_password[25];

                    bool isValid() const { return !(slot_number & 0xF0); }
                    std::string dissect() const {
                      std::stringstream ss;
                      ss << "slot_number:\t" << (int)(slot_number) << std::endl;
                      ss << "challenge:\t" << (challenge) << std::endl;
                      ss << "last_totp_time:\t" << (last_totp_time) << std::endl;
                      ss << "last_interval:\t" << (int)(last_interval) << std::endl;
                      return ss.str();
                    }
                } __packed;

                struct ResponsePayload {
                    union {
                        uint8_t whole_response[18]; //14 bytes reserved for config, but used only 1
                        struct {
                            uint32_t code;
                            union{
                                uint8_t _slot_config;
                                struct{
                                    bool use_8_digits   : 1;
                                    bool use_enter      : 1;
                                    bool use_tokenID    : 1;
                                };
                            };
                        } __packed ;
                    } __packed ;

                    bool isValid() const { return true; }
                    std::string dissect() const {
                      std::stringstream ss;
                      ss << "code:\t" << (code) << std::endl;
                      ss << "slot_config:\t" << std::bitset<8>((int)_slot_config) << std::endl;
                      ss << "\tuse_8_digits(0):\t" << use_8_digits << std::endl;
                      ss << "\tuse_enter(1):\t" << use_enter << std::endl;
                      ss << "\tuse_tokenID(2):\t" << use_tokenID << std::endl;
                      return ss.str();
                    }
                } __packed;

                typedef Transaction<command_id(), struct CommandPayload, struct ResponsePayload>
                    CommandTransaction;
            };


        }
    }
}
#endif //LIBNITROKEY_STICK10_COMMANDS_0_8_H
