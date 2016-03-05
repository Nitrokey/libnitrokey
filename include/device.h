#ifndef DEVICE_H
#define DEVICE_H
#include <chrono>
#include <hidapi/hidapi.h>
#include "inttypes.h"

#define HID_REPORT_SIZE 65

// TODO !! SEMAPHORE

namespace nitrokey {
namespace device {

enum class CommError {
  ERR_NO_ERROR = 0,
  ERR_NOT_CONNECTED = -1,
  ERR_WRONG_RESPONSE_CRC = -2,
  ERR_SENDING = -3,
  ERR_STATUS_NOT_OK = -4
};

class Device {
 public:
  Device();

  // lack of device is not actually an error,
  // so it doesn't throw
  virtual bool connect();
  virtual bool disconnect();

  /*
   *	Sends packet of HID_REPORT_SIZE.
   */
  virtual CommError send(const void *packet);

  /*
   *	Gets packet of HID_REPORT_SIZE.
   *	Can sleep. See below.
   */
  virtual CommError recv(void *packet);

 protected:
  uint16_t m_vid;
  uint16_t m_pid;

  /*
   *	While the project uses Signal11 portable HIDAPI
   *	library, there's no way of doing it asynchronously,
   *	hence polling.
   */
  int m_retry_count;
  std::chrono::milliseconds m_retry_timeout;

  hid_device *mp_devhandle;
};

class Stick10 : public Device {
 public:
  Stick10();
};

class Stick20 : public Device {
 public:
  Stick20();
};
}
}
#endif
