#ifndef DEVICE_PROTO_H
#define DEVICE_PROTO_H

#include <utility>
#include <thread>
#include <type_traits>
#include <stdexcept>
#include <string>
#include <strings.h>
// a local version for compatibility with Windows
#include <stdint.h>
#include "cxx_semantics.h"
#include "device.h"
#include "misc.h"
#include "log.h"
#include "command_id.h"
#include "dissect.h"
#include "CommandFailedException.h"
#include "LongOperationInProgressException.h"

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

#include <mutex>

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
        template<CommandID cmd_id, typename Payload>
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
              return misc::stm_crc32((const uint8_t *) (this) + 1,
                                     (size_t) (HID_REPORT_SIZE - 5));
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
        namespace DeviceResponseConstants{
            //magic numbers from firmware
            static constexpr auto storage_status_absolute_address = 21;
            static constexpr auto storage_data_absolute_address = storage_status_absolute_address + 5;
            static constexpr auto header_size = 8; //from _zero to last_command_status inclusive
            static constexpr auto footer_size = 4; //crc
            static constexpr auto wrapping_size = header_size + footer_size;
        }

        template<CommandID cmd_id, typename ResponsePayload>
        struct DeviceResponse {
            static constexpr auto storage_status_padding_size =
                DeviceResponseConstants::storage_status_absolute_address - DeviceResponseConstants::header_size;

            uint8_t _zero;
            uint8_t device_status;
            uint8_t command_id;  // originally last_command_type
            uint32_t last_command_crc;
            uint8_t last_command_status;

            union {
                uint8_t _padding[HID_REPORT_SIZE - DeviceResponseConstants::wrapping_size];
                ResponsePayload payload;
                struct {
                    uint8_t _storage_status_padding[storage_status_padding_size];
                    uint8_t command_counter;
                    uint8_t command_id;
                    uint8_t device_status; //@see stick20::device_status
                    uint8_t progress_bar_value;
                } __packed storage_status;
            } __packed;

            uint32_t crc;

            void initialize() { bzero(this, sizeof *this); }

            uint32_t calculate_CRC() const {
              // w/o leading zero, a part of each HID packet
              // w/o 4-byte crc
              return misc::stm_crc32((const uint8_t *) (this) + 1,
                                     (size_t) (HID_REPORT_SIZE - 5));
            }

            void update_CRC() { crc = calculate_CRC(); }
            bool isCRCcorrect() const { return crc == calculate_CRC(); }
            bool isValid() const {
              //		return !_zero && payload.isValid() && isCRCcorrect() &&
              //				command_id == (uint8_t)(cmd_id);
              return crc != 0;
            }

            operator std::string() const {
              return ResponseDissector<cmd_id, decltype(*this)>::dissect(*this);
            }
        } __packed;

        struct EmptyPayload {
            bool isValid() const { return true; }

            std::string dissect() const { return std::string("Empty Payload."); }
        } __packed;

        template<typename command_packet, typename response_payload>
        class ClearingProxy {
        public:
            ClearingProxy(command_packet &p) {
              packet = p;
              bzero(&p, sizeof(p));
            }

            ~ClearingProxy() {
              bzero(&packet, sizeof(packet));
            }

            response_payload &data() {
              return packet.payload;
            }

            command_packet packet;
        };

        template<CommandID cmd_id, typename command_payload, typename response_payload>
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

            static uint32_t getCRC(
                const command_payload &payload) {
              OutgoingPacket outp;
              outp.initialize();
              outp.payload = payload;
              outp.update_CRC();
              return outp.crc;
            }

            template<typename T>
            static void clear_packet(T &st) {
              bzero(&st, sizeof(st));
            }

            static ClearingProxy<ResponsePacket, response_payload> run(device::Device &dev,
                                                                       const command_payload &payload) {
              using namespace ::nitrokey::device;
              using namespace ::nitrokey::log;
              using namespace std::chrono_literals;

              static std::mutex send_receive_mtx;
              std::lock_guard<std::mutex> guard(send_receive_mtx);

              Log::instance()(__PRETTY_FUNCTION__, Loglevel::DEBUG_L2);

              int status;
              OutgoingPacket outp;
              ResponsePacket resp;

              // POD types can't have non-default constructors
              outp.initialize();
              resp.initialize();

              outp.payload = payload;
              outp.update_CRC();

              Log::instance()("Outgoing HID packet:", Loglevel::DEBUG);
              Log::instance()(static_cast<std::string>(outp), Loglevel::DEBUG);

              if (!outp.isValid()) throw std::runtime_error("Invalid outgoing packet");

              bool successful_communication = false;
              int receiving_retry_counter = 0;
              int sending_retry_counter = dev.get_retry_sending_count();
              while (sending_retry_counter-- > 0) {
                status = dev.send(&outp);
                if (status <= 0)
                  throw std::runtime_error(
                      std::string("Device error while sending command ") +
                      std::to_string(status));

                std::this_thread::sleep_for(dev.get_send_receive_delay());

                // FIXME make checks done in device:recv here
                receiving_retry_counter = dev.get_retry_receiving_count();
                while (receiving_retry_counter-- > 0) {
                  status = dev.recv(&resp);

                  if (dev.get_device_model() == DeviceModel::STORAGE &&
                      resp.command_id >= stick20::CMD_START_VALUE &&
                      resp.command_id < stick20::CMD_END_VALUE ) {
                    Log::instance()(std::string("Detected storage device cmd, status: ") +
                                    std::to_string(resp.storage_status.device_status), Loglevel::DEBUG_L2);

                    resp.last_command_status = static_cast<uint8_t>(stick10::command_status::ok);
                    switch (static_cast<stick20::device_status>(resp.storage_status.device_status)) {
                      case stick20::device_status::idle :
                      case stick20::device_status::ok:
                        resp.device_status = static_cast<uint8_t>(stick10::device_status::ok);
                        break;
                      case stick20::device_status::busy:
                      case stick20::device_status::busy_progressbar: //TODO this will be modified later for getting progressbar status
                        resp.device_status = static_cast<uint8_t>(stick10::device_status::busy);
                        break;
                      case stick20::device_status::wrong_password:
                        resp.last_command_status = static_cast<uint8_t>(stick10::command_status::wrong_password);
                        resp.device_status = static_cast<uint8_t>(stick10::device_status::ok);
                        break;
                      default:
                        Log::instance()(std::string("Unknown storage device status, cannot translate: ") +
                                        std::to_string(resp.storage_status.device_status), Loglevel::DEBUG);
                        resp.device_status = resp.storage_status.device_status;
                        break;
                    };
                  }

                  //SENDPASSWORD gives wrong CRC , for now rely on !=0 (TODO report)
//                  if (resp.device_status == 0 && resp.last_command_crc == outp.crc && resp.isCRCcorrect()) break;
                  if (resp.device_status == static_cast<uint8_t>(stick10::device_status::ok) &&
                      resp.last_command_crc == outp.crc && resp.isValid()){
                    successful_communication = true;
                    break;
                  }
                  if (resp.device_status == static_cast<uint8_t>(stick10::device_status::busy)) {
                    receiving_retry_counter++;
                    Log::instance()("Status busy, not decresing receiving_retry_counter counter: " +
                                    std::to_string(receiving_retry_counter), Loglevel::DEBUG_L2);
                  }
                  if (resp.device_status == static_cast<uint8_t>(stick10::device_status::busy) &&
                      static_cast<stick20::device_status>(resp.storage_status.device_status)
                      == stick20::device_status::busy_progressbar){
                    successful_communication = true;
                    break;
                  }
                  Log::instance()(std::string("Retry status - dev status, awaited cmd crc, correct packet CRC: ")
                                  + std::to_string(resp.device_status) + " " +
                                  std::to_string(resp.last_command_crc == outp.crc) +
                                  " " + std::to_string(resp.isCRCcorrect()), Loglevel::DEBUG_L2);

                  Log::instance()(
                      "Device is not ready or received packet's last CRC is not equal to sent CRC packet, retrying...",
                      Loglevel::DEBUG);
                  Log::instance()("Invalid incoming HID packet:", Loglevel::DEBUG_L2);
                  Log::instance()(static_cast<std::string>(resp), Loglevel::DEBUG_L2);
                  std::this_thread::sleep_for(dev.get_retry_timeout());
                  continue;
                }
                if (successful_communication) break;
                Log::instance()(std::string("Resending (outer loop) "), Loglevel::DEBUG_L2);
                Log::instance()(std::string("sending_retry_counter count: ") + std::to_string(sending_retry_counter),
                                Loglevel::DEBUG);
              }

              dev.set_last_command_status(resp.last_command_status); // FIXME should be handled on device.recv

              clear_packet(outp);

              if (status <= 0)
                throw std::runtime_error( //FIXME replace with CriticalErrorException
                    std::string("Device error while executing command ") +
                    std::to_string(status));

              Log::instance()("Incoming HID packet:", Loglevel::DEBUG);
              Log::instance()(static_cast<std::string>(resp), Loglevel::DEBUG);
              Log::instance()(std::string("receiving_retry_counter count: ") + std::to_string(receiving_retry_counter),
                              Loglevel::DEBUG);

              if (resp.device_status == static_cast<uint8_t>(stick10::device_status::busy) &&
                  static_cast<stick20::device_status>(resp.storage_status.device_status)
                  == stick20::device_status::busy_progressbar){
                throw LongOperationInProgressException(
                    resp.command_id, resp.device_status, resp.storage_status.progress_bar_value);
              }

              if (!resp.isValid()) throw std::runtime_error("Invalid incoming packet");
              if (receiving_retry_counter <= 0)
                throw std::runtime_error(
                    "Maximum receiving_retry_counter count reached for receiving response from the device!");
              if (resp.last_command_status != static_cast<uint8_t>(stick10::command_status::ok))
                throw CommandFailedException(resp.command_id, resp.last_command_status);


              // See: DeviceResponse
              return resp;
            }

            static ClearingProxy<ResponsePacket, response_payload> run(device::Device &dev) {
              command_payload empty_payload;
              return run(dev, empty_payload);
            }
        };
    }
}
#endif
