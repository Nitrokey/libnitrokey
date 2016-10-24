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
void execute_password_command(Device &stick, const char *password) {
  auto p = get_payload<CMDTYPE>();
  p.set_kind_user();
  strcpyT(p.password, password);
  CMDTYPE::CommandTransaction::run(stick, p);
}


TEST_CASE("test", "[test]") {
  Stick20 stick;
  bool connected = stick.connect();
  REQUIRE(connected == true);

  Log::instance().set_loglevel(Loglevel::DEBUG_L2);

  stick10::LockDevice::CommandTransaction::run(stick);
//  execute_password_command<EnableEncryptedPartition>(stick, "123456");
//  execute_password_command<DisableEncryptedPartition>(stick, "123456");
  execute_password_command<EnableEncryptedPartition>(stick, "123456");
  this_thread::sleep_for(1000ms);
  execute_password_command<EnableHiddenEncryptedPartition>(stick, "123123123");
  this_thread::sleep_for(1000ms);
  stick10::LockDevice::CommandTransaction::run(stick);

  stick.disconnect();
}
