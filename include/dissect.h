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


                // FIXME use values from firmware (possibly generate separate 
                // header automatically)
                std::string status[4];
                status[0] = " STATUS_READY";
                status[1]= " STATUS_BUSY";
                status[2]= " STATUS_ERROR";
                status[3]= " STATUS_RECEIVED_REPORT";
                std::string cmd[11];
                cmd[0]= " CMD_STATUS_OK";
                cmd[1]= " CMD_STATUS_WRONG_CRC";
                cmd[2]= " CMD_STATUS_WRONG_SLOT";
                cmd[3]= " CMD_STATUS_SLOT_NOT_PROGRAMMED";
                cmd[4]= " CMD_STATUS_WRONG_PASSWORD";
                cmd[5]= " CMD_STATUS_NOT_AUTHORIZED";
                cmd[6]= " CMD_STATUS_TIMESTAMP_WARNING";
                cmd[7]= " CMD_STATUS_NO_NAME_ERROR";
                cmd[8]= " CMD_STATUS_NOT_SUPPORTED";
                cmd[9]= " CMD_STATUS_UNKNOWN_COMMAND";
                cmd[10]= " CMD_STATUS_AES_DEC_FAILED";
                 

		out << "Raw HID packet:" << std::endl;
		out << ::nitrokey::misc::hexdump((const char *)(&pod), sizeof pod);

		out << "Device status:\t" << pod.device_status + 0<<" "<< cmd[ pod.device_status ] << std::endl;
		out << "Command ID:\t" << commandid_to_string((CommandID)(pod.command_id)) << std::endl;
		out << "Last command CRC:\t" << pod.last_command_crc << std::endl;
		out << "Last command status:\t" << pod.last_command_status + 0<<" "<<status[pod.last_command_status] << std::endl;
		out << "CRC:\t" << pod.crc << std::endl;

		out << "Payload:" << std::endl;
		out << pod.payload.dissect();
		return out.str();
	}
};


}
}

#endif
