/*
 *	Protocol packet dissection
 */
#ifndef DISSECT_H
#define DISSECT_H
#include <string>
#include <sstream>
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
		out << "Command ID:\t" << commandid_to_string((CommandID)(pod.command_id)) << std::endl;
		out << "CRC:\t" << pod.crc << std::endl;

		out << "Payload:" << std::endl;
		out << pod.payload.dissect();
		return out.str();
	}
};

template <CommandID id, class HIDPacket>
class ResponseDissector : semantics::non_constructible {
public:
	static std::string dissect(const HIDPacket &pod) {
		std::stringstream out;

		out << "Raw HID packet:" << std::endl;
		out << ::nitrokey::misc::hexdump((const char *)(&pod), sizeof pod);

		out << "Device status:\t" << pod.device_status + 0 << std::endl;
		out << "Command ID:\t" << commandid_to_string((CommandID)(pod.command_id)) << std::endl;
		out << "Last command CRC:\t" << pod.last_command_crc << std::endl;
		out << "Last command status:\t" << pod.last_command_status + 0 << std::endl;
		out << "CRC:\t" << pod.crc << std::endl;

		out << "Payload:" << std::endl;
		out << pod.payload.dissect();
		return out.str();
	}
};


}
}

#endif
