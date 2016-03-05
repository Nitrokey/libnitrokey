#ifndef DEVICE_PROTO_H
#define DEVICE_PROTO_H
#include <utility>
#include <type_traits>
#include <stdexcept>
#include <string>
#include <strings.h>
// a local version for compatibility with Windows
#include "inttypes.h"
#include "cxx_semantics.h"
#include "device.h"
#include "misc.h"
#include "log.h"
#include "command_id.h"
#include "dissect.h"

#define STICK20_UPDATE_MODE_VID 0x03EB
#define STICK20_UPDATE_MODE_PID 0x2FF1

#define PAYLOAD_SIZE 53
#define PWS_SLOT_COUNT 16
#define PWS_SLOTNAME_LENGTH 11
#define PWS_PASSWORD_LENGTH 20
#define PWS_LOGINNAME_LENGTH 32

#define PWS_SEND_PASSWORD 0
#define PWS_SEND_LOGINNAME 1
#define PWS_SEND_TAB 2
#define PWS_SEND_CR 3

namespace nitrokey {
namespace proto {
/*
 *	POD types for HID proto commands
 *	Instances are meant to be __packed.
 *
 *	TODO (future) support for Big Endian
 */

/*
 *	Every packet is a USB HID report (check USB spec)
 */
template <CommandID cmd_id, typename Payload>
struct HIDReport {
  uint8_t _zero;
  CommandID command_id;  // uint8_t
  union {
    uint8_t _padding[HID_REPORT_SIZE - 6];
    Payload payload;
  } __packed;
  uint32_t crc;

  // POD types can't have non-default constructors
  // used in Transaction<>::run()
  void initialize() {
    bzero(this, sizeof *this);
    command_id = cmd_id;
  }

  uint32_t calculate_CRC() const {
    // w/o leading zero, a part of each HID packet
    // w/o 4-byte crc
    return misc::stm_crc32((const uint8_t *)(this) + 1,
                           (size_t)(HID_REPORT_SIZE - 5));
  }

  void update_CRC() { crc = calculate_CRC(); }

  bool isCRCcorrect() const { return crc == calculate_CRC(); }

  bool isValid() const {
    return true;
    //		return !_zero && payload.isValid() && isCRCcorrect();
  }

  operator std::string() const {
    // Packet type is known upfront in normal operation.
    // Can't be used to dissect random packets.
    return QueryDissector<cmd_id, decltype(*this)>::dissect(*this);
  }
} __packed;

/*
 *	Response payload (the parametrized type inside struct HIDReport)
 *
 *	command_id member in incoming HIDReport structure carries the command
 *	type last used.
 */
template <CommandID cmd_id, typename ResponsePayload>
struct DeviceResponse {
  uint8_t _zero;
  uint8_t device_status;
  uint8_t command_id;  // originally last_command_type
  uint32_t last_command_crc;
  uint8_t last_command_status;
  union {
    uint8_t _padding[HID_REPORT_SIZE - 12];
    ResponsePayload payload;
  } __packed;
  uint32_t crc;

  void initialize() { bzero(this, sizeof *this); }

  uint32_t calculate_CRC() const {
    // w/o leading zero, a part of each HID packet
    // w/o 4-byte crc
    return misc::stm_crc32((const uint8_t *)(this) + 1,
                           (size_t)(HID_REPORT_SIZE - 5));
  }

  void update_CRC() { crc = calculate_CRC(); }

  bool isCRCcorrect() const { return crc == calculate_CRC(); }

  bool isValid() const {
    //		return !_zero && payload.isValid() && isCRCcorrect() &&
    //				command_id == (uint8_t)(cmd_id);
    return true;
  }

  operator std::string() const {
    return ResponseDissector<cmd_id, decltype(*this)>::dissect(*this);
  }
} __packed;

struct EmptyPayload {
  uint8_t _data[];

  bool isValid() const { return true; }

  std::string dissect() const { return std::string("Empty Payload."); }
} __packed;

template <CommandID cmd_id, typename command_payload, typename response_payload>
class Transaction : semantics::non_constructible {
 public:
  // Types declared in command class scope can't be reached from there.
  typedef command_payload CommandPayload;
  typedef response_payload ResponsePayload;

  typedef struct HIDReport<cmd_id, CommandPayload> OutgoingPacket;
  typedef struct DeviceResponse<cmd_id, ResponsePayload> ResponsePacket;

  static_assert(std::is_pod<OutgoingPacket>::value,
                "outgoingpacket must be a pod type");
  static_assert(std::is_pod<ResponsePacket>::value,
                "ResponsePacket must be a POD type");
  static_assert(sizeof(OutgoingPacket) == HID_REPORT_SIZE,
                "OutgoingPacket type is not the right size");
  static_assert(sizeof(ResponsePacket) == HID_REPORT_SIZE,
                "ResponsePacket type is not the right size");

  static response_payload run(device::Device &dev,
                              const command_payload &payload) {
    using namespace ::nitrokey::device;
    using namespace ::nitrokey::log;

    Log::instance()(__PRETTY_FUNCTION__, Loglevel::DEBUG_L2);

    CommError status;
    OutgoingPacket outp;
    ResponsePacket resp;

    // POD types can't have non-default constructors
    outp.initialize();
    resp.initialize();

    outp.payload = payload;
    outp.update_CRC();

    Log::instance()("Outgoing HID packet:", Loglevel::DEBUG);
    Log::instance()((std::string)(outp), Loglevel::DEBUG);

    if (!outp.isValid()) throw std::runtime_error("Invalid outgoing packet");

    status = dev.send(&outp);
    if ((int)(status) < 0 && status != CommError::ERR_NO_ERROR)
      throw std::runtime_error(
          std::string("Device error while sending command ") +
          std::to_string((int)(status)));

    status = dev.recv(&resp);
    if ((int)(status) < 0 && status != CommError::ERR_NO_ERROR)
      throw std::runtime_error(
          std::string("Device error while executing command ") +
          std::to_string((int)(status)));

    Log::instance()("Incoming HID packet:", Loglevel::DEBUG);
    Log::instance()((std::string)(resp), Loglevel::DEBUG);

    if (!resp.isValid()) throw std::runtime_error("Invalid incoming packet");

    // See: DeviceResponse
    return resp.payload;
  }

  static response_payload run(device::Device &dev) {
    command_payload empty_payload;
    return run(dev, empty_payload);
  }
};
}
}
#endif
