#ifndef LIBNITROKEY_NK_C_API_HELPERS_H
#define LIBNITROKEY_NK_C_API_HELPERS_H


#include "NK_C_API.h"
#include <iostream>
#include <tuple>
#include "libnitrokey/NitrokeyManager.h"
#include <cstring>
#include "libnitrokey/LibraryException.h"
#include "libnitrokey/cxx_semantics.h"
#include "libnitrokey/stick20_commands.h"
#include "libnitrokey/device_proto.h"
#include "libnitrokey/version.h"

void clear_string(std::string &s);

extern uint8_t NK_last_command_status;


template <typename T>
T* duplicate_vector_and_clear(std::vector<T> &v){
  auto d = new T[v.size()];
  std::copy(v.begin(), v.end(), d);
  std::fill(v.begin(), v.end(), 0);
  return d;
}

template <typename R, typename T>
std::tuple<int, R> get_with_status(T func, R fallback) {
  NK_last_command_status = 0;
  try {
    return std::make_tuple(0, func());
  }
  catch (CommandFailedException & commandFailedException){
    NK_last_command_status = commandFailedException.last_command_status;
  }
  catch (LibraryException & libraryException){
    NK_last_command_status = libraryException.exception_id();
  }
  catch (const DeviceCommunicationException &deviceException){
    NK_last_command_status = 256-deviceException.getType();
  }
  return std::make_tuple(NK_last_command_status, fallback);
}

template <typename T>
uint8_t * get_with_array_result(T func){
  return std::get<1>(get_with_status<uint8_t*>(func, nullptr));
}

template <typename T>
char* get_with_string_result(T func){
  auto result = std::get<1>(get_with_status<char*>(func, nullptr));
  if (result == nullptr) {
    return strndup("", MAXIMUM_STR_REPLY_LENGTH);
  }
  return result;
}

template <typename T>
auto get_with_result(T func){
  return std::get<1>(get_with_status(func, static_cast<decltype(func())>(0)));
}

template <typename T>
uint8_t get_without_result(T func){
  NK_last_command_status = 0;
  try {
    func();
    return 0;
  }
  catch (CommandFailedException & commandFailedException){
    NK_last_command_status = commandFailedException.last_command_status;
  }
  catch (LibraryException & libraryException){
    NK_last_command_status = libraryException.exception_id();
  }
  catch (const InvalidCRCReceived &invalidCRCException){
    ;
  }
  catch (const DeviceCommunicationException &deviceException){
    NK_last_command_status = 256-deviceException.getType();
  }
  return NK_last_command_status;
}


#endif // LIBNITROKEY_NK_C_API_HELPERS_H
