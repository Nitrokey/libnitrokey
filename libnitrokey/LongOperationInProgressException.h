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

#ifndef LIBNITROKEY_LONGOPERATIONINPROGRESSEXCEPTION_H
#define LIBNITROKEY_LONGOPERATIONINPROGRESSEXCEPTION_H

#include "DeviceCommunicationExceptions.h"

class LongOperationInProgressException : public DeviceCommunicationException {

public:
    unsigned char progress_bar_value;

    LongOperationInProgressException(unsigned char _progress_bar_value)
    : DeviceCommunicationException("Device returned busy status with long operation in progress"),
      progress_bar_value(_progress_bar_value){
      LOG(
          std::string("LongOperationInProgressException, progress bar status: ")+
              std::to_string(progress_bar_value), nitrokey::log::Loglevel::DEBUG);
    }

    uint8_t getType() const {return 6;};
};


#endif //LIBNITROKEY_LONGOPERATIONINPROGRESSEXCEPTION_H
