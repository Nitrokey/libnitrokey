//
// Created by sz on 09.08.16.
//

#ifndef LIBNITROKEY_TOOLONGSTRINGEXCEPTION_H
#define LIBNITROKEY_TOOLONGSTRINGEXCEPTION_H


#include <cstdint>
#include <string>
#include <exception>

class TooLongStringException : public std::exception {
public:
    static const std::uint8_t exception_id = 200;

    std::size_t size_source;
    std::size_t size_destination;
    std::string message;

    TooLongStringException(size_t size_source, size_t size_destination, const std::string &message = "") : size_source(
            size_source), size_destination(size_destination), message(message) {}

    virtual const char *what() const throw() {
        //TODO add sizes and message data to final message
        return "Too long string has been supplied as an argument";
    }

};


#endif //LIBNITROKEY_TOOLONGSTRINGEXCEPTION_H
