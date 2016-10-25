#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch.hpp"

#include <iostream>
#include <string.h>
#include <NitrokeyManager.h>
#include "device_proto.h"
#include "log.h"
//#include "stick10_commands.h"
#include "stick20_commands.h"

using namespace std;
using namespace nitrokey::device;
using namespace nitrokey::proto;
using namespace nitrokey::proto::stick20;
using namespace nitrokey::log;
using namespace nitrokey::misc;


template<typename CMDTYPE>
void execute_password_command(Device &stick, const char *password, const char kind = 'P') {
  auto p = get_payload<CMDTYPE>();
  if (kind == 'P'){
    p.set_kind_user();
  } else {
    p.set_kind_admin();
  }
  strcpyT(p.password, password);
  CMDTYPE::CommandTransaction::run(stick, p);
  this_thread::sleep_for(1000ms);
}


TEST_CASE("long operation test", "[test_long]") {
  Stick20 stick;
  bool connected = stick.connect();
  REQUIRE(connected == true);
  Log::instance().set_loglevel(Loglevel::DEBUG_L2);
  try{
//    execute_password_command<FillSDCardWithRandomChars>(stick, "12345678", 'P');
    auto p = get_payload<FillSDCardWithRandomChars>();
    p.set_defaults();
    strcpyT(p.password, "12345678");
    FillSDCardWithRandomChars::CommandTransaction::run(stick, p);
    this_thread::sleep_for(1000ms);

    CHECK(false);
  }
  catch (LongOperationInProgressException &progressException){
    CHECK(true);
  }


  for (int i = 0; i < 30; ++i) {
    try {
      stick10::GetStatus::CommandTransaction::run(stick);
    }
    catch (LongOperationInProgressException &progressException){
      CHECK((int)progressException.progress_bar_value>=0);
      CAPTURE((int)progressException.progress_bar_value);
      this_thread::sleep_for(2000ms);
    }

  }

}

TEST_CASE("test", "[test]") {
  Stick20 stick;
  bool connected = stick.connect();
  REQUIRE(connected == true);

  Log::instance().set_loglevel(Loglevel::DEBUG_L2);

  stick10::LockDevice::CommandTransaction::run(stick);
//  execute_password_command<EnableEncryptedPartition>(stick, "123456");
//  execute_password_command<DisableEncryptedPartition>(stick, "123456");
  this_thread::sleep_for(1000ms);
  execute_password_command<EnableEncryptedPartition>(stick, "123456");
  this_thread::sleep_for(4000ms);
  bool passed = false;
  for(int i=0; i<5; i++){
    try {
      execute_password_command<EnableHiddenEncryptedPartition>(stick, "123123123");
      CHECK(true);
      passed=true;
      break;
    }
    catch (CommandFailedException &e){
      this_thread::sleep_for(2000ms);
    }
  }
  if(!passed){
    CHECK(false);
  }

  execute_password_command<DisableHiddenEncryptedPartition>(stick, "123123123");
  execute_password_command<SendSetReadonlyToUncryptedVolume>(stick, "123456");
  execute_password_command<SendSetReadwriteToUncryptedVolume>(stick, "123456");
  execute_password_command<SendClearNewSdCardFound>(stick, "12345678", 'A');
  stick20::GetDeviceStatus::CommandTransaction::run(stick);
  this_thread::sleep_for(1000ms);
//  execute_password_command<LockFirmware>(stick, "123123123"); //CAUTION
//  execute_password_command<EnableFirmwareUpdate>(stick, "123123123"); //CAUTION FIRMWARE PIN

  execute_password_command<ExportFirmware>(stick, "12345678", 'A');
//  execute_password_command<FillSDCardWithRandomChars>(stick, "12345678", 'A');

  stick10::LockDevice::CommandTransaction::run(stick);
}
