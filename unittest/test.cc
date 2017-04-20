#include "catch.hpp"

#include <iostream>
#include <string.h>
#include "device_proto.h"
#include "log.h"
#include "NKPro_commands.h"

using namespace std;
using namespace nitrokey::device;
using namespace nitrokey::proto::NKPro;
using namespace nitrokey::log;
using namespace nitrokey::misc;

using Dev10 = std::shared_ptr<Device_NKPro>;

std::string getSlotName(Dev10 stick, int slotNo) {
  auto slot_req = get_payload<ReadSlot>();
  slot_req.slot_number = slotNo;
  auto slot = ReadSlot::CommandTransaction::run(stick, slot_req);
  std::string sName(reinterpret_cast<char *>(slot.data().slot_name));
  return sName;
}

TEST_CASE("Slot names are correct", "[slotNames]") {
  auto stick = make_shared<Device_NKPro>();
  bool connected = stick->connect();
  REQUIRE(connected == true);

  Log::instance().set_loglevel(Loglevel::DEBUG);

  auto resp = GetStatus::CommandTransaction::run(stick);

  auto authreq = get_payload<FirstAuthenticate>();
  strcpy((char *)(authreq.card_password), "12345678");
  FirstAuthenticate::CommandTransaction::run(stick, authreq);

  {
    auto authreq = get_payload<EnablePasswordSafe>();
    strcpy((char *)(authreq.user_password), "123456");
    EnablePasswordSafe::CommandTransaction::run(stick, authreq);
  }

  //assuming these values were set earlier, thus failing on normal use
  REQUIRE(getSlotName(stick, 0x20) == std::string("1"));
  REQUIRE(getSlotName(stick, 0x21) == std::string("slot2"));

  {
    auto resp = GetPasswordRetryCount::CommandTransaction::run(stick);
    REQUIRE(resp.data().password_retry_count == 3);
  }
  {
    auto resp = GetUserPasswordRetryCount::CommandTransaction::run(stick);
    REQUIRE(resp.data().password_retry_count == 3);
  }

  {
    auto slot = get_payload<GetPasswordSafeSlotName>();
    slot.slot_number = 0;
    auto resp2 = GetPasswordSafeSlotName::CommandTransaction::run(stick, slot);
    std::string sName(reinterpret_cast<char *>(resp2.data().slot_name));
    REQUIRE(sName == std::string("web1"));
  }

  {
    auto slot = get_payload<GetPasswordSafeSlotPassword>();
    slot.slot_number = 0;
    auto resp2 =
        GetPasswordSafeSlotPassword::CommandTransaction::run(stick, slot);
    std::string sName(reinterpret_cast<char *>(resp2.data().slot_password));
    REQUIRE(sName == std::string("pass1"));
  }

  {
    auto slot = get_payload<GetPasswordSafeSlotLogin>();
    slot.slot_number = 0;
    auto resp2 = GetPasswordSafeSlotLogin::CommandTransaction::run(stick, slot);
    std::string sName(reinterpret_cast<char *>(resp2.data().slot_login));
    REQUIRE(sName == std::string("login1"));
  }

  stick->disconnect();
}
