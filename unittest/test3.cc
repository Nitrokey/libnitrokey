//
// Created by sz on 08.11.16.
//

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()

static const char *const default_admin_pin = "12345678";
static const char *const default_user_pin = "123456";
const char * temporary_password = "123456789012345678901234";
const char * RFC_SECRET = "12345678901234567890";

#include "catch.hpp"

#include <iostream>
#include <string.h>
#include <NitrokeyManager.h>
#include "device_proto.h"
#include "log.h"
#include "stick10_commands.h"
#include "stick10_commands_0.8.h"
//#include "stick20_commands.h"

using namespace std;
using namespace nitrokey::device;
using namespace nitrokey::proto;
//using namespace nitrokey::proto::stick10_08;
using namespace nitrokey::proto::stick10;
using namespace nitrokey::log;
using namespace nitrokey::misc;

void connect_and_setup(Stick10 &stick) {
  bool connected = stick.connect();
  REQUIRE(connected == true);
  Log::instance().set_loglevel(Loglevel::DEBUG);
}

void authorize(Stick10 &stick) {
  auto authreq = get_payload<FirstAuthenticate>();
  strcpy((char *) (authreq.card_password), default_admin_pin);
  strcpy((char *) (authreq.temporary_password), temporary_password);
  FirstAuthenticate::CommandTransaction::run(stick, authreq);
}

TEST_CASE("write slot", "[pronew]"){
  Stick10 stick;
  connect_and_setup(stick);

  auto p = get_payload<stick10_08::WriteToHOTPSlot>();
//  p.slot_number = 0 + 0x10;
  strcpyT(p.slot_secret, RFC_SECRET);
  strcpyT(p.temporary_admin_password, temporary_password);
  p.use_8_digits = true;
  stick10_08::WriteToHOTPSlot::CommandTransaction::run(stick, p);

  auto p2 = get_payload<stick10_08::WriteToHOTPSlot_2>();
  strcpyT(p2.temporary_admin_password, temporary_password);
  p2.slot_number = 0 + 0x10;
  p2.slot_counter = 0;
  strcpyT(p2.slot_name, "test name aaa");
  stick10_08::WriteToHOTPSlot_2::CommandTransaction::run(stick, p2);

  auto p3 = get_payload<GetHOTP>();
  p3.slot_number = 0 + 0x10;
  GetHOTP::CommandTransaction::run(stick, p3);
  
}


TEST_CASE("erase slot", "[pronew]"){
  Stick10 stick;
  connect_and_setup(stick);
  authorize(stick);

  auto erase_payload = get_payload<stick10_08::EraseSlot>();
  erase_payload.slot_number = 1 + 0x10;
  strcpyT(erase_payload.temporary_admin_password, temporary_password);
  stick10_08::EraseSlot::CommandTransaction::run(stick, erase_payload);
}