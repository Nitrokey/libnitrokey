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
	Stick20 stick;

	Log::instance().set_loglevel(Loglevel::DEBUG);

	{
		auto resp = GetStatus::CommandTransaction::run(stick);
	}

	{
		FirstAuthenticate::CommandTransaction::CommandPayload authreq;
		strcpy((char *)(authreq.card_password), "12345678");
		FirstAuthenticate::CommandTransaction::run(stick, authreq);
	}

	{
		for (int i=0; i<10; i++) {
			ReadSlot::CommandTransaction::CommandPayload slot_req;
			slot_req.slot_number = i;
			auto slot = ReadSlot::CommandTransaction::run(stick, slot_req);
		}
	}
}
