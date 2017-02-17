#include <chrono>
#include <thread>
#include <cstddef>
#include <stdexcept>
#include "hidapi/hidapi.h"
#include "include/misc.h"
#include "include/device.h"
#include "include/log.h"
#include <mutex>
#include "DeviceCommunicationExceptions.h"

std::mutex mex_dev_com;

using namespace nitrokey::device;
using namespace nitrokey::log;
using namespace std::chrono;

Device::Device(const uint16_t vid, const uint16_t pid, const DeviceModel model,
               const milliseconds send_receive_delay, const int retry_receiving_count,
               const milliseconds retry_timeout)
    : m_vid(vid),
      m_pid(pid),
      m_retry_sending_count(3),
      m_retry_receiving_count(retry_receiving_count),
      m_retry_timeout(retry_timeout),
      mp_devhandle(nullptr),
      last_command_status(0),
      m_model(model),
      m_send_receive_delay(send_receive_delay)
{}

bool Device::disconnect() {
  //called in object's destructor
  Log::instance()(__FUNCTION__, Loglevel::DEBUG_L2);
  std::lock_guard<std::mutex> lock(mex_dev_com);
  Log::instance()(std::string(__FUNCTION__) +  std::string(m_model==DeviceModel::PRO?"PRO":"STORAGE"), Loglevel::DEBUG_L2);
  Log::instance()(std::string(__FUNCTION__) +  std::string(" *IN* "), Loglevel::DEBUG_L2);

  Log::instance()(std::string("Disconnection success: ") + std::to_string(mp_devhandle == nullptr), Loglevel::DEBUG_L2);
  if(mp_devhandle == nullptr) return false;

  hid_close(mp_devhandle);
  mp_devhandle = nullptr;
  //FIXME hidexit should not be called if some devices are still active - use static active devices counter
  //  hid_exit();
  return true;
}
bool Device::connect() {
  Log::instance()(__FUNCTION__, Loglevel::DEBUG_L2);
  std::lock_guard<std::mutex> lock(mex_dev_com);
  Log::instance()(std::string(__FUNCTION__) +  std::string(" *IN* "), Loglevel::DEBUG_L2);

//   hid_init(); // done automatically on hid_open
  mp_devhandle = hid_open(m_vid, m_pid, nullptr);
  const auto success = mp_devhandle != nullptr;
  Log::instance()(std::string("Connection success: ") + std::to_string(success), Loglevel::DEBUG_L2);
  return success;
}

int Device::send(const void *packet) {
  Log::instance()(__FUNCTION__, Loglevel::DEBUG_L2);
  std::lock_guard<std::mutex> lock(mex_dev_com);
  Log::instance()(std::string(__FUNCTION__) +  std::string(" *IN* "), Loglevel::DEBUG_L2);

  if (mp_devhandle == nullptr) {
    Log::instance()(std::string("Connection fail") , Loglevel::DEBUG_L2);
    throw DeviceNotConnected("Attempted HID send on an invalid descriptor.");
  }

  return (hid_send_feature_report(
      mp_devhandle, (const unsigned char *)(packet), HID_REPORT_SIZE));
}

int Device::recv(void *packet) {
  Log::instance()(__FUNCTION__, Loglevel::DEBUG_L2);
  std::lock_guard<std::mutex> lock(mex_dev_com);
  Log::instance()(std::string(__FUNCTION__) +  std::string(" *IN* "), Loglevel::DEBUG_L2);
  int status;
  int retry_count = 0;


  if (mp_devhandle == nullptr){
    Log::instance()(std::string("Connection fail") , Loglevel::DEBUG_L2);
    throw DeviceNotConnected("Attempted HID receive on an invalid descriptor.");
  }

  // FIXME extract error handling and repeating to parent function in
  // device_proto:192
  for (;;) {
    status = (hid_get_feature_report(mp_devhandle, (unsigned char *)(packet),
                                     HID_REPORT_SIZE));

    // FIXME handle getting libhid error message somewhere else
    auto pwherr = hid_error(mp_devhandle);
    std::wstring wherr = (pwherr != nullptr) ? pwherr : L"No error message";
    std::string herr(wherr.begin(), wherr.end());
    Log::instance()(std::string("libhid error message: ") + herr,
                    Loglevel::DEBUG_L2);

    if (status > 0) break;  // success
    if (retry_count++ >= m_retry_receiving_count) {
      Log::instance()(
          "Maximum retry count reached: " + std::to_string(retry_count),
          Loglevel::WARNING);
      Log::instance()(
          std::string("Counter stats: ") + m_counters.get_as_string(),
          Loglevel::DEBUG);
      break;
    }
    Log::instance()("Retrying... " + std::to_string(retry_count),
                    Loglevel::DEBUG);
    std::this_thread::sleep_for(m_retry_timeout);
  }

  return status;
}

bool Device::could_be_enumerated() {
  Log::instance()(__FUNCTION__, Loglevel::DEBUG_L2);
  std::lock_guard<std::mutex> lock(mex_dev_com);
  if (mp_devhandle==nullptr){
    return false;
  }
  auto pInfo = hid_enumerate(m_vid, m_pid);
  if (pInfo != nullptr){
    hid_free_enumeration(pInfo);
    return true;
  }
  return false;

//  alternative:
//  unsigned char buf[1];
//  return hid_read_timeout(mp_devhandle, buf, sizeof(buf), 20) != -1;
}

void Device::show_stats() {
  auto s = m_counters.get_as_string();
  Log::instance()(s, Loglevel::DEBUG_L2);
}

Stick10::Stick10():
  Device(0x20a0, 0x4108, DeviceModel::PRO, 100ms, 5, 100ms)
  {}


Stick20::Stick20():
  Device(0x20a0, 0x4109, DeviceModel::STORAGE, 200ms, 5, 200ms)
  {}

#include <sstream>
#define p(x) ss << #x << " " << x << ", ";
std::string Device::ErrorCounters::get_as_string() {
  std::stringstream ss;
  p(total_comm_runs);
  p(communication_successful);
  ss << "(";
  p(command_successful_recv);
  p(command_result_not_equal_0_recv);
  ss << "), ";
  p(sends_executed);
  p(recv_executed);
  p(successful_storage_commands);
  p(total_retries);
  ss << "(";
  p(busy);
  p(busy_progressbar);
  p(CRC_other_than_awaited);
  p(wrong_CRC);
  ss << "), ";
  p(sending_error);
  p(receiving_error);
  return ss.str();
}
