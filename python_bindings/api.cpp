
#include <iostream>
//#include "toplevel.h"
#include "../include/device.h"
#include "../include/stick10_commands.h"
#include "../include/log.h"
#include "../include/device_proto.h"
#include <cstdlib>
#include <cstring>

using namespace std;
using namespace nitrokey::device;
using namespace nitrokey::proto::stick10;
using namespace nitrokey::proto;
using namespace nitrokey::log;

/*
- manage (=create, change, delete, list, read) OTP entries
- use/generate OTPs
- Change PINs (so that the user doesn't need any other tool other than
his Python application)
* */


bool writeHOTPSlot(Device &stick, int slotNumber, const char *slotName, const char *temporary_password,
                   const char *secret);
bool authenticate(Device &stick, const char *card_password, const char *temporary_password);

int NK_login(char *user_type, char *pin);
int NK_logout();

//some_struct
void NK_list_slots();
int NK_erase_slot(int slot_num);
int NK_erase_totp_slot(int slot_num);
int NK_erase_hotp_slot(int slot_num);
int NK_write_hotp_slot(char *secret, int hotp_counter);
int NK_write_totp_slot(char *secret, int time_window);
int NK_change_PIN();

void initHotp(const char *card_password, int slot, const char *slot_name, const char *secret) {
    //Log::instance().set_loglevel(Loglevel::DEBUG);
    Stick10 stick;
    bool connected = stick.connect();
    auto response = GetStatus::CommandTransaction::run(stick);
    const char *temporary_password = "123456789012345678901234";
    bool success = authenticate(stick, card_password, temporary_password);
//    hexStringToByte(hwrite.slot_secret, hexSecret);
    success = writeHOTPSlot(stick, slot, slot_name, temporary_password, secret);
    stick.disconnect();
}

bool writeHOTPSlot(Device &stick, int slotNumber, const char *slotName, const char *temporary_password,
                   const char *secret) {
    Transaction::CommandPayload hwrite;
    hwrite.slot_number = slotNumber;
    strcpy(reinterpret_cast<char *>(hwrite.slot_name), slotName);
    strcpy(reinterpret_cast<char *>(hwrite.slot_secret), secret);

    //authorize writehotp first
    Transaction::CommandPayload auth;
    strcpy((char *) (auth.temporary_password), temporary_password);
    auth.crc_to_authorize = auth.crc_to_authorize = WriteToHOTPSlot::CommandTransaction::getCRC(hwrite);
    Authorize::CommandTransaction::run(stick, auth);

    //run hotp command
    WriteToHOTPSlot::CommandTransaction::run(stick, hwrite);
    return true;
}

bool authenticate(Device &stick, const char *card_password, const char *temporary_password) {
    Transaction::CommandPayload authreq;
    strcpy((char *) (authreq.card_password), card_password);
    strcpy((char *) (authreq.temporary_password), temporary_password);
    FirstAuthenticate::CommandTransaction::run(stick, authreq);
    return true;
}
