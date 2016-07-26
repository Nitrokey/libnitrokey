#include <cassert>
#include <cstring>
#include "include/NitrokeyManager.h"

namespace nitrokey{

    template <typename T>
    void strcpyT(T& dest, const char* src){
        const int s = sizeof dest;
        assert(strlen(src) <= s);
        strncpy((char*) &dest, src, s);
    }

    template <typename T>
    typename T::CommandPayload get_payload(){
        //Create, initialize and return by value command payload
        typename T::CommandPayload st;
        bzero(&st, sizeof(st));
        return st;
    }

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
        auto gh = get_payload<GetHOTP>();
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
        slot_number = get_internal_slot_number_for_totp(slot_number);
        auto gt = get_payload<GetTOTP>();
        gt.slot_number = slot_number;
        gt.challenge = challenge;
        gt.last_interval = last_interval;
        gt.last_totp_time = last_totp_time;
        auto resp = GetTOTP::CommandTransaction::run(*device, gt);
        return resp.code;
    }

    bool NitrokeyManager::erase_slot(uint8_t slot_number) {
        auto p = get_payload<EraseSlot>();
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
        auto payload = get_payload<WriteToHOTPSlot>();
        payload.slot_number = slot_number;
        strcpyT(payload.slot_secret, secret);
        strcpyT(payload.slot_name, slot_name);
        payload.slot_counter = hotp_counter;
        payload.slot_config; //TODO

        auto auth = get_payload<Authorize>();
        strcpyT(auth.temporary_password, temporary_password);
        auth.crc_to_authorize = WriteToHOTPSlot::CommandTransaction::getCRC(payload);
        Authorize::CommandTransaction::run(*device, auth);

        auto resp = WriteToHOTPSlot::CommandTransaction::run(*device, payload);
        return true;
    }

    enum totp_config{digits8=0, enter=1, tokenID=2};

    bool NitrokeyManager::write_TOTP_slot(uint8_t slot_number, const char *slot_name, const char *secret,
                                          uint16_t time_window, bool use_8_digits, const char *temporary_password) {
        auto payload = get_payload<WriteToTOTPSlot>();
        assert(is_valid_totp_slot_number(slot_number));
        assert(strlen(secret) == sizeof payload.slot_secret); //160 bits
        assert(strlen(slot_name) <= sizeof payload.slot_name);

        slot_number = get_internal_slot_number_for_totp(slot_number);
        payload.slot_number = slot_number;
        strcpyT(payload.slot_secret, secret);
        strcpyT(payload.slot_name, slot_name);
        payload.slot_interval = time_window; //FIXME naming
        bitset<8> config; //FIXME better config manipulation
        config.set(totp_config::digits8, use_8_digits);
        payload.slot_config = (uint8_t) config.to_ulong();

        auto auth = get_payload<Authorize>();
        strcpyT(auth.temporary_password, temporary_password);
        auth.crc_to_authorize = WriteToTOTPSlot::CommandTransaction::getCRC(payload);
        Authorize::CommandTransaction::run(*device, auth);
//        auto auth_successful = device->last_command_sucessfull();

        auto resp = WriteToTOTPSlot::CommandTransaction::run(*device, payload);
//        auto write_successful = device->last_command_sucessfull();
//        return auth_successful && write_successful; //left to show alternative approach
        return true;
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

    uint8_t *NitrokeyManager::get_slot_name(uint8_t slot_number) const { //FIXME -const s/uint/char:string
        auto payload = get_payload<GetSlotName>();
        payload.slot_number = slot_number;
        auto resp = GetSlotName::CommandTransaction::run(*device, payload);
        return (uint8_t *) strdup((const char *) resp.slot_name);
    }

    bool NitrokeyManager::authorize(const char *pin, const char *temporary_password) {
        auto authreq = get_payload<FirstAuthenticate>();

        assert(strlen(pin) < sizeof authreq.card_password);
        assert(strlen(temporary_password) < sizeof authreq.temporary_password);

        strcpyT(authreq.card_password, pin);
        strcpyT(authreq.temporary_password, temporary_password);
        FirstAuthenticate::CommandTransaction::run(*device, authreq);
        return true;
    }

    bool NitrokeyManager::set_time(uint64_t time) {
        auto p = get_payload<SetTime>();
        p.reset = 1;
        p.time = time;
        SetTime::CommandTransaction::run(*device, p);
        return false;
    }

    bool NitrokeyManager::get_time() {
        auto p = get_payload<SetTime>();
        p.reset = 0;
        SetTime::CommandTransaction::run(*device, p);
        return false;
    }

    void NitrokeyManager::change_user_PIN(char *current_PIN, char *new_PIN) {
        auto p = get_payload<ChangeUserPin>();
        strcpyT(p.old_pin, current_PIN);
        strcpyT(p.new_pin, new_PIN);
        ChangeUserPin::CommandTransaction::run(*device, p);
    }
    void NitrokeyManager::change_admin_PIN(char *current_PIN, char *new_PIN) {
        auto p = get_payload<ChangeAdminPin>();
        strcpyT(p.old_pin, current_PIN);
        strcpyT(p.new_pin, new_PIN);
        ChangeAdminPin::CommandTransaction::run(*device, p);
    }

    void NitrokeyManager::enable_password_safe(const char *user_pin) {
        auto p = get_payload<EnablePasswordSafe>();
        strcpyT(p.user_password, user_pin);
        EnablePasswordSafe::CommandTransaction::run(*device, p);
    }

    void NitrokeyManager::get_password_safe_slot_status() {
        GetPasswordSafeSlotStatus::CommandTransaction::run(*device); //TODO FIXME
    }

    uint8_t NitrokeyManager::get_user_retry_count() {
        auto response = GetUserPasswordRetryCount::CommandTransaction::run(*device);
        return response.password_retry_count;
    }
    uint8_t NitrokeyManager::get_admin_retry_count() {
        auto response = GetPasswordRetryCount::CommandTransaction::run(*device);
        return response.password_retry_count;
    }

    void NitrokeyManager::lock_device() {
        LockDevice::CommandTransaction::run(*device);
    }

    const char *NitrokeyManager::get_password_safe_slot_name(uint8_t slot_number, const char *temporary_password) {
        auto p = get_payload<GetPasswordSafeSlotName>();
        p.slot_number = slot_number;

        auto auth = get_payload<UserAuthorize>();
        strcpyT(auth.temporary_password, temporary_password);
        auth.crc_to_authorize = GetPasswordSafeSlotName::CommandTransaction::getCRC(p);
        UserAuthorize::CommandTransaction::run(*device, auth);

        auto response = GetPasswordSafeSlotName::CommandTransaction::run(*device, p);
        return strdup((const char *) response.slot_name);
    }

    const char *NitrokeyManager::get_password_safe_slot_login(uint8_t slot_number, const char *temporary_password) {
        auto p = get_payload<GetPasswordSafeSlotLogin>();
        p.slot_number = slot_number;
        auto response = GetPasswordSafeSlotLogin::CommandTransaction::run(*device, p);
        return strdup((const char *) response.slot_login);
    }

    const char *NitrokeyManager::get_password_safe_slot_password(uint8_t slot_number, const char *temporary_password) {
        auto p = get_payload<GetPasswordSafeSlotPassword>();
        p.slot_number = slot_number;
        auto response = GetPasswordSafeSlotPassword::CommandTransaction::run(*device, p);
        return strdup((const char *) response.slot_password);
    }

}