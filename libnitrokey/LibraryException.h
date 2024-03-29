/*
 * Copyright (c) 2015-2018 Nitrokey UG
 *
 * This file is part of libnitrokey.
 *
 * libnitrokey is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * libnitrokey is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libnitrokey. If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0
 */

#ifndef LIBNITROKEY_LIBRARYEXCEPTION_H
#define LIBNITROKEY_LIBRARYEXCEPTION_H

#include <mutex>
#include <exception>
#include <cstdint>
#include <string>
#include "log.h"

class LibraryException: std::exception {
public:
    virtual uint8_t exception_id()= 0;
};

// Use static string object for keeping the c_str message for the caller.
// Strings collection used as an alternative to memory leaks done via strdup().
// TargetBufferSmallerThanSource Exception should never happen in a correctly written library client.
static std::vector<std::string> g_exception_messages;
static std::mutex g_exception_message_mutex;

class TargetBufferSmallerThanSource: public LibraryException {
public:
    virtual uint8_t exception_id() override {
        return 203;
    }

public:
    size_t source_size;
    size_t target_size;

    TargetBufferSmallerThanSource(
            size_t source_size_, size_t target_size_
            ) : source_size(source_size_),  target_size(target_size_) {}

    virtual const char *what() const noexcept override {
        std::lock_guard<std::mutex> lock(g_exception_message_mutex);
        std::string s = " ";
        auto ts = [](size_t x){ return std::to_string(x); };
        g_exception_messages.emplace_back(std::string("Target buffer size is smaller than source: [source size, buffer size]")
                              + s + ts(source_size) + s + ts(target_size));
        return g_exception_messages.back().c_str();
    }

};

class InvalidHexString : public LibraryException {
public:
    virtual uint8_t exception_id() override {
        return 202;
    }

public:
    uint8_t invalid_char;

    InvalidHexString (uint8_t invalid_char_) : invalid_char(invalid_char_) {}

    virtual const char *what() const noexcept override {
        return "Invalid character in hex string";
    }

};

class InvalidSlotException : public LibraryException {
public:
    virtual uint8_t exception_id() override {
        return 201;
    }

public:
    uint8_t slot_selected;

    InvalidSlotException(uint8_t slot_selected_) : slot_selected(slot_selected_) {}

    virtual const char *what() const noexcept override {
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

    TooLongStringException(size_t size_source_, size_t size_destination_, const std::string &message_ = "") : size_source(
            size_source_), size_destination(size_destination_), message(message_) {
      LOG(std::string("TooLongStringException, size diff: ")+ std::to_string(size_source-size_destination), nitrokey::log::Loglevel::DEBUG);

    }

    virtual const char *what() const noexcept override {
        //TODO add sizes and message data to final message
        return "Too long string has been supplied as an argument";
    }

};

#endif //LIBNITROKEY_LIBRARYEXCEPTION_H
