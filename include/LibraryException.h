#ifndef LIBNITROKEY_LIBRARYEXCEPTION_H
#define LIBNITROKEY_LIBRARYEXCEPTION_H

#include <exception>
#include <cstdint>
#include <string>

class LibraryException: std::exception {
public:
    virtual uint8_t exception_id()= 0;
};



class InvalidSlotException : public LibraryException {
public:
    virtual uint8_t exception_id() override {
        return 201;
    }

public:
    uint8_t slot_selected;

    InvalidSlotException(uint8_t slot_selected) : slot_selected(slot_selected) {}

    virtual const char *what() const throw() override {
        return "Wrong slot selected";
    }

};



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

#endif //LIBNITROKEY_LIBRARYEXCEPTION_H
