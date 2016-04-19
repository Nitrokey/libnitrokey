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
#include "utils/crc32.h"

#define STICK20_UPDATE_MODE_VID 0x03EB
#define STICK20_UPDATE_MODE_PID 0x2FF1

#define PAYLOAD_SIZE 53
#define PWS_SLOT_COUNT 16
#define PWS_SLOTNAME_LENGTH 11
#define PWS_PASSWORD_LENGTH 20
#define PWS_LOGINNAME_LENGTH 32

#define PWS_SEND_PASSWORD     0
#define PWS_SEND_LOGINNAME    1
#define PWS_SEND_TAB          2
#define PWS_SEND_CR           3

namespace device {
	class Device;
	enum class CommError;
}

static inline uint32_t crc(const uint8_t *data, size_t size) {
	uint32_t crc = 0xffffffff;
	const uint32_t *pend = (const uint32_t *)(data + size);
	for (const uint32_t *p = (const uint32_t *)(data); p < pend; p++)
		crc = Crc32 (crc, *p);
	return crc;
}

namespace proto {
enum class CommandID : uint8_t {
	GET_STATUS                     = 0x00,
	WRITE_TO_SLOT                  = 0x01,
	READ_SLOT_NAME                 = 0x02,
	READ_SLOT                      = 0x03,
	GET_CODE                       = 0x04,
	WRITE_CONFIG                   = 0x05,
	ERASE_SLOT                     = 0x06,
	FIRST_AUTHENTICATE             = 0x07,
	AUTHORIZE                      = 0x08,
	GET_PASSWORD_RETRY_COUNT       = 0x09,
	CLEAR_WARNING                  = 0x0A,
	SET_TIME                       = 0x0B,
	TEST_COUNTER                   = 0x0C,
	TEST_TIME                      = 0x0D,
	USER_AUTHENTICATE              = 0x0E,
	GET_USER_PASSWORD_RETRY_COUNT  = 0x0F,
	USER_AUTHORIZE                 = 0x10,
	UNLOCK_USER_PASSWORD           = 0x11,
	LOCK_DEVICE                    = 0x12,
	FACTORY_RESET                  = 0x13,
	CHANGE_USER_PIN                = 0x14,
	CHANGE_ADMIN_PIN               = 0x15,

	ENABLE_CRYPTED_PARI = 0x20,
	DISABLE_CRYPTED_PARI,
	ENABLE_HIDDEN_CRYPTED_PARI,
	DISABLE_HIDDEN_CRYPTED_PARI,
	ENABLE_FIRMWARE_UPDATE,
	EXPORT_FIRMWARE_TO_FILE,
	GENERATE_NEW_KEYS,
	FILL_SD_CARD_WITH_RANDOM_CHARS,

	WRITE_STATUS_DATA,
	ENABLE_READONLY_UNCRYPTED_LUN,
	ENABLE_READWRITE_UNCRYPTED_LUN,

	SEND_PASSWORD_MATRIX,
	SEND_PASSWORD_MATRIX_PINDATA,
	SEND_PASSWORD_MATRIX_SETUP,

	GET_DEVICE_STATUS,
	SEND_DEVICE_STATUS,

	SEND_HIDDEN_VOLUME_PASSWORD,
	SEND_HIDDEN_VOLUME_SETUP,
	SEND_PASSWORD,
	SEND_NEW_PASSWORD,
	CLEAR_NEW_SD_CARD_FOUND,

	SEND_STARTUP,
	SEND_CLEAR_STICK_KEYS_NOT_INITIATED,
	SEND_LOCK_STICK_HARDWARE,

	PRODUCTION_TEST,
	SEND_DEBUG_DATA,

	CHANGE_UPDATE_PIN,

	GET_PW_SAFE_SLOT_STATUS			= 0x60,
	GET_PW_SAFE_SLOT_NAME      		= 0x61,
	GET_PW_SAFE_SLOT_PASSWORD  		= 0x62,
	GET_PW_SAFE_SLOT_LOGINNAME 		= 0x63,
	SET_PW_SAFE_SLOT_DATA_1    		= 0x64,
	SET_PW_SAFE_SLOT_DATA_2    		= 0x65,
	PW_SAFE_ERASE_SLOT         		= 0x66,
	PW_SAFE_ENABLE             		= 0x67,
	PW_SAFE_INIT_KEY           		= 0x68,
	PW_SAFE_SEND_DATA          		= 0x69,
	SD_CARD_HIGH_WATERMARK     		= 0x70,
	DETECT_SC_AES              		= 0x6a,
	NEW_AES_KEY                		= 0x6b
};

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
	CommandID command_id; // uint8_t
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
		return ::crc((const uint8_t *)(this) + 1,
					(size_t)(HID_REPORT_SIZE - 5));
	}

	void update_CRC() {
		crc = calculate_CRC();
	}

	bool isCRCcorrect() const {
		return crc == calculate_CRC();
	}

	bool isValid() const {
		return !_zero && payload.isValid() && isCRCcorrect();
	}
} __packed;

/*
 *	Response payload (the parametrized type inside struct HIDReport)
 *
 *	command_id member in incoming HIDReport structure carries the command
 *	type last used.
 */
template <typename ResponsePayload>
struct DeviceResponse {
	uint8_t _zero;
	uint8_t device_status;
	uint8_t last_command_type;
	uint32_t last_command_crc;
	uint8_t	last_command_status;
	union {
		uint8_t _padding[HID_REPORT_SIZE - 12];
		ResponsePayload payload;
	} __packed;
	uint32_t crc;

	void initialize() {
		bzero(this, sizeof *this);
	}

	uint32_t calculate_CRC() const {
		// w/o leading zero, a part of each HID packet
		// w/o 4-byte crc
		return ::crc((const uint8_t *)(this) + 1,
					(size_t)(HID_REPORT_SIZE - 5));
	}

	void update_CRC() {
		crc = calculate_CRC();
	}

	bool isCRCcorrect() const {
		return crc == calculate_CRC();
	}

	bool isValid() const {
		return !_zero && payload.isValid() && isCRCcorrect();
	}
} __packed;

struct EmptyPayload {
	uint8_t _data[];

	bool isValid() const {
		return true;
	}
} __packed;

template<CommandID cmd_id, typename command_payload,
							typename response_payload>
class Transaction : semantics::non_constructible {
public:
	// Types declared in command class scope can't be reached from there.
	typedef command_payload CommandPayload;
	typedef response_payload ResponsePayload;

	typedef struct HIDReport<cmd_id, CommandPayload> OutgoingPacket;
	typedef struct DeviceResponse<ResponsePayload> ResponsePacket;

	static_assert(std::is_pod<OutgoingPacket>::value,
					"OutgoingPacket must be a POD type");
	static_assert(std::is_pod<ResponsePacket>::value,
					"ResponsePacket must be a POD type");
	static_assert(sizeof(OutgoingPacket) == HID_REPORT_SIZE,
					"OutgoingPacket type is not the right size");
	static_assert(sizeof(ResponsePacket) == HID_REPORT_SIZE,
					"ResponsePacket type is not the right size");

	static response_payload run(device::Device &dev,
			const command_payload &payload) {
		using namespace device;

		CommError status;
		OutgoingPacket outp;
		ResponsePacket resp;

		// POD types can't have non-default constructors
		outp.initialize();
		resp.initialize();

		outp.payload = payload;
		outp.update_CRC();

		if (!outp.isValid())
			throw std::runtime_error("Invalid outgoing packet");

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

		if (!resp.isValid())
			throw std::runtime_error("Invalid incoming packet");

		// See: DeviceResponse
		return resp.payload;
	}

	static response_payload run(device::Device &dev) {
		command_payload empty_payload;
		return run(dev, empty_payload);
	}
};

/*
 *	Stick10 protocol definition
 */
namespace stick10 {
	namespace command {
		class GetSlotName : semantics::non_constructible {
		public:
			// reachable as a typedef in Transaction
			struct CommandPayload {
				uint8_t slot_number;

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			struct ResponsePayload {
				uint8_t slot_name[15];

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::READ_SLOT_NAME,
								struct CommandPayload,
								struct ResponsePayload> CommandTransaction;
		};

		class EraseSlot : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			typedef Transaction<CommandID::ERASE_SLOT,
								struct CommandPayload,
								struct EmptyPayload> CommandTransaction;
		};

		class SetTime : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t		reset; // 0 - get time, 1 - set time
				uint64_t	time; // posix time

				bool isValid() const {
					return reset && reset != 1;
				}
			} __packed;

			typedef Transaction<CommandID::SET_TIME,
								struct CommandPayload,
								struct EmptyPayload> CommandTransaction;
		};

		// TODO duplicate TOTP
		class WriteToHOTPSlot : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;
				uint8_t slot_name[15];
				uint8_t slot_secret[20];
				uint8_t slot_config;
				uint8_t slot_token_id[13];
				uint8_t slot_counter[8];

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			typedef Transaction<CommandID::WRITE_TO_SLOT,
							struct CommandPayload,
							struct EmptyPayload> CommandTransaction;
		};

		class WriteToTOTPSlot : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;
				uint8_t slot_name[15];
				uint8_t slot_secret[20];
				uint8_t slot_config;
				uint8_t slot_token_id[13];
				uint16_t slot_interval;

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			typedef Transaction<CommandID::WRITE_TO_SLOT,
								struct CommandPayload,
								struct EmptyPayload> CommandTransaction;
		};

		class GetCode : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;
				uint64_t challenge;
				uint64_t last_totp_time;
				uint8_t	last_interval;

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			struct ResponsePayload {
				uint8_t code[18];

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::GET_CODE,
								struct CommandPayload,
								struct ResponsePayload> CommandTransaction;
		};

		class GetHOTP : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			typedef Transaction<CommandID::GET_CODE,
								struct CommandPayload,
								struct EmptyPayload> CommandTransaction;
		};

		class ReadSlot : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			struct ResponsePayload {
				uint8_t slot_name[15];
				uint8_t config;
				uint8_t token_id[13];
				uint64_t counter;

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::READ_SLOT,
								struct CommandPayload,
								struct ResponsePayload> CommandTransaction;
		};

		class GetStatus : semantics::non_constructible {
		public:
			struct ResponsePayload {
				uint16_t firmware_version;
				uint8_t	card_serial[4];
				uint8_t	general_config[3];
				uint8_t	otp_password_config[2];

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::GET_STATUS,
								struct EmptyPayload,
								struct ResponsePayload> CommandTransaction;
		};

		class GetPasswordRetryCount : semantics::non_constructible {
		public:
			struct ResponsePayload {
				uint8_t password_retry_count;

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::GET_PASSWORD_RETRY_COUNT,
								struct EmptyPayload,
								struct ResponsePayload> CommandTransaction;
		};

		class GetUserPasswordRetryCount : semantics::non_constructible {
		public:
			struct ResponsePayload {
				uint8_t password_retry_count;

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::GET_USER_PASSWORD_RETRY_COUNT,
								struct EmptyPayload,
								struct ResponsePayload> CommandTransaction;
		};

		class GetPasswordSafeSlotStatus : semantics::non_constructible {
		public:
			struct ResponsePayload {
				uint8_t password_safe_status[PWS_SLOT_COUNT];

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::GET_PW_SAFE_SLOT_STATUS,
								struct EmptyPayload,
								struct ResponsePayload> CommandTransaction;
		};

		class GetPasswordSafeSlotName : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			struct ResponsePayload {
				uint8_t slot_name[PWS_SLOTNAME_LENGTH];

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::GET_PW_SAFE_SLOT_NAME,
								struct CommandPayload,
								struct ResponsePayload> CommandTransaction;
		};

		class GetPasswordSafeSlotPassword : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			struct ResponsePayload {
				uint8_t slot_password[PWS_PASSWORD_LENGTH];

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::GET_PW_SAFE_SLOT_PASSWORD,
								struct CommandPayload,
								struct ResponsePayload> CommandTransaction;
		};

		class GetPasswordSafeSlotLogin : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			struct ResponsePayload {
				uint8_t slot_login[PWS_LOGINNAME_LENGTH];

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::GET_PW_SAFE_SLOT_LOGINNAME,
								struct CommandPayload,
								struct ResponsePayload> CommandTransaction;
		};

		class SetPasswordSafeSlotData : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;
				uint8_t slot_name[PWS_SLOTNAME_LENGTH];
				uint8_t slot_password[PWS_PASSWORD_LENGTH];

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			typedef Transaction<CommandID::SET_PW_SAFE_SLOT_DATA_1,
								struct CommandPayload,
								struct EmptyPayload> CommandTransaction;
		};

		class SetPasswordSafeSlotData2 : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;
				uint8_t slot_name[PWS_SLOTNAME_LENGTH];

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			typedef Transaction<CommandID::SET_PW_SAFE_SLOT_DATA_2,
								struct CommandPayload,
								struct EmptyPayload> CommandTransaction;
		};

		class ErasePasswordSafeSlot : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			typedef Transaction<CommandID::PW_SAFE_ERASE_SLOT,
								struct CommandPayload,
								struct EmptyPayload> CommandTransaction;
		};

		class EnablePasswordSafe : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[30];

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::PW_SAFE_ENABLE,
								struct CommandPayload,
								struct EmptyPayload> CommandTransaction;
		};

		class PasswordSafeInitKey : semantics::non_constructible {
		public:
			typedef Transaction<CommandID::PW_SAFE_INIT_KEY,
								struct EmptyPayload,
								struct EmptyPayload> CommandTransaction;
		};

		// TODO naming screwed up, see above
		class PasswordSafeSendSlotViaHID: semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t slot_number;
				uint8_t slot_kind;

				bool isValid() const {
					return !(slot_number & 0xF0);
				}
			} __packed;

			typedef Transaction<CommandID::PW_SAFE_SEND_DATA,
								struct CommandPayload,
								struct EmptyPayload> CommandTransaction;
		};


		// TODO "Device::passwordSafeSendSlotDataViaHID"

		class WriteGeneralConfig : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t	config[5];
			} __packed;

			typedef Transaction<CommandID::WRITE_CONFIG,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class FirstAuthenticate : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t card_password[25];
				uint8_t temporary_password[25];

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::FIRST_AUTHENTICATE,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class UserAuthenticate : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t card_password[25];
				uint8_t temporary_password[25];

				bool isValid() const {
					return true;
				}
			} __packed;

			typedef Transaction<CommandID::USER_AUTHENTICATE,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class Authorize : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t crc[4];
				uint8_t password[25];
			} __packed;

			typedef Transaction<CommandID::AUTHORIZE,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class UserAuthorize : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t crc[4];
				uint8_t password[25];
			} __packed;

			typedef Transaction<CommandID::USER_AUTHORIZE,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class UnlockUserPassword : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t admin_password[20]; // TODO
			} __packed;

			// TODO could we get the stick to return the retry count?

			typedef Transaction<CommandID::UNLOCK_USER_PASSWORD,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class ChangeUserPin : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t old_pin[25];
				uint8_t new_pin[25];
			} __packed;

			typedef Transaction<CommandID::CHANGE_USER_PIN,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		// TODO why is it needed?
		class IsAESSupported : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[20];
			} __packed;

			typedef Transaction<CommandID::DETECT_SC_AES,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class ChangeAdminPin : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t old_pin[25];
				uint8_t new_pin[25];
			} __packed;

			typedef Transaction<CommandID::CHANGE_ADMIN_PIN,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class LockDevice : semantics::non_constructible {
		public:
			typedef Transaction<CommandID::LOCK_DEVICE,
									struct EmptyPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class FactoryReset : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[20];
			} __packed;

			typedef Transaction<CommandID::FACTORY_RESET,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class BuildAESKey : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[20];
			} __packed;

			typedef Transaction<CommandID::NEW_AES_KEY,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};
	}
}

/*
*	STICK20 protocol command ids
*	a superset of STICK10
*/
namespace stick20 {
	namespace command {
		class EnableEncryptedPartition : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[30]; // TODO check w/ firmware
			};

			typedef Transaction<CommandID::ENABLE_CRYPTED_PARI,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class DisableEncryptedPartition : semantics::non_constructible {
		public:
			typedef Transaction<CommandID::DISABLE_CRYPTED_PARI,
									struct EmptyPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class EnableHiddenEncryptedPartition : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[30]; // TODO check w/ firmware
			};

			typedef Transaction<CommandID::ENABLE_HIDDEN_CRYPTED_PARI,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class DisableHiddenEncryptedPartition : semantics::non_constructible {
		public:
			typedef Transaction<CommandID::DISABLE_CRYPTED_PARI,
									struct EmptyPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class EnableFirmwareUpdate : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[30]; // TODO check w/ firmware
			};

			typedef Transaction<CommandID::ENABLE_FIRMWARE_UPDATE,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class UpdatePassword : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t old_password[15];
				uint8_t new_password[15];
			};

			typedef Transaction<CommandID::CHANGE_UPDATE_PIN,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class ExportFirmware : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[30];
			};

			typedef Transaction<CommandID::EXPORT_FIRMWARE_TO_FILE,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class CreateNewKeys : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[30];
			};

			typedef Transaction<CommandID::GENERATE_NEW_KEYS,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class FillSDCardWithRandomChars : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t volume_flag;
				uint8_t password[30];
			};

			typedef Transaction<CommandID::FILL_SD_CARD_WITH_RANDOM_CHARS,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class SetupHiddenVolume : semantics::non_constructible {
		public:
			typedef Transaction<CommandID::SEND_HIDDEN_VOLUME_SETUP,
									struct EmptyPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class SendPasswordMatrix : semantics::non_constructible {
		public:
			typedef Transaction<CommandID::SEND_PASSWORD_MATRIX,
									struct EmptyPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class SendPasswordMatrixPinData : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t pin_data[30]; // TODO how long actually can it be?
			};

			typedef Transaction<CommandID::SEND_PASSWORD_MATRIX_PINDATA,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class SendPasswordMatrixSetup : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t setup_data[30]; // TODO how long actually can it be?
			};

			typedef Transaction<CommandID::SEND_PASSWORD_MATRIX_SETUP,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class GetDeviceStatus : semantics::non_constructible {
		public:
			typedef Transaction<CommandID::GET_DEVICE_STATUS,
									struct EmptyPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class SendPassword : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[30];
			};

			typedef Transaction<CommandID::SEND_PASSWORD,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class SendNewPassword : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[30];
			};

			typedef Transaction<CommandID::SEND_NEW_PASSWORD,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		// TODO fix original nomenclature
		class SendSetReadonlyToUncryptedVolume : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[30];
			};

			typedef Transaction<CommandID::ENABLE_READWRITE_UNCRYPTED_LUN,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class SendSetReadwriteToUncryptedVolume : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[30];
			};

			typedef Transaction<CommandID::ENABLE_READWRITE_UNCRYPTED_LUN,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class SendClearNewSdCardFound : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[30];
			};

			typedef Transaction<CommandID::CLEAR_NEW_SD_CARD_FOUND,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class SendStartup : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint64_t localtime; // POSIX
			};

			typedef Transaction<CommandID::SEND_STARTUP,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class SendHiddenVolumeSetup : semantics::non_constructible {
		public:
			struct CommandPayload {
				// TODO HiddenVolumeSetup_tst type
			};

			typedef Transaction<CommandID::SEND_HIDDEN_VOLUME_SETUP,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class LockFirmware : semantics::non_constructible {
		public:
			struct CommandPayload {
				uint8_t password[30];
			};

			typedef Transaction<CommandID::SEND_LOCK_STICK_HARDWARE,
									struct CommandPayload,
									struct EmptyPayload> CommandTransaction;
		};

		class ProductionTest : semantics::non_constructible {
		public:
			typedef Transaction<CommandID::PRODUCTION_TEST,
									struct EmptyPayload,
									struct EmptyPayload> CommandTransaction;
		};
	}
}
}

#endif
