#include <chrono>
#include <thread>
#include <cstddef>
#include <stdexcept>
#include <hidapi/hidapi.h>
#include "device.h"
#include "log.h"
#include "misc.h"

using namespace nitrokey::device;
using namespace nitrokey::log;

Device::Device()
: m_vid(0), m_pid(0),
  m_retry_count(4),
  m_retry_timeout(50),
  mp_devhandle(NULL) {}

bool Device::connect() {
	Log::instance()(__PRETTY_FUNCTION__, Loglevel::DEBUG_L2);

	hid_init();
	mp_devhandle = hid_open(m_vid, m_pid, NULL);
	return mp_devhandle != NULL;
}

CommError Device::send(const void *packet) {
	Log::instance()(__PRETTY_FUNCTION__, Loglevel::DEBUG_L2);

	if (mp_devhandle == NULL)
		throw std::runtime_error("Attempted HID send on an invalid descriptor.");

	return (CommError)(
		hid_send_feature_report(mp_devhandle, (const unsigned char *)(packet),
			HID_REPORT_SIZE));
}

CommError Device::recv(void *packet) {
	CommError status;
	int retry_count = 0;

	Log::instance()(__PRETTY_FUNCTION__, Loglevel::DEBUG_L2);

	if (mp_devhandle == NULL)
		throw std::runtime_error("Attempted HID receive on an invalid descriptor.");

	for(;;) {
		status = (CommError)(
			hid_get_feature_report(mp_devhandle, (unsigned char *)(packet),
				HID_REPORT_SIZE));
		if (status == CommError::ERR_NO_ERROR || retry_count++ >= m_retry_count)
			break;
		std::this_thread::sleep_for(m_retry_timeout);
	}

	return status;
}

Stick10::Stick10() {
	m_vid = 0x20a0;
	m_pid = 0x4108;
}

Stick20::Stick20() {
	m_vid = 0x20a0;
	m_pid = 0x4109;
}
