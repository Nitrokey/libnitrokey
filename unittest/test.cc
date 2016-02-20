#include <iostream>
#include <string.h>
#include "device_proto.h"
#include "log.h"
#include "stick10_commands.h"

using namespace std;
using namespace nitrokey::device;
using namespace nitrokey::proto::stick10;
using namespace nitrokey::log;

int main() {
	Stick10 stick;
        cout << stick.connect() << endl;

	Log::instance().set_loglevel(Loglevel::DEBUG_L2);

	{
		auto resp = GetStatus::CommandTransaction::run(stick);
	}

	{
		FirstAuthenticate::CommandTransaction::CommandPayload authreq;
		strcpy((char *)(authreq.card_password), "12345678");
		FirstAuthenticate::CommandTransaction::run(stick, authreq);
	}

	{
		for (int i=0x20; i<0x23; i++) {
			ReadSlot::CommandTransaction::CommandPayload slot_req;
			slot_req.slot_number = i;
			auto slot = ReadSlot::CommandTransaction::run(stick, slot_req);
		}
	}
        stick.disconnect();
}
