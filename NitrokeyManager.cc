#include <cassert>
#include <cstring>
#include "include/NitrokeyManager.h"

namespace nitrokey{

    template <typename T>
    void initialize(T& st){ bzero(&st, sizeof(st)); }

    NitrokeyManager * NitrokeyManager::_instance = nullptr;

    NitrokeyManager::NitrokeyManager(): device(nullptr) {
        set_debug(true);
    }
    NitrokeyManager::~NitrokeyManager() {delete _instance; delete device;}

    bool NitrokeyManager::connect() {
        device = new Stick10();
        return device->connect();
    }

    NitrokeyManager *NitrokeyManager::instance() {
        if (_instance == nullptr){
            _instance = new NitrokeyManager();
        }
        return _instance;
    }

    bool NitrokeyManager::disconnect() {
        return device->disconnect();
    }

    void NitrokeyManager::set_debug(bool state) {
        if (state){
            Log::instance().set_loglevel(Loglevel::DEBUG_L2);
        } else {
            Log::instance().set_loglevel(Loglevel::ERROR);
        }
    }

    string NitrokeyManager::get_status() {
        auto response = GetStatus::CommandTransaction::run(*device);
        return response.dissect();
    }

    uint32_t NitrokeyManager::get_HOTP_code(uint8_t slot_number) {
        assert(is_valid_hotp_slot_number(slot_number));
        GetHOTP::CommandTransaction::CommandPayload gh;
        gh.slot_number = get_internal_slot_number_for_hotp(slot_number);
        auto resp = GetHOTP::CommandTransaction::run(*device, gh);
        return resp.code;
    }


    bool NitrokeyManager::is_valid_hotp_slot_number(uint8_t slot_number) const { return slot_number < 3; }
    bool NitrokeyManager::is_valid_totp_slot_number(uint8_t slot_number) const { return slot_number < 0x10; }
    uint8_t NitrokeyManager::get_internal_slot_number_for_totp(uint8_t slot_number) const { return (uint8_t) (0x20 + slot_number); }
    uint8_t NitrokeyManager::get_internal_slot_number_for_hotp(uint8_t slot_number) const { return (uint8_t) (0x10 + slot_number); }

    uint32_t NitrokeyManager::get_TOTP_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time,
                                                uint8_t last_interval) {
        assert(is_valid_totp_slot_number(slot_number));
        GetTOTP::CommandTransaction::CommandPayload gt;
        gt.slot_number = slot_number;
        gt.challenge = challenge;
        gt.last_interval = last_interval;
        gt.last_totp_time = last_totp_time;
        auto resp = GetTOTP::CommandTransaction::run(*device, gt);
        return resp.code;
    }

    bool NitrokeyManager::erase_slot(uint8_t slot_number) {
        EraseSlot::CommandTransaction::CommandPayload p;
        p.slot_number = slot_number;
        auto resp = EraseSlot::CommandTransaction::run(*device,p);
        return true;
    }

    bool NitrokeyManager::erase_hotp_slot(uint8_t slot_number) {
        assert(is_valid_hotp_slot_number(slot_number));
        slot_number = get_internal_slot_number_for_hotp(slot_number);
        return erase_slot(slot_number);
    }

    bool NitrokeyManager::erase_totp_slot(uint8_t slot_number) {
        assert(is_valid_totp_slot_number(slot_number));
        slot_number = get_internal_slot_number_for_totp(slot_number);
        return erase_slot(slot_number);
    }

    bool NitrokeyManager::write_HOTP_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint64_t hotp_counter,
                                              const char *temporary_password) {
        assert(is_valid_hotp_slot_number(slot_number));
        assert(strlen(secret)==20); //160 bits
        assert(strlen(slot_name)<=15);

        slot_number = get_internal_slot_number_for_hotp(slot_number);
        WriteToHOTPSlot::CommandPayload payload;
        payload.slot_number = slot_number;
        strcpy((char *) payload.slot_secret, secret);
        strcpy((char *) payload.slot_name, slot_name);
        payload.slot_counter = hotp_counter;
        payload.slot_config;
        memset(payload.slot_token_id, 0, sizeof(payload.slot_token_id)); //?????

        Authorize::CommandPayload auth;
        initialize(auth);
        strcpy((char *) (auth.temporary_password), temporary_password);
        auth.crc_to_authorize = auth.crc_to_authorize = WriteToHOTPSlot::CommandTransaction::getCRC(payload);
        Authorize::CommandTransaction::run(*device, auth);

        auto resp = WriteToHOTPSlot::CommandTransaction::run(*device, payload);
        return false;
    }

    bool NitrokeyManager::write_TOTP_slot(uint8_t slot_number, const char *secret, uint16_t time_window) {
        assert(is_valid_totp_slot_number(slot_number));
        slot_number = get_internal_slot_number_for_totp(slot_number);
        return false;
    }

    const char * NitrokeyManager::get_totp_slot_name(uint8_t slot_number) {
        assert(is_valid_totp_slot_number(slot_number));
        slot_number = get_internal_slot_number_for_totp(slot_number);
        return (const char *) get_slot_name(slot_number);
    }
    const char * NitrokeyManager::get_hotp_slot_name(uint8_t slot_number) {
        assert(is_valid_hotp_slot_number(slot_number));
        slot_number = get_internal_slot_number_for_hotp(slot_number);
        return (const char *) get_slot_name(slot_number);
    }

    uint8_t *NitrokeyManager::get_slot_name(uint8_t slot_number) const {
        GetSlotName::CommandPayload payload;
        payload.slot_number = slot_number;
        auto resp = GetSlotName::CommandTransaction::run(*device, payload);
        return (uint8_t *) strdup((const char *) resp.slot_name);
    }

    bool NitrokeyManager::authorize(const char *pin, const char *temporary_password) {
        FirstAuthenticate::CommandPayload authreq;
        initialize(authreq); //TODO
        strcpy((char *) (authreq.card_password), pin);
        strcpy((char *) (authreq.temporary_password), temporary_password);
        FirstAuthenticate::CommandTransaction::run(*device, authreq);
        return true;
    }


}