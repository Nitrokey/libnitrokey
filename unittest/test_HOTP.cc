#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch.hpp"

#include <iostream>
//#include <string.h>
#include "device_proto.h"
#include "log.h"
#include "stick10_commands.h"
#include <cstdlib>

using namespace std;
using namespace nitrokey::device;
using namespace nitrokey::proto::stick10;
using namespace nitrokey::log;

void hexStringToByte(uint8_t data[], const char* hexString){
    assert(strlen(hexString)%2==0);
    char buf[2];
    for(int i=0; i<strlen(hexString); i++){
        buf[i%2] = hexString[i];
        if (i%2==1){
            data[i/2] = strtoul(buf, NULL, 16) & 0xFF;
        }
    } 
}; 

TEST_CASE("test secret", "[functions]") {
    uint8_t slot_secret[21];
    slot_secret[20] = 0;
    const char* secretHex = "3132333435363738393031323334353637383930";
     hexStringToByte(slot_secret, secretHex);
    CAPTURE(slot_secret);
    REQUIRE(strcmp("12345678901234567890",reinterpret_cast<char *>(slot_secret) ) == 0 );
}

TEST_CASE("Test HOTP codes according to RFC", "[HOTP]") {
    Stick10 stick;
    bool connected = stick.connect();

  REQUIRE(connected == true);

  Log::instance().set_loglevel(Loglevel::DEBUG);

  auto resp = GetStatus::CommandTransaction::run(stick);

  const char * temporary_password = "123456789012345678901234";
  {
      FirstAuthenticate::CommandTransaction::CommandPayload authreq;
      strcpy((char *)(authreq.card_password), "12345678");
      strcpy((char *)(authreq.temporary_password), temporary_password);
      FirstAuthenticate::CommandTransaction::run(stick, authreq);
  }

  //test according to https://tools.ietf.org/html/rfc4226#page-32
  {
    WriteToHOTPSlot::CommandTransaction::CommandPayload hwrite;
    hwrite.slot_number = 0x10;
    strcpy(reinterpret_cast<char *>(hwrite.slot_name), "rfc4226_lib");
    //strcpy(reinterpret_cast<char *>(hwrite.slot_secret), "");
    const char* secretHex = "3132333435363738393031323334353637383930";
    hexStringToByte(hwrite.slot_secret, secretHex);

    // We need to reset the counter of the slot, in case the slot was used earlier
    memset(hwrite.slot_counter, 0, 8);

    //TODO check various configs in separate test cases
    //strcpy(reinterpret_cast<char *>(hwrite.slot_token_id), "");
    //strcpy(reinterpret_cast<char *>(hwrite.slot_counter), "");

    //authorize writehotp first
    {
        Authorize::CommandTransaction::CommandPayload auth;
        strcpy((char *)(auth.temporary_password), temporary_password);
        auth.crc_to_authorize = WriteToHOTPSlot::CommandTransaction::getCRC(hwrite);
        Authorize::CommandTransaction::run(stick, auth);
  }
    
    //run hotp command
    WriteToHOTPSlot::CommandTransaction::run(stick, hwrite);

    uint32_t codes[] = {
            755224, 287082, 359152, 969429, 338314, 
            254676, 287922, 162583, 399871, 520489
    };

    for( auto code: codes){
        GetHOTP::CommandTransaction::CommandPayload gh;
        gh.slot_number =  0x10;
        auto resp = GetHOTP::CommandTransaction::run(stick, gh);
        REQUIRE( resp.code == code);
    }
    //checking slot programmed before with nitro-app
    /*
    for( auto code: codes){
        GetHOTP::CommandTransaction::CommandPayload gh;
        gh.slot_number =  0x12;
        auto resp = GetHOTP::CommandTransaction::run(stick, gh);
        REQUIRE( resp.code == code);
    }
    */
  }


  stick.disconnect();
}
