#ifndef LIBNITROKEY_DEVICECOMMUNICATIONEXCEPTIONS_H
#define LIBNITROKEY_DEVICECOMMUNICATIONEXCEPTIONS_H

#include <exception>
#include <string>
//class DeviceCommunicationException: public std::exception {
class DeviceCommunicationException: public std::runtime_error{
  std::string message;
public:
  DeviceCommunicationException(std::string _msg): runtime_error(_msg), message(_msg){}
//  virtual const char* what() const throw() override {
//    return message.c_str();
//  }
};

class DeviceNotConnected: public DeviceCommunicationException {
public:
  DeviceNotConnected(std::string msg) : DeviceCommunicationException(msg){}
};

class DeviceSendingFailure: public DeviceCommunicationException {
public:
  DeviceSendingFailure(std::string msg) : DeviceCommunicationException(msg){}
};

class DeviceReceivingFailure: public DeviceCommunicationException {
public:
  DeviceReceivingFailure(std::string msg) : DeviceCommunicationException(msg){}
};

#endif //LIBNITROKEY_DEVICECOMMUNICATIONEXCEPTIONS_H
