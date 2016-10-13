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
            ss << " old_pin:\t" <<  old_pin<< std::endl;
            return ss.str();
          }
          void set_kind(PasswordKind k){
            kind = (uint8_t)k;
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
              ss << " new_pin:\t" << new_pin<< std::endl;
              return ss.str();
            }
            void set_kind(PasswordKind k){
              kind = (uint8_t)k;
            }

        } __packed;

        typedef Transaction<command_id(), struct CommandPayload, struct EmptyPayload>
                CommandTransaction;
    };

class EnableEncryptedPartition : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t password[30];  // TODO check w/ firmware
  };

  typedef Transaction<CommandID::ENABLE_CRYPTED_PARI, struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class DisableEncryptedPartition : semantics::non_constructible {
 public:
  typedef Transaction<CommandID::DISABLE_CRYPTED_PARI, struct EmptyPayload,
                      struct EmptyPayload> CommandTransaction;
};

class EnableHiddenEncryptedPartition : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t password[30];  // TODO check w/ firmware
  };

  typedef Transaction<CommandID::ENABLE_HIDDEN_CRYPTED_PARI,
                      struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class DisableHiddenEncryptedPartition : semantics::non_constructible {
 public:
  typedef Transaction<CommandID::DISABLE_CRYPTED_PARI, struct EmptyPayload,
                      struct EmptyPayload> CommandTransaction;
};

class EnableFirmwareUpdate : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t password[30];  // TODO check w/ firmware
  };

  typedef Transaction<CommandID::ENABLE_FIRMWARE_UPDATE, struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class UpdatePassword : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t old_password[15];
    uint8_t new_password[15];
  };

  typedef Transaction<CommandID::CHANGE_UPDATE_PIN, struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class ExportFirmware : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t password[30];
  };

  typedef Transaction<CommandID::EXPORT_FIRMWARE_TO_FILE, struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class CreateNewKeys : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t password[30];
  };

  typedef Transaction<CommandID::GENERATE_NEW_KEYS, struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class FillSDCardWithRandomChars : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t volume_flag;
    uint8_t password[30];
  };

  typedef Transaction<CommandID::FILL_SD_CARD_WITH_RANDOM_CHARS,
                      struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class SetupHiddenVolume : semantics::non_constructible {
 public:
  typedef Transaction<CommandID::SEND_HIDDEN_VOLUME_SETUP, struct EmptyPayload,
                      struct EmptyPayload> CommandTransaction;
};

class SendPasswordMatrix : semantics::non_constructible {
 public:
  typedef Transaction<CommandID::SEND_PASSWORD_MATRIX, struct EmptyPayload,
                      struct EmptyPayload> CommandTransaction;
};

class SendPasswordMatrixPinData : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t pin_data[30];  // TODO how long actually can it be?
  };

  typedef Transaction<CommandID::SEND_PASSWORD_MATRIX_PINDATA,
                      struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class SendPasswordMatrixSetup : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t setup_data[30];  // TODO how long actually can it be?
  };

  typedef Transaction<CommandID::SEND_PASSWORD_MATRIX_SETUP,
                      struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

#define d(x) ss << " "#x":\t" << (int)x << std::endl;

    class GetDeviceStatus : Command<CommandID::GET_DEVICE_STATUS> {
    public:
        static const int OUTPUT_CMD_RESULT_STICK20_STATUS_START = 20 +1;
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
                 << ::nitrokey::misc::hexdump((const char *)(_padding),
                                              sizeof _padding);
              return ss.str();
            }
        } __packed;

        typedef Transaction<command_id(), struct EmptyPayload, struct ResponsePayload>
                CommandTransaction;
    };


class SendPassword : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t password[30];
  };

  typedef Transaction<CommandID::SEND_PASSWORD, struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class SendNewPassword : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t password[30];
  };

  typedef Transaction<CommandID::SEND_NEW_PASSWORD, struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

// TODO fix original nomenclature
class SendSetReadonlyToUncryptedVolume : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t password[30];
  };

  typedef Transaction<CommandID::ENABLE_READWRITE_UNCRYPTED_LUN,
                      struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class SendSetReadwriteToUncryptedVolume : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t password[30];
  };

  typedef Transaction<CommandID::ENABLE_READWRITE_UNCRYPTED_LUN,
                      struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class SendClearNewSdCardFound : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t password[30];
  };

  typedef Transaction<CommandID::CLEAR_NEW_SD_CARD_FOUND, struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class SendStartup : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint64_t localtime;  // POSIX
  };

  typedef Transaction<CommandID::SEND_STARTUP, struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class SendHiddenVolumeSetup : semantics::non_constructible {
 public:
  struct CommandPayload {
    // TODO HiddenVolumeSetup_tst type
  };

  typedef Transaction<CommandID::SEND_HIDDEN_VOLUME_SETUP,
                      struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class LockFirmware : semantics::non_constructible {
 public:
  struct CommandPayload {
    uint8_t password[30];
  };

  typedef Transaction<CommandID::SEND_LOCK_STICK_HARDWARE,
                      struct CommandPayload,
                      struct EmptyPayload> CommandTransaction;
};

class ProductionTest : semantics::non_constructible {
 public:
  typedef Transaction<CommandID::PRODUCTION_TEST, struct EmptyPayload,
                      struct EmptyPayload> CommandTransaction;
};
}
}
}

#endif
