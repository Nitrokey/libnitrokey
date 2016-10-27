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


#include "test_command_ids_header.h"

TEST_CASE("test device commands ids", "[fast]") {

//  REQUIRE(STICK20_CMD_START_VALUE == static_cast<uint8_t>(CommandID::START_VALUE));
  REQUIRE(STICK20_CMD_ENABLE_CRYPTED_PARI == static_cast<uint8_t>(CommandID::ENABLE_CRYPTED_PARI));
  REQUIRE(STICK20_CMD_DISABLE_CRYPTED_PARI == static_cast<uint8_t>(CommandID::DISABLE_CRYPTED_PARI));
  REQUIRE(STICK20_CMD_ENABLE_HIDDEN_CRYPTED_PARI == static_cast<uint8_t>(CommandID::ENABLE_HIDDEN_CRYPTED_PARI));
  REQUIRE(STICK20_CMD_DISABLE_HIDDEN_CRYPTED_PARI == static_cast<uint8_t>(CommandID::DISABLE_HIDDEN_CRYPTED_PARI));
  REQUIRE(STICK20_CMD_ENABLE_FIRMWARE_UPDATE == static_cast<uint8_t>(CommandID::ENABLE_FIRMWARE_UPDATE));
  REQUIRE(STICK20_CMD_EXPORT_FIRMWARE_TO_FILE == static_cast<uint8_t>(CommandID::EXPORT_FIRMWARE_TO_FILE));
  REQUIRE(STICK20_CMD_GENERATE_NEW_KEYS == static_cast<uint8_t>(CommandID::GENERATE_NEW_KEYS));
  REQUIRE(STICK20_CMD_FILL_SD_CARD_WITH_RANDOM_CHARS == static_cast<uint8_t>(CommandID::FILL_SD_CARD_WITH_RANDOM_CHARS));

  REQUIRE(STICK20_CMD_WRITE_STATUS_DATA == static_cast<uint8_t>(CommandID::WRITE_STATUS_DATA));
  REQUIRE(STICK20_CMD_ENABLE_READONLY_UNCRYPTED_LUN == static_cast<uint8_t>(CommandID::ENABLE_READONLY_UNCRYPTED_LUN));
  REQUIRE(STICK20_CMD_ENABLE_READWRITE_UNCRYPTED_LUN == static_cast<uint8_t>(CommandID::ENABLE_READWRITE_UNCRYPTED_LUN));

  REQUIRE(STICK20_CMD_SEND_PASSWORD_MATRIX == static_cast<uint8_t>(CommandID::SEND_PASSWORD_MATRIX));
  REQUIRE(STICK20_CMD_SEND_PASSWORD_MATRIX_PINDATA == static_cast<uint8_t>(CommandID::SEND_PASSWORD_MATRIX_PINDATA));
  REQUIRE(STICK20_CMD_SEND_PASSWORD_MATRIX_SETUP == static_cast<uint8_t>(CommandID::SEND_PASSWORD_MATRIX_SETUP));

  REQUIRE(STICK20_CMD_GET_DEVICE_STATUS == static_cast<uint8_t>(CommandID::GET_DEVICE_STATUS));
  REQUIRE(STICK20_CMD_SEND_DEVICE_STATUS == static_cast<uint8_t>(CommandID::SEND_DEVICE_STATUS));

  REQUIRE(STICK20_CMD_SEND_HIDDEN_VOLUME_PASSWORD == static_cast<uint8_t>(CommandID::SEND_HIDDEN_VOLUME_PASSWORD));
  REQUIRE(STICK20_CMD_SEND_HIDDEN_VOLUME_SETUP == static_cast<uint8_t>(CommandID::SEND_HIDDEN_VOLUME_SETUP));
  REQUIRE(STICK20_CMD_SEND_PASSWORD == static_cast<uint8_t>(CommandID::SEND_PASSWORD));
  REQUIRE(STICK20_CMD_SEND_NEW_PASSWORD == static_cast<uint8_t>(CommandID::SEND_NEW_PASSWORD));
  REQUIRE(STICK20_CMD_CLEAR_NEW_SD_CARD_FOUND == static_cast<uint8_t>(CommandID::CLEAR_NEW_SD_CARD_FOUND));

  REQUIRE(STICK20_CMD_SEND_STARTUP == static_cast<uint8_t>(CommandID::SEND_STARTUP));
  REQUIRE(STICK20_CMD_SEND_CLEAR_STICK_KEYS_NOT_INITIATED == static_cast<uint8_t>(CommandID::SEND_CLEAR_STICK_KEYS_NOT_INITIATED));
  REQUIRE(STICK20_CMD_SEND_LOCK_STICK_HARDWARE == static_cast<uint8_t>(CommandID::SEND_LOCK_STICK_HARDWARE));

  REQUIRE(STICK20_CMD_PRODUCTION_TEST == static_cast<uint8_t>(CommandID::PRODUCTION_TEST));
  REQUIRE(STICK20_CMD_SEND_DEBUG_DATA == static_cast<uint8_t>(CommandID::SEND_DEBUG_DATA));

  REQUIRE(STICK20_CMD_CHANGE_UPDATE_PIN == static_cast<uint8_t>(CommandID::CHANGE_UPDATE_PIN));

}

TEST_CASE("test device internal status with various commands", "[fast]") {
  Stick20 stick;
  bool connected = stick.connect();
  REQUIRE(connected == true);

  Log::instance().set_loglevel(Loglevel::DEBUG_L2);
  auto p = get_payload<stick20::SendStartup::CommandPayload>();
  p.set_defaults();
  auto device_status = stick20::SendStartup::CommandTransaction::run(stick, p);
  REQUIRE(device_status.data().AdminPwRetryCount == 3);
  REQUIRE(device_status.data().UserPwRetryCount == 3);
  REQUIRE(device_status.data().ActiveSmartCardID_u32 != 0);

  auto production_status = stick20::ProductionTest::CommandTransaction::run(stick);
  REQUIRE(production_status.data().SD_Card_Size_u8 == 8);
  REQUIRE(production_status.data().SD_CardID_u32 != 0);
}

TEST_CASE("general test", "[test]") {
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
