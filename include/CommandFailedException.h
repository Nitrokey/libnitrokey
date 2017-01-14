//
// Created by sz on 23.07.16.
//

#ifndef LIBNITROKEY_COMMANDFAILEDEXCEPTION_H
#define LIBNITROKEY_COMMANDFAILEDEXCEPTION_H

#include <exception>
#include <cstdint>
#include "log.h"

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
      return last_command_status == 3; //FIXME use enum status codes
    }

};


#endif //LIBNITROKEY_COMMANDFAILEDEXCEPTION_H
