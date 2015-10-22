#include <iostream>
#include <string.h>
#include "device_proto.h"
#include "device.h"

using namespace std;

using namespace device;
using namespace proto;
using namespace proto::stick10::command;

int main() {
	Stick20 stick;
	cout << stick.connect() << endl;

	{
		auto resp = GetStatus::CommandTransaction::run(stick);
		cout << resp.firmware_version << endl;
	}

	{
		FirstAuthenticate::CommandTransaction::CommandPayload authreq;
		strcpy((char *)(authreq.card_password), "12345678");
		FirstAuthenticate::CommandTransaction::run(stick, authreq);
	}

	{
	for (int i=0; i<32; i++) {
		GetSlotName::CommandTransaction::CommandPayload slotname_req;
		slotname_req.slot_number=i;
		auto slot_resp = GetSlotName::CommandTransaction::run(stick);
		cout << slot_resp.slot_name << endl;
	}
	}
}
