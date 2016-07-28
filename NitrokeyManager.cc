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


    // package type to auth, auth type [Authorize,UserAuthorize]
    template <typename S, typename A, typename T>
    void auth_package(T& package, const char* admin_temporary_password, Device * device){
        auto auth = get_payload<A>();
        strcpyT(auth.temporary_password, admin_temporary_password);
        auth.crc_to_authorize = S::CommandTransaction::getCRC(package);
        A::CommandTransaction::run(*device, auth);
    }

    NitrokeyManager * NitrokeyManager::_instance = nullptr;

    NitrokeyManager::NitrokeyManager(): device(nullptr) {
        set_debug(true);
    }
    NitrokeyManager::~NitrokeyManager() {delete _instance; delete device;}

    bool NitrokeyManager::connect(const char *device_model) {
        switch (device_model[0]){
            case 'P':
                device = new Stick10();
                break;
            case 'S':
                device = new Stick20();
                break;
            default:
                throw std::runtime_error("Unknown model");
        }
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

    uint32_t NitrokeyManager::get_HOTP_code(uint8_t slot_number, const char *user_temporary_password) {
        assert(is_valid_hotp_slot_number(slot_number));
        auto gh = get_payload<GetHOTP>();
        gh.slot_number = get_internal_slot_number_for_hotp(slot_number);

        //TODO handle user authorization requests (taken from config)
        if(user_temporary_password != nullptr && strlen(user_temporary_password)!=0){ //FIXME use string instead of strlen
            auth_package<GetHOTP, UserAuthorize>(gh, user_temporary_password, device);
        }

        auto resp = GetHOTP::CommandTransaction::run(*device, gh);
        return resp.code;
    }


    bool NitrokeyManager::is_valid_hotp_slot_number(uint8_t slot_number) const { return slot_number < 3; }
    bool NitrokeyManager::is_valid_totp_slot_number(uint8_t slot_number) const { return slot_number < 0x10; }
    uint8_t NitrokeyManager::get_internal_slot_number_for_totp(uint8_t slot_number) const { return (uint8_t) (0x20 + slot_number); }
    uint8_t NitrokeyManager::get_internal_slot_number_for_hotp(uint8_t slot_number) const { return (uint8_t) (0x10 + slot_number); }

    uint32_t NitrokeyManager::get_TOTP_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time,
                                            uint8_t last_interval,
                                            const char *user_temporary_password) {
        assert(is_valid_totp_slot_number(slot_number));
        slot_number = get_internal_slot_number_for_totp(slot_number);
        auto gt = get_payload<GetTOTP>();
        gt.slot_number = slot_number;
        gt.challenge = challenge;
        gt.last_interval = last_interval;
        gt.last_totp_time = last_totp_time;
        //TODO handle user authorization requests (taken from config)
        if(user_temporary_password != nullptr && strlen(user_temporary_password)!=0){ //FIXME use string instead of strlen
            auth_package<GetTOTP, UserAuthorize>(gt, user_temporary_password, device);
        }
        auto resp = GetTOTP::CommandTransaction::run(*device, gt);
        return resp.code;
    }

    bool NitrokeyManager::erase_slot(uint8_t slot_number, const char *temporary_password) {
        auto p = get_payload<EraseSlot>();
        p.slot_number = slot_number;

        auto auth = get_payload<Authorize>();
        strcpyT(auth.temporary_password, temporary_password);
        auth.crc_to_authorize = EraseSlot::CommandTransaction::getCRC(p);
        Authorize::CommandTransaction::run(*device, auth);

        auto resp = EraseSlot::CommandTransaction::run(*device,p);
        return true;
    }

    bool NitrokeyManager::erase_hotp_slot(uint8_t slot_number, const char *temporary_password) {
        assert(is_valid_hotp_slot_number(slot_number));
        slot_number = get_internal_slot_number_for_hotp(slot_number);
        return erase_slot(slot_number, temporary_password);
    }

    bool NitrokeyManager::erase_totp_slot(uint8_t slot_number, const char *temporary_password) {
        assert(is_valid_totp_slot_number(slot_number));
        slot_number = get_internal_slot_number_for_totp(slot_number);
        return erase_slot(slot_number, temporary_password);
    }


    bool NitrokeyManager::write_HOTP_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint64_t hotp_counter,
                                              bool use_8_digits, const char *temporary_password) {
        assert(is_valid_hotp_slot_number(slot_number));
        assert(strlen(secret)==20); //160 bits
        assert(strlen(slot_name)<=15);

        slot_number = get_internal_slot_number_for_hotp(slot_number);
        auto payload = get_payload<WriteToHOTPSlot>();
        payload.slot_number = slot_number;
        strcpyT(payload.slot_secret, secret);
        strcpyT(payload.slot_name, slot_name);
        payload.slot_counter = hotp_counter;
        payload.use_8_digits = use_8_digits;

        auto auth = get_payload<Authorize>();
        strcpyT(auth.temporary_password, temporary_password);
        auth.crc_to_authorize = WriteToHOTPSlot::CommandTransaction::getCRC(payload);
        Authorize::CommandTransaction::run(*device, auth);

        auto resp = WriteToHOTPSlot::CommandTransaction::run(*device, payload);
        return true;
    }

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
        payload.use_8_digits = use_8_digits;

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

    bool NitrokeyManager::first_authenticate(const char *pin, const char *temporary_password) {
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

    uint8_t * NitrokeyManager::get_password_safe_slot_status() {
        auto responsePayload = GetPasswordSafeSlotStatus::CommandTransaction::run(*device); //TODO FIXME
        auto res = new uint8_t[16];
        memcpy(res, responsePayload.password_safe_status, 16*sizeof (uint8_t));
        //FIXME return vector<uint8_t> and do copy on C_API side
        return res;
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

    const char *NitrokeyManager::get_password_safe_slot_name(uint8_t slot_number) {
        assert (is_valid_password_safe_slot_number(slot_number));
        auto p = get_payload<GetPasswordSafeSlotName>();
        p.slot_number = slot_number;
        auto response = GetPasswordSafeSlotName::CommandTransaction::run(*device, p);
        return strdup((const char *) response.slot_name);
    }

    bool NitrokeyManager::is_valid_password_safe_slot_number(uint8_t slot_number) const { return slot_number < 16; }

    const char *NitrokeyManager::get_password_safe_slot_login(uint8_t slot_number) {
        assert (is_valid_password_safe_slot_number(slot_number));
        auto p = get_payload<GetPasswordSafeSlotLogin>();
        p.slot_number = slot_number;
        auto response = GetPasswordSafeSlotLogin::CommandTransaction::run(*device, p);
        return strdup((const char *) response.slot_login);
    }

    const char *NitrokeyManager::get_password_safe_slot_password(uint8_t slot_number) {
        assert (is_valid_password_safe_slot_number(slot_number));
        auto p = get_payload<GetPasswordSafeSlotPassword>();
        p.slot_number = slot_number;
        auto response = GetPasswordSafeSlotPassword::CommandTransaction::run(*device, p);
        return strdup((const char *) response.slot_password);
    }

    void NitrokeyManager::write_password_safe_slot(uint8_t slot_number, const char *slot_name, const char *slot_login,
                                                       const char *slot_password) {
        assert (is_valid_password_safe_slot_number(slot_number));
        auto p = get_payload<SetPasswordSafeSlotData>();
        p.slot_number = slot_number;
        strcpyT(p.slot_name, slot_name);
        strcpyT(p.slot_password, slot_password);
        SetPasswordSafeSlotData::CommandTransaction::run(*device, p);

        auto p2 = get_payload<SetPasswordSafeSlotData2>();
        p2.slot_number = slot_number;
        strcpyT(p2.slot_login_name, slot_login);
        SetPasswordSafeSlotData2::CommandTransaction::run(*device, p2);
    }

    void NitrokeyManager::erase_password_safe_slot(uint8_t slot_number) {
        assert (is_valid_password_safe_slot_number(slot_number));
        auto p = get_payload<ErasePasswordSafeSlot>();
        p.slot_number = slot_number;
        ErasePasswordSafeSlot::CommandTransaction::run(*device, p);
    }

    void NitrokeyManager::user_authenticate(const char *user_password, const char *temporary_password) {
        auto p = get_payload<UserAuthenticate>();
        strcpyT(p.card_password, user_password);
        strcpyT(p.temporary_password, temporary_password);
        UserAuthenticate::CommandTransaction::run(*device, p);
    }

    void NitrokeyManager::build_aes_key(const char *admin_password) {
        auto p = get_payload<BuildAESKey>();
        strcpyT(p.admin_password, admin_password);
        BuildAESKey::CommandTransaction::run(*device, p);
    }

    void NitrokeyManager::factory_reset(const char *admin_password) {
        auto p = get_payload<FactoryReset>();
        strcpyT(p.admin_password, admin_password);
        FactoryReset::CommandTransaction::run(*device, p);
    }

    void NitrokeyManager::unlock_user_password(const char *admin_password) {
        auto p = get_payload<UnlockUserPassword>();
        strcpyT(p.admin_password, admin_password);
        UnlockUserPassword::CommandTransaction::run(*device, p);
    }


    void NitrokeyManager::write_config(bool numlock, bool capslock, bool scrolllock, bool enable_user_password, bool delete_user_password, const char *admin_temporary_password) {
        auto p = get_payload<WriteGeneralConfig>();
        p.numlock = (uint8_t) numlock;
        p.capslock = (uint8_t) capslock;
        p.scrolllock = (uint8_t) scrolllock;
        p.enable_user_password = (uint8_t) enable_user_password;
        p.delete_user_password = (uint8_t) delete_user_password;

        auth_package<WriteGeneralConfig, Authorize>(p, admin_temporary_password, device);

        WriteGeneralConfig::CommandTransaction::run(*device, p);
    }

    vector<uint8_t> NitrokeyManager::read_config() {
        auto responsePayload = GetStatus::CommandTransaction::run(*device);
        vector<uint8_t> v = vector<uint8_t>(responsePayload.general_config,
                                            responsePayload.general_config+sizeof(responsePayload.general_config));
        return v;
    }

}