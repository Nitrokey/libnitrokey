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

std::string getSlotName(Stick10 &stick, int slotNo) {
  ReadSlot::CommandTransaction::CommandPayload slot_req;
  slot_req.slot_number = slotNo;
  auto slot = ReadSlot::CommandTransaction::run(stick, slot_req);
  std::string sName(reinterpret_cast<char *>(slot.slot_name));
  return sName;
}

void setSecret (uint8_t slot_secret[]){}; 

TEST_CASE("Slot names are correct", "[slotNames]") {
  Stick10 stick;
  bool connected = stick.connect();
  REQUIRE(connected == true);

  Log::instance().set_loglevel(Loglevel::DEBUG);

  auto resp = GetStatus::CommandTransaction::run(stick);

  {
  FirstAuthenticate::CommandTransaction::CommandPayload authreq;
  strcpy((char *)(authreq.card_password), "12345678");
  FirstAuthenticate::CommandTransaction::run(stick, authreq);
  }

  {
    WriteToHOTPSlot::CommandTransaction::CommandPayload hwrite;
    hwrite.slot_number = 0;
    strcpy(hwrite.slot_name, "rfc_test");
    strcpy(hwrite.slot_secret, "");
    hwrite.slot_config;
    strcpy(hwrite.slot_token_id, "");
    strcpy(hwrite.slot_counter, "");
  }


  stick.disconnect();
}
