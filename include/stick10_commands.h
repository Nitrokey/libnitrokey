#ifndef STICK10_COMMANDS_H
#define STICK10_COMMANDS_H
#include <string>
#include <sstream>
#include "inttypes.h"
#include "command.h"

namespace nitrokey {
namespace proto {

/*
 *	Stick10 protocol definition
 */
namespace stick10 {
class GetSlotName : public Command<CommandID::READ_SLOT_NAME> {
 public:
  // reachable as a typedef in Transaction
  struct CommandPayload {
    uint8_t slot_number;

    bool isValid() const { return !(slot_number & 0xF0); }
  } __packed;

  struct ResponsePayload {
    uint8_t slot_name[15];

    bool isValid() const { return true; }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload,
                      struct ResponsePayload> CommandTransaction;
};

class EraseSlot : Command<CommandID::ERASE_SLOT> {
 public:
  struct CommandPayload {
    uint8_t slot_number;

    bool isValid() const { return !(slot_number & 0xF0); }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class SetTime : Command<CommandID::SET_TIME> {
 public:
  struct CommandPayload {
    uint8_t reset;  // 0 - get time, 1 - set time
    uint64_t time;  // posix time

    bool isValid() const { return reset && reset != 1; }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

// TODO duplicate TOTP
class WriteToHOTPSlot : Command<CommandID::WRITE_TO_SLOT> {
 public:
  struct CommandPayload {
    uint8_t slot_number;
    uint8_t slot_name[15];
    uint8_t slot_secret[20];
    uint8_t slot_config;
    uint8_t slot_token_id[13];
    uint8_t slot_counter[8];

    bool isValid() const { return !(slot_number & 0xF0); }
    std::string dissect() const {
        std::stringstream ss;
        ss << "slot_number:\t" << (int)(slot_number) << std::endl;
        ss << "slot_name" << slot_name << std::endl;
        ss << "slot_secret" << slot_secret << std::endl;
        ss << "slot_config" << slot_config << std::endl;
        ss << "slot_token_id" << slot_token_id << std::endl;
        ss << "slot_counter" << slot_counter << std::endl;
        return ss.str();
    }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class WriteToTOTPSlot : Command<CommandID::WRITE_TO_SLOT> {
 public:
  struct CommandPayload {
    uint8_t slot_number;
    uint8_t slot_name[15];
    uint8_t slot_secret[20];
    uint8_t slot_config;
    uint8_t slot_token_id[13];
    uint16_t slot_interval;

    bool isValid() const { return !(slot_number & 0xF0); }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class GetCode : Command<CommandID::GET_CODE> {
 public:
  struct CommandPayload {
    uint8_t slot_number;
    uint64_t challenge;
    uint64_t last_totp_time;
    uint8_t last_interval;

    bool isValid() const { return !(slot_number & 0xF0); }
  } __packed;

  struct ResponsePayload {
    uint8_t code[18];

    bool isValid() const { return true; }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload,
                      struct ResponsePayload> CommandTransaction;
};

class GetHOTP : Command<CommandID::GET_CODE> {
 public:
  struct CommandPayload {
    uint8_t slot_number;

    bool isValid() const { return !(slot_number & 0xF0); }
    std::string dissect() const {
      std::stringstream ss;
      ss << "slot_number:\t" << (int)(slot_number) << std::endl;
      return ss.str();
    }
  } __packed;

  struct ResponsePayload {
    uint8_t code[18];

    bool isValid() const { return true; }
    std::string dissect() const {
      std::stringstream ss;
      ss << "code:\t" << (code) << std::endl;
      return ss.str();
    }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class ReadSlot : Command<CommandID::READ_SLOT> {
 public:
  struct CommandPayload {
    uint8_t slot_number;

    bool isValid() const { return !(slot_number & 0xF0); }

    std::string dissect() const {
      std::stringstream ss;
      ss << "slot_number:\t" << (int)(slot_number) << std::endl;
      return ss.str();
    }
  } __packed;

  struct ResponsePayload {
    uint8_t slot_name[15];
    uint8_t config;
    uint8_t token_id[13];
    uint64_t counter;

    bool isValid() const { return true; }

    std::string dissect() const {
      std::stringstream ss;
      ss << "slot_name:\t" << slot_name << std::endl;
      ss << "config:\t" << config << std::endl;
      ss << "token_id:\t" << token_id << std::endl;
      ss << "counter:\t" << counter << std::endl;
      return ss.str();
    }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload,
                      struct ResponsePayload> CommandTransaction;
};

class GetStatus : Command<CommandID::GET_STATUS> {
 public:
  struct ResponsePayload {
    uint16_t firmware_version;
    uint8_t card_serial[4];
    uint8_t general_config[3];
    uint8_t otp_password_config[2];

    bool isValid() const { return true; }

    std::string dissect() const {
      std::stringstream ss;
      ss << "firmware_version:\t" << firmware_version << std::endl;
      ss << "card_serial:\t"
         << ::nitrokey::misc::hexdump((const char *)(card_serial),
                                      sizeof card_serial);
      ss << "general_config:\t"
         << ::nitrokey::misc::hexdump((const char *)(general_config),
                                      sizeof general_config);
      ss << "otp_password_config:\t"
         << ::nitrokey::misc::hexdump((const char *)(otp_password_config),
                                      sizeof otp_password_config);
      return ss.str();
    }
  } __packed;

  typedef Transaction<command_id(), struct EmptyPayload, struct ResponsePayload>
      CommandTransaction;
};

class GetPasswordRetryCount : Command<CommandID::GET_PASSWORD_RETRY_COUNT> {
 public:
  struct ResponsePayload {
    uint8_t password_retry_count;

    bool isValid() const { return true; }
    std::string dissect() const {
      std::stringstream ss;
      ss << " password_retry_count\t" << password_retry_count << std::endl;
      return ss.str();
    }
  } __packed;

  typedef Transaction<command_id(), struct EmptyPayload, struct ResponsePayload>
      CommandTransaction;
};

class GetUserPasswordRetryCount
    : Command<CommandID::GET_USER_PASSWORD_RETRY_COUNT> {
 public:
  struct ResponsePayload {
    uint8_t password_retry_count;

    bool isValid() const { return true; }
    std::string dissect() const {
      std::stringstream ss;
      ss << " password_retry_count\t" << password_retry_count << std::endl;
      return ss.str();
    }
  } __packed;

  typedef Transaction<command_id(), struct EmptyPayload, struct ResponsePayload>
      CommandTransaction;
};

class GetPasswordSafeSlotStatus : Command<CommandID::GET_PW_SAFE_SLOT_STATUS> {
 public:
  struct ResponsePayload {
    uint8_t password_safe_status[PWS_SLOT_COUNT];

    bool isValid() const { return true; }
  } __packed;

  typedef Transaction<command_id(), struct EmptyPayload, struct ResponsePayload>
      CommandTransaction;
};

class GetPasswordSafeSlotName : Command<CommandID::GET_PW_SAFE_SLOT_NAME> {
 public:
  struct CommandPayload {
    uint8_t slot_number;

    bool isValid() const { return !(slot_number & 0xF0); }
    std::string dissect() const {
      std::stringstream ss;
      ss << "slot_number\t" << slot_number << std::endl;
      return ss.str();
    }
  } __packed;

  struct ResponsePayload {
    uint8_t slot_name[PWS_SLOTNAME_LENGTH];

    bool isValid() const { return true; }
    std::string dissect() const {
      std::stringstream ss;
      ss << " slot_name\t" << slot_name << std::endl;
      return ss.str();
    }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload,
                      struct ResponsePayload> CommandTransaction;
};

class GetPasswordSafeSlotPassword
    : Command<CommandID::GET_PW_SAFE_SLOT_PASSWORD> {
 public:
  struct CommandPayload {
    uint8_t slot_number;

    bool isValid() const { return !(slot_number & 0xF0); }
    std::string dissect() const {
      std::stringstream ss;
      ss << "   slot_number\t" << slot_number << std::endl;
      return ss.str();
    }
  } __packed;

  struct ResponsePayload {
    uint8_t slot_password[PWS_PASSWORD_LENGTH];

    bool isValid() const { return true; }
    std::string dissect() const {
      std::stringstream ss;
      ss << " slot_password\t" << slot_password << std::endl;
      return ss.str();
    }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload,
                      struct ResponsePayload> CommandTransaction;
};

class GetPasswordSafeSlotLogin
    : Command<CommandID::GET_PW_SAFE_SLOT_LOGINNAME> {
 public:
  struct CommandPayload {
    uint8_t slot_number;

    bool isValid() const { return !(slot_number & 0xF0); }
    std::string dissect() const {
      std::stringstream ss;
      ss << "   slot_number\t" << slot_number << std::endl;
      return ss.str();
    }
  } __packed;

  struct ResponsePayload {
    uint8_t slot_login[PWS_LOGINNAME_LENGTH];

    bool isValid() const { return true; }
    std::string dissect() const {
      std::stringstream ss;
      ss << " slot_login\t" << slot_login << std::endl;
      return ss.str();
    }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload,
                      struct ResponsePayload> CommandTransaction;
};

class SetPasswordSafeSlotData : Command<CommandID::SET_PW_SAFE_SLOT_DATA_1> {
 public:
  struct CommandPayload {
    uint8_t slot_number;
    uint8_t slot_name[PWS_SLOTNAME_LENGTH];
    uint8_t slot_password[PWS_PASSWORD_LENGTH];

    bool isValid() const { return !(slot_number & 0xF0); }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class SetPasswordSafeSlotData2 : Command<CommandID::SET_PW_SAFE_SLOT_DATA_2> {
 public:
  struct CommandPayload {
    uint8_t slot_number;
    uint8_t slot_name[PWS_SLOTNAME_LENGTH];

    bool isValid() const { return !(slot_number & 0xF0); }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class ErasePasswordSafeSlot : Command<CommandID::PW_SAFE_ERASE_SLOT> {
 public:
  struct CommandPayload {
    uint8_t slot_number;

    bool isValid() const { return !(slot_number & 0xF0); }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class EnablePasswordSafe : Command<CommandID::PW_SAFE_ENABLE> {
 public:
  struct CommandPayload {
    uint8_t password[30];

    bool isValid() const { return true; }
    std::string dissect() const {
      std::stringstream ss;
      ss << " password\t" << password << std::endl;
      return ss.str();
    }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class PasswordSafeInitKey : Command<CommandID::PW_SAFE_INIT_KEY> {
 public:
  typedef Transaction<command_id(), struct EmptyPayload, struct EmptyPayload>
      CommandTransaction;
};

// TODO naming screwed up, see above
class PasswordSafeSendSlotViaHID : Command<CommandID::PW_SAFE_SEND_DATA> {
 public:
  struct CommandPayload {
    uint8_t slot_number;
    uint8_t slot_kind;

    bool isValid() const { return !(slot_number & 0xF0); }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

// TODO "Device::passwordSafeSendSlotDataViaHID"

class WriteGeneralConfig : Command<CommandID::WRITE_CONFIG> {
 public:
  struct CommandPayload {
    uint8_t config[5];
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class FirstAuthenticate : Command<CommandID::FIRST_AUTHENTICATE> {
 public:
  struct CommandPayload {
    uint8_t card_password[25];
    uint8_t temporary_password[25];

    bool isValid() const { return true; }

    std::string dissect() const {
      std::stringstream ss;
      ss << "card_password:\t" << card_password << std::endl;
      ss << "temporary_password:\t" << temporary_password << std::endl;
      return ss.str();
    }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class UserAuthenticate : Command<CommandID::USER_AUTHENTICATE> {
 public:
  struct CommandPayload {
    uint8_t card_password[25];
    uint8_t temporary_password[25];

    bool isValid() const { return true; }
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class Authorize : Command<CommandID::AUTHORIZE> {
 public:
  struct CommandPayload {
    uint8_t crc[4];
    uint8_t password[25];
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class UserAuthorize : Command<CommandID::USER_AUTHORIZE> {
 public:
  struct CommandPayload {
    uint8_t crc[4];
    uint8_t password[25];
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class UnlockUserPassword : Command<CommandID::UNLOCK_USER_PASSWORD> {
 public:
  struct CommandPayload {
    uint8_t admin_password[20];  // TODO
  } __packed;

  // TODO could we get the stick to return the retry count?

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class ChangeUserPin : Command<CommandID::CHANGE_USER_PIN> {
 public:
  struct CommandPayload {
    uint8_t old_pin[25];
    uint8_t new_pin[25];
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

// TODO why is it needed?
class IsAESSupported : Command<CommandID::DETECT_SC_AES> {
 public:
  struct CommandPayload {
    uint8_t password[20];
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class ChangeAdminPin : Command<CommandID::CHANGE_ADMIN_PIN> {
 public:
  struct CommandPayload {
    uint8_t old_pin[25];
    uint8_t new_pin[25];
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class LockDevice : Command<CommandID::LOCK_DEVICE> {
 public:
  typedef Transaction<command_id(), struct EmptyPayload, struct EmptyPayload>
      CommandTransaction;
};

class FactoryReset : Command<CommandID::FACTORY_RESET> {
 public:
  struct CommandPayload {
    uint8_t password[20];
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};

class BuildAESKey : Command<CommandID::NEW_AES_KEY> {
 public:
  struct CommandPayload {
    uint8_t password[20];
  } __packed;

  typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
      CommandTransaction;
};
}
}
}
#endif
