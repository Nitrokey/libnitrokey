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
#include "stick10_commands_0.8.h"
//#include "stick20_commands.h"

using namespace std;
using namespace nitrokey::device;
using namespace nitrokey::proto;
using namespace nitrokey::proto::stick10_08;
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

  auto user_auth = get_payload<UserAuthenticate>();
  strcpyT(user_auth.temporary_password, temporary_password);
  strcpyT(user_auth.card_password, default_user_pin);
  UserAuthenticate::CommandTransaction::run(stick, user_auth);
}

TEST_CASE("write slot", "[pronew]"){
  Stick10 stick;
  connect_and_setup(stick);
  authorize(stick);

  auto p = get_payload<WriteToHOTPSlot>();
  strcpyT(p.slot_secret, RFC_SECRET);
  strcpyT(p.temporary_admin_password, temporary_password);
  p.use_8_digits = true;
  stick10_08::WriteToHOTPSlot::CommandTransaction::run(stick, p);

  auto p2 = get_payload<WriteToHOTPSlot_2>();
  strcpyT(p2.temporary_admin_password, temporary_password);
  p2.slot_number = 0 + 0x10;
  p2.slot_counter = 0;
  strcpyT(p2.slot_name, "test name aaa");
  stick10_08::WriteToHOTPSlot_2::CommandTransaction::run(stick, p2);

  auto pc = get_payload<WriteGeneralConfig>();
  pc.enable_user_password = 0;
  strcpyT(pc.temporary_admin_password, temporary_password);
  WriteGeneralConfig::CommandTransaction::run(stick, pc);

  auto p3 = get_payload<GetHOTP>();
  p3.slot_number = 0 + 0x10;
  GetHOTP::CommandTransaction::run(stick, p3);
  
}


TEST_CASE("erase slot", "[pronew]"){
  Stick10 stick;
  connect_and_setup(stick);
  authorize(stick);

  auto p = get_payload<WriteGeneralConfig>();
  p.enable_user_password = 0;
  strcpyT(p.temporary_admin_password, temporary_password);
  WriteGeneralConfig::CommandTransaction::run(stick, p);

  auto p3 = get_payload<GetHOTP>();
  p3.slot_number = 0 + 0x10;
  GetHOTP::CommandTransaction::run(stick, p3);

  auto erase_payload = get_payload<EraseSlot>();
  erase_payload.slot_number = 0 + 0x10;
  strcpyT(erase_payload.temporary_admin_password, temporary_password);
  EraseSlot::CommandTransaction::run(stick, erase_payload);

  auto p4 = get_payload<GetHOTP>();
  p4.slot_number = 0 + 0x10;
  REQUIRE_THROWS(
      GetHOTP::CommandTransaction::run(stick, p4)
  );
}

TEST_CASE("write general config", "[pronew]") {
  Stick10 stick;
  connect_and_setup(stick);
  authorize(stick);

  auto p = get_payload<WriteGeneralConfig>();
  p.enable_user_password = 1;
  REQUIRE_THROWS(
      WriteGeneralConfig::CommandTransaction::run(stick, p);
  );
  strcpyT(p.temporary_admin_password, temporary_password);
  WriteGeneralConfig::CommandTransaction::run(stick, p);
}

TEST_CASE("authorize user HOTP", "[pronew]") {
  Stick10 stick;
  connect_and_setup(stick);
  authorize(stick);

  auto p = get_payload<WriteGeneralConfig>();
  p.enable_user_password = 1;
  strcpyT(p.temporary_admin_password, temporary_password);
  WriteGeneralConfig::CommandTransaction::run(stick, p);

  auto pw = get_payload<WriteToHOTPSlot>();
  strcpyT(pw.slot_secret, RFC_SECRET);
  strcpyT(pw.temporary_admin_password, temporary_password);
  pw.use_8_digits = true;
  WriteToHOTPSlot::CommandTransaction::run(stick, pw);

  auto pw2 = get_payload<WriteToHOTPSlot_2>();
  strcpyT(pw2.temporary_admin_password, temporary_password);
  pw2.slot_number = 0 + 0x10;
  pw2.slot_counter = 0;
  strcpyT(pw2.slot_name, "test name aaa");
  WriteToHOTPSlot_2::CommandTransaction::run(stick, pw2);

  auto p3 = get_payload<GetHOTP>();
  p3.slot_number = 0 + 0x10;
  REQUIRE_THROWS(
      GetHOTP::CommandTransaction::run(stick, p3);
  );
  strcpyT(p3.temporary_user_password, temporary_password);
  auto code_response = GetHOTP::CommandTransaction::run(stick, p3);
  REQUIRE(code_response.data().code == 84755224);

}


TEST_CASE("authorize user TOTP", "[pronew]") {
  Stick10 stick;
  connect_and_setup(stick);
  authorize(stick);

  auto p = get_payload<WriteGeneralConfig>();
  p.enable_user_password = 1;
  strcpyT(p.temporary_admin_password, temporary_password);
  WriteGeneralConfig::CommandTransaction::run(stick, p);

  auto pw = get_payload<WriteToTOTPSlot>();
  strcpyT(pw.slot_secret, RFC_SECRET);
  strcpyT(pw.temporary_admin_password, temporary_password);
  pw.use_8_digits = true;
  WriteToTOTPSlot::CommandTransaction::run(stick, pw);

  auto pw2 = get_payload<WriteToTOTPSlot_2>();
  strcpyT(pw2.temporary_admin_password, temporary_password);
  pw2.slot_number = 0 + 0x20;
  pw2.slot_interval= 30;
  strcpyT(pw2.slot_name, "test name TOTP");
  WriteToTOTPSlot_2::CommandTransaction::run(stick, pw2);

  auto p_get_totp = get_payload<GetTOTP>();
  p_get_totp.slot_number = 0 + 0x20;

  REQUIRE_THROWS(
      GetTOTP::CommandTransaction::run(stick, p_get_totp);
  );
  strcpyT(p_get_totp.temporary_user_password, temporary_password);

  auto p_set_time = get_payload<SetTime>();
  p_set_time.reset = 1;
  p_set_time.time = 59;
  SetTime::CommandTransaction::run(stick, p_set_time);
  auto code = GetTOTP::CommandTransaction::run(stick, p_get_totp);
  REQUIRE(code.data().code == 94287082);

}