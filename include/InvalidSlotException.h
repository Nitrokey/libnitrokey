//
// Created by sz on 09.08.16.
//

#ifndef LIBNITROKEY_INVALIDSLOTEXCEPTION_H
#define LIBNITROKEY_INVALIDSLOTEXCEPTION_H


#include <cstdint>
#include <string>
#include "LibraryException.h"


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

#endif //LIBNITROKEY_INVALIDSLOTEXCEPTION_H
