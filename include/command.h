#ifndef COMMAND_H
#define COMMAND_H
#include <string>
#include "command_id.h"
#include "cxx_semantics.h"

namespace nitrokey {
    namespace proto {

        template<CommandID cmd_id>
        class Command : semantics::non_constructible {
        public:
            constexpr static CommandID command_id() { return cmd_id; }

            template<typename T>
            static std::string dissect(const T &) {
              return std::string("Payload dissection is unavailable");
            }
        };

#define print_to_ss(x) ( ss << " #x:\t" << (x) << std::endl );

        template<CommandID cmd_id>
        class PasswordCommand : public Command<cmd_id> {
        public:
            struct CommandPayload {
                uint8_t kind;
                uint8_t password[20];

                std::string dissect() const {
                  std::stringstream ss;
                  print_to_ss( kind );
                  print_to_ss(password);
                  return ss.str();
                }
                void set_kind_admin() {
                  kind = (uint8_t) 'A';
                }
                void set_kind_user() {
                  kind = (uint8_t) 'P';
                }

            } __packed;

            typedef Transaction<Command<cmd_id>::command_id(), struct CommandPayload, struct EmptyPayload>
                CommandTransaction;

        };
    }
}

#endif
