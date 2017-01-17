//
// Created by sz on 23.07.16.
//

#ifndef LIBNITROKEY_COMMANDFAILEDEXCEPTION_H
#define LIBNITROKEY_COMMANDFAILEDEXCEPTION_H

#include <exception>
#include <cstdint>
#include "log.h"
#include "command_id.h"

using cs = nitrokey::proto::stick10::command_status;

class CommandFailedException : public std::exception {
public:
    const uint8_t last_command_code;
    const uint8_t last_command_status;

    CommandFailedException(uint8_t last_command_code, uint8_t last_command_status) :
            last_command_code(last_command_code),
            last_command_status(last_command_status){
      nitrokey::log::Log::instance()(std::string("CommandFailedException, status: ")+ std::to_string(last_command_status), nitrokey::log::Loglevel::DEBUG);
    }

    virtual const char *what() const throw() {
        return "Command execution has failed on device";
    }

    bool reason_slot_not_programmed() const throw(){
      return last_command_status == static_cast<uint8_t>(cs::slot_not_programmed);
    }

    bool reason_wrong_password() const throw(){
      return last_command_status == static_cast<uint8_t>(cs::wrong_password);
    }

};


#endif //LIBNITROKEY_COMMANDFAILEDEXCEPTION_H
