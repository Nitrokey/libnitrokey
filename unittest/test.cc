#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch.hpp"

#include <iostream>
#include <string.h>
#include "device_proto.h"
#include "log.h"
#include "stick10_commands.h"

using namespace std;
using namespace nitrokey::device;
using namespace nitrokey::proto::stick10;
using namespace nitrokey::log;

//int main() {

std::string getSlotName(Stick10& stick, int slotNo){
        ReadSlot::CommandTransaction::CommandPayload slot_req;
        slot_req.slot_number = slotNo;
        auto slot = ReadSlot::CommandTransaction::run(stick, slot_req);
        std::string sName(reinterpret_cast<char*>(slot.slot_name));
        return sName;
}

TEST_CASE( "Slot names are correct", "[slotNames]" ) {
	Stick10 stick;

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
	//for (int i=0x20; i<0x23; i++) {
	//	ReadSlot::CommandTransaction::CommandPayload slot_req;
	//	slot_req.slot_number = i;
	//	auto slot = ReadSlot::CommandTransaction::run(stick, slot_req);
	//}
            REQUIRE( getSlotName(stick, 0x20) == std::string("slot1")  ); 
            REQUIRE( getSlotName(stick, 0x21) == std::string("slot2")  ); 
            REQUIRE( getSlotName(stick, 0x22) == std::string("slot3")  ); 
	}
        stick.disconnect();
}
