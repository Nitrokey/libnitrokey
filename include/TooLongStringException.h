//
// Created by sz on 09.08.16.
//

#ifndef LIBNITROKEY_TOOLONGSTRINGEXCEPTION_H
#define LIBNITROKEY_TOOLONGSTRINGEXCEPTION_H


#include <cstdint>
#include <string>
#include "LibraryException.h"

class TooLongStringException : public LibraryException {
public:
    virtual uint8_t exception_id() override {
        return 200;
    }

    std::size_t size_source;
    std::size_t size_destination;
    std::string message;

    TooLongStringException(size_t size_source, size_t size_destination, const std::string &message = "") : size_source(
            size_source), size_destination(size_destination), message(message) {}

    virtual const char *what() const throw() override {
        //TODO add sizes and message data to final message
        return "Too long string has been supplied as an argument";
    }

};


#endif //LIBNITROKEY_TOOLONGSTRINGEXCEPTION_H
