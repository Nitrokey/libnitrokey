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

#ifndef DEVICE_H
#define DEVICE_H
#include <chrono>
#include "hidapi/hidapi.h"
#include <cstdint>
#include <memory>
#include <string>
#include <ostream>
#include <vector>
#include "misc.h"

#define HID_REPORT_SIZE 65

#include <atomic>

namespace nitrokey {
namespace device {
    using namespace std::chrono_literals;
    using std::chrono::milliseconds;

    struct EnumClassHash
    {
        template <typename T>
        std::size_t operator()(T t) const
        {
          return static_cast<std::size_t>(t);
        }
    };

enum class DeviceModel{
    PRO,
    STORAGE
};

std::ostream& operator<<(std::ostream& stream, DeviceModel model);

/**
 * The USB vendor ID for Nitrokey devices.
 */
extern const uint16_t NITROKEY_VID;
/**
 * The USB product ID for the Nitrokey Pro.
 */
extern const uint16_t NITROKEY_PRO_PID;
/**
 * The USB product ID for the Nitrokey Storage.
 */
extern const uint16_t NITROKEY_STORAGE_PID;

/**
 * Convert the given USB product ID to a Nitrokey model.  If there is no model
 * with that ID, return an absent value.
 */
misc::Option<DeviceModel> product_id_to_model(uint16_t product_id);

/**
 * Information about a connected device.
 *
 * This struct contains the information about a connected device returned by
 * hidapi when enumerating the connected devices.
 */
struct DeviceInfo {
    /**
     * The model of the connected device.
     */
    DeviceModel m_deviceModel;
    /**
     * The USB connection path for the device.
     */
    std::string m_path;
    /**
     * The serial number of the device.
     */
    std::string m_serialNumber;
};

#include <atomic>

class Device {

public:

  struct ErrorCounters{
    using cnt = std::atomic_int;
    cnt wrong_CRC;
    cnt CRC_other_than_awaited;
    cnt busy;
    cnt total_retries;
    cnt sending_error;
    cnt receiving_error;
    cnt total_comm_runs;
    cnt successful_storage_commands;
    cnt command_successful_recv;
    cnt recv_executed;
    cnt sends_executed;
    cnt busy_progressbar;
    cnt command_result_not_equal_0_recv;
    cnt communication_successful;
    cnt low_level_reconnect;
    std::string get_as_string();

  } m_counters = {};


    Device(const uint16_t vid, const uint16_t pid, const DeviceModel model,
                   const milliseconds send_receive_delay, const int retry_receiving_count,
                   const milliseconds retry_timeout);

    virtual ~Device();

  // lack of device is not actually an error,
  // so it doesn't throw
  virtual bool connect();
  virtual bool disconnect();

  /*
   *	Sends packet of HID_REPORT_SIZE.
   */
  virtual int send(const void *packet);

  /*
   *	Gets packet of HID_REPORT_SIZE.
   *	Can sleep. See below.
   */
  virtual int recv(void *packet);

  /***
   * Returns true if some device is visible by OS with given VID and PID
   * whether the device is connected through HID API or not.
   * @return true if visible by OS
   */
  bool could_be_enumerated();
  /**
   * Returns a vector with all connected Nitrokey devices.
   *
   * @return information about all connected devices
   */
  static std::vector<DeviceInfo> enumerate();

  /**
   * Create a Device of the given model.
   */
  static std::shared_ptr<Device> create(DeviceModel model);


        void show_stats();
//  ErrorCounters get_stats(){ return m_counters; }
  int get_retry_receiving_count() const { return m_retry_receiving_count; };
  int get_retry_sending_count() const { return m_retry_sending_count; };
  std::chrono::milliseconds get_retry_timeout() const { return m_retry_timeout; };
  std::chrono::milliseconds get_send_receive_delay() const {return m_send_receive_delay;}

  int get_last_command_status() {int a = std::atomic_exchange(&last_command_status, static_cast<uint8_t>(0)); return a;};
  void set_last_command_status(uint8_t _err) { last_command_status = _err;} ;
  bool last_command_sucessfull() const {return last_command_status == 0;};
  DeviceModel get_device_model() const {return m_model;}
  void set_receiving_delay(std::chrono::milliseconds delay);
  void set_retry_delay(std::chrono::milliseconds delay);
  static void set_default_device_speed(int delay);
  void setDefaultDelay();
  void set_path(const std::string path);


        private:
  std::atomic<uint8_t> last_command_status;
  void _reconnect();
  bool _connect();
  bool _disconnect();

protected:
  const uint16_t m_vid;
  const uint16_t m_pid;
  const DeviceModel m_model;

  /*
   *	While the project uses Signal11 portable HIDAPI
   *	library, there's no way of doing it asynchronously,
   *	hence polling.
   */
  const int m_retry_sending_count;
  const int m_retry_receiving_count;
  std::chrono::milliseconds m_retry_timeout;
  std::chrono::milliseconds m_send_receive_delay;
  std::atomic<hid_device *>mp_devhandle;
  std::string m_path;

  static std::atomic_int instances_count;
  static std::chrono::milliseconds default_delay ;
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
