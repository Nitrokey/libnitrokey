#include <iostream>
//#include <string.h>
#include "device_proto.h"
#include "log.h"
#include "stick10_commands.h"
#include "toplevel.h"
#include <cstdlib>

using namespace std;
using namespace nitrokey::device;
using namespace nitrokey::proto::stick10;
using namespace nitrokey::log;

void hexStringToByte(uint8_t data[], const char* hexString){
    char buf[2];
    for(int i=0; i<strlen(hexString); i++){
        buf[i%2] = hexString[i];
        if (i%2==1){
            data[i/2] = strtoul(buf, NULL, 16) & 0xFF;
        }
    } 
}; 

void initHotp(int slot, const char* card_password, const char* hexSecret, const char* slotName) {
  /*
  slot: The slot number to initialize. HOTP slots are 16 - 18
  card_password: The SO password of the nitrokey
  hexSecret: The 160bit HMAC-SHA1 secret key
  slotName: The new name of the slot
  */
  Stick10 stick;
  bool connected = stick.connect();

  //Log::instance().set_loglevel(Loglevel::DEBUG);

  auto resp = GetStatus::CommandTransaction::run(stick);

  const char * temporary_password = "123456789012345678901234";
  {
      FirstAuthenticate::CommandTransaction::CommandPayload authreq;
      strcpy((char *)(authreq.card_password), card_password);
      strcpy((char *)(authreq.temporary_password), temporary_password);
      FirstAuthenticate::CommandTransaction::run(stick, authreq);
  }

  {
    WriteToHOTPSlot::CommandTransaction::CommandPayload hwrite;
    hwrite.slot_number = slot;
    strcpy(reinterpret_cast<char *>(hwrite.slot_name), slotName);
    hexStringToByte(hwrite.slot_secret, hexSecret);

    // We need to reset the counter of the slot, in case the slot was used earlier
    memset(hwrite.slot_counter, 0, 8);
    //authorize writehotp first
    {
        Authorize::CommandTransaction::CommandPayload auth;
        strcpy((char *)(auth.temporary_password), temporary_password);
        auth.crc_to_authorize = WriteToHOTPSlot::CommandTransaction::getCRC(hwrite);
        Authorize::CommandTransaction::run(stick, auth);
  }
    
    //run hotp command
    WriteToHOTPSlot::CommandTransaction::run(stick, hwrite);

  }


  stick.disconnect();
}
