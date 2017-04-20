/*
 *	Protocol packet dissection
 */
#ifndef DISSECT_H
#define DISSECT_H
#include <string>
#include <sstream>
#include <iomanip>
#include "misc.h"
#include "cxx_semantics.h"
#include "command_id.h"
#include "device_proto.h"

namespace nitrokey {
namespace proto {

template <CommandID id, class HIDPacket>
class QueryDissector : semantics::non_constructible {
 public:
  static std::string dissect(const HIDPacket &pod) {
    std::stringstream out;

    out << "Raw HID packet:" << std::endl;
    out << ::nitrokey::misc::hexdump((const char *)(&pod), sizeof pod);

    out << "Contents:" << std::endl;
    out << "Command ID:\t" << commandid_to_string((CommandID)(pod.command_id))
        << std::endl;
      out << "CRC:\t"
        << std::hex << std::setw(2) << std::setfill('0')
        << pod.crc << std::endl;

      out << "Payload:" << std::endl;
    out << pod.payload.dissect();
    return out.str();
  }
};




template <CommandID id, class HIDPacket>
class ResponseDissector : semantics::non_constructible {
 public:
    static std::string status_translate_device(int status){
      auto enum_status = static_cast<proto::NKPro::device_status>(status);
      switch (enum_status){
        case NKPro::device_status::ok: return "OK";
        case NKPro::device_status::busy: return "BUSY";
        case NKPro::device_status::error: return "ERROR";
        case NKPro::device_status::received_report: return "RECEIVED_REPORT";
      }
      return std::string("UNKNOWN: ") + std::to_string(status);
    }

    static std::string to_upper(std::string str){
        for (auto & c: str) c = toupper(c);
      return str;
    }
    static std::string status_translate_command(int status){
      auto enum_status = static_cast<proto::NKPro::command_status >(status);
      switch (enum_status) {
#define p(X) case X: return to_upper(std::string(#X));
        p(NKPro::command_status::ok)
        p(NKPro::command_status::wrong_CRC)
        p(NKPro::command_status::wrong_slot)
        p(NKPro::command_status::slot_not_programmed)
        p(NKPro::command_status::wrong_password)
        p(NKPro::command_status::not_authorized)
        p(NKPro::command_status::timestamp_warning)
        p(NKPro::command_status::no_name_error)
        p(NKPro::command_status::not_supported)
        p(NKPro::command_status::unknown_command)
        p(NKPro::command_status::AES_dec_failed)
#undef p
      }
      return std::string("UNKNOWN: ") + std::to_string(status);
    }

  static std::string dissect(const HIDPacket &pod) {
    std::stringstream out;

    // FIXME use values from firmware (possibly generate separate
    // header automatically)

    out << "Raw HID packet:" << std::endl;
    out << ::nitrokey::misc::hexdump((const char *)(&pod), sizeof pod);

    out << "Device status:\t" << pod.device_status + 0 << " "
        << status_translate_device(pod.device_status) << std::endl;
    out << "Command ID:\t" << commandid_to_string((CommandID)(pod.command_id)) << " hex: " << std::hex << (int)pod.command_id
        << std::endl;
    out << "Last command CRC:\t"
            << std::hex << std::setw(2) << std::setfill('0')
            << pod.last_command_crc << std::endl;
    out << "Last command status:\t" << pod.last_command_status + 0 << " "
        << status_translate_command(pod.last_command_status) << std::endl;
    out << "CRC:\t"
            << std::hex << std::setw(2) << std::setfill('0')
            << pod.crc << std::endl;
    if((int)pod.command_id == pod.storage_status.command_id){
      out << "Storage stick status (where applicable):" << std::endl;
#define d(x) out << " "#x": \t"<< std::hex << std::setw(2) \
    << std::setfill('0')<< static_cast<int>(x) << std::endl;
    d(pod.storage_status.command_counter);
    d(pod.storage_status.command_id);
    d(pod.storage_status.device_status);
    d(pod.storage_status.progress_bar_value);
#undef d
    }

    out << "Payload:" << std::endl;
    out << pod.payload.dissect();
    return out.str();
  }
};
}
}

#endif
