#include <chrono>
#include <thread>
#include <cstddef>
#include <stdexcept>
#include <hidapi/hidapi.h>
#include "include/misc.h"
#include "include/device.h"
#include "include/log.h"

using namespace nitrokey::device;
using namespace nitrokey::log;

Device::Device()
    : m_vid(0),
      m_pid(0),
      m_retry_count(40),
      m_retry_timeout(100),
      mp_devhandle(NULL),
      last_command_status(0){}

bool Device::disconnect() {
  Log::instance()(__PRETTY_FUNCTION__, Loglevel::DEBUG_L2);

  hid_exit();
  mp_devhandle = NULL;
  return true;
}
bool Device::connect() {
  Log::instance()(__PRETTY_FUNCTION__, Loglevel::DEBUG_L2);

  // hid_init();
  mp_devhandle = hid_open(m_vid, m_pid, NULL);
  // hid_init();
  return mp_devhandle != NULL;
}

int Device::send(const void *packet) {
  Log::instance()(__PRETTY_FUNCTION__, Loglevel::DEBUG_L2);

  if (mp_devhandle == NULL)
    throw std::runtime_error("Attempted HID send on an invalid descriptor.");

  return (hid_send_feature_report(
      mp_devhandle, (const unsigned char *)(packet), HID_REPORT_SIZE));
}

int Device::recv(void *packet) {
  int status;
  int retry_count = 0;

  Log::instance()(__PRETTY_FUNCTION__, Loglevel::DEBUG_L2);

  if (mp_devhandle == NULL)
    throw std::runtime_error("Attempted HID receive on an invalid descriptor.");

  // FIXME extract error handling and repeating to parent function in
  // device_proto:192
  for (;;) {
    status = (hid_get_feature_report(mp_devhandle, (unsigned char *)(packet),
                                     HID_REPORT_SIZE));

    // FIXME handle getting libhid error message somewhere else
    auto pwherr = hid_error(mp_devhandle);
    std::wstring wherr = (pwherr != NULL) ? pwherr : L"No error message";
    std::string herr(wherr.begin(), wherr.end());
    Log::instance()(std::string("libhid error message: ") + herr,
                    Loglevel::DEBUG_L2);

    if (status > 0) break;  // success
    if (retry_count++ >= m_retry_count) {
      Log::instance()(
          "Maximum retry count reached" + std::to_string(retry_count),
          Loglevel::WARNING);
      break;
    }
    Log::instance()("Retrying... " + std::to_string(retry_count),
                    Loglevel::DEBUG);
    std::this_thread::sleep_for(m_retry_timeout);
  }

  return status;
}

Stick10::Stick10() {
  m_vid = 0x20a0;
  m_pid = 0x4108;
  m_model = DeviceModel::PRO;
    m_send_receive_delay = 100ms;
  m_retry_count = 100;
}

Stick20::Stick20() {
  m_vid = 0x20a0;
  m_pid = 0x4109;
  m_retry_timeout = std::chrono::milliseconds(500);
  m_model = DeviceModel::STORAGE;
    m_send_receive_delay = 1000ms;
}
