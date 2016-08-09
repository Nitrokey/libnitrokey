//
// Created by sz on 09.08.16.
//

#ifndef LIBNITROKEY_INVALIDSLOTEXCEPTION_H
#define LIBNITROKEY_INVALIDSLOTEXCEPTION_H


#include <cstdint>
#include <string>
#include <exception>

class InvalidSlotException : public std::exception {
public:
    static const std::uint8_t exception_id = 201;

    uint8_t slot_selected;

    InvalidSlotException(uint8_t slot_selected) : slot_selected(slot_selected) {}

    virtual const char *what() const throw() {
        return "Wrong slot selected";
    }

};

#endif //LIBNITROKEY_INVALIDSLOTEXCEPTION_H
