//
// Created by sz on 09.08.16.
//

#ifndef LIBNITROKEY_LIBRARYEXCEPTION_H
#define LIBNITROKEY_LIBRARYEXCEPTION_H

#include <exception>
#include <cstdint>

class LibraryException: std::exception {
public:
    virtual uint8_t exception_id()= 0;
};


#endif //LIBNITROKEY_LIBRARYEXCEPTION_H
