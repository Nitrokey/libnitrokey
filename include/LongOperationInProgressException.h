//
// Created by sz on 24.10.16.
//

#ifndef LIBNITROKEY_LONGOPERATIONINPROGRESSEXCEPTION_H
#define LIBNITROKEY_LONGOPERATIONINPROGRESSEXCEPTION_H


class LongOperationInProgressException : public std::exception {

public:
    unsigned char progress_bar_value;
    unsigned char command_id;

    LongOperationInProgressException(unsigned char _command_id, unsigned char _progress_bar_value) {
        command_id = _command_id;
        progress_bar_value = _progress_bar_value;
      nitrokey::log::Log::instance()(
          std::string("LongOperationInProgressException, progress bar status: ")+
              std::to_string(progress_bar_value), nitrokey::log::Loglevel::DEBUG);
    }
    virtual const char *what() const throw() {
      return "Device returned busy status with long operation in progress";
    }
};


#endif //LIBNITROKEY_LONGOPERATIONINPROGRESSEXCEPTION_H
