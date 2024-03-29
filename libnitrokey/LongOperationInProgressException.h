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

#include "CommandFailedException.h"

class LongOperationInProgressException : public CommandFailedException {

public:
    unsigned char progress_bar_value;

    LongOperationInProgressException(
        unsigned char command_id_, uint8_t last_command_status_, unsigned char progress_bar_value_)
    : CommandFailedException(command_id_, last_command_status_), progress_bar_value(progress_bar_value_){
      LOG(
          std::string("LongOperationInProgressException, progress bar status: ")+
              std::to_string(progress_bar_value), nitrokey::log::Loglevel::DEBUG);
    }
    virtual const char *what() const noexcept override {
      return "Device returned busy status with long operation in progress";
    }
};


#endif //LIBNITROKEY_LONGOPERATIONINPROGRESSEXCEPTION_H
