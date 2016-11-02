#include <cstring>
#include <iostream>
#include "include/NitrokeyManager.h"
#include "include/LibraryException.h"
#include <algorithm>

namespace nitrokey{

    template <typename T>
    void strcpyT(T& dest, const char* src){

      if (src == nullptr)
//            throw EmptySourceStringException(slot_number);
            return;
        const size_t s_dest = sizeof dest;
      nitrokey::log::Log::instance()(std::string("strcpyT sizes dest src ")
                                     +std::to_string(s_dest)+ " "
                                     +std::to_string(strlen(src))+ " "
          ,nitrokey::log::Loglevel::DEBUG_L2);
        if (strlen(src) > s_dest){
            throw TooLongStringException(strlen(src), s_dest, src);
        }
        strncpy((char*) &dest, src, s_dest);
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
    void authorize_packet(T &package, const char *admin_temporary_password, shared_ptr<Device> device){
        auto auth = get_payload<A>();
        strcpyT(auth.temporary_password, admin_temporary_password);
        auth.crc_to_authorize = S::CommandTransaction::getCRC(package);
        A::CommandTransaction::run(*device, auth);
    }

    shared_ptr <NitrokeyManager> NitrokeyManager::_instance = nullptr;

    NitrokeyManager::NitrokeyManager() {
        set_debug(true);
    }
    NitrokeyManager::~NitrokeyManager() {
    }

    bool NitrokeyManager::connect() {
        this->disconnect();
        vector< shared_ptr<Device> > devices = { make_shared<Stick10>(), make_shared<Stick20>() };
        for( auto & d : devices ){
            if (d->connect()){
                device = std::shared_ptr<Device>(d);
            }
        }
        return device != nullptr;
    }


    bool NitrokeyManager::connect(const char *device_model) {
      this->disconnect();
      switch (device_model[0]){
            case 'P':
                device = make_shared<Stick10>();
                break;
            case 'S':
                device = make_shared<Stick20>();
                break;
            default:
                throw std::runtime_error("Unknown model");
        }
        return device->connect();
    }

    shared_ptr<NitrokeyManager> NitrokeyManager::instance() {
        if (_instance == nullptr){
            _instance = shared_ptr<NitrokeyManager>(new NitrokeyManager());
        }
        return _instance;
    }

    bool NitrokeyManager::disconnect() {
      if (device == nullptr){
        return false;
      }
      const auto res = device->disconnect();
      device = nullptr;
      return res;
    }

    void NitrokeyManager::set_debug(bool state) {
        if (state){
            Log::instance().set_loglevel(Loglevel::DEBUG);
        } else {
            Log::instance().set_loglevel(Loglevel::ERROR);
        }
    }

    string NitrokeyManager::get_serial_number() {
        auto response = GetStatus::CommandTransaction::run(*device);
        return response.data().get_card_serial_hex();
    }

    string NitrokeyManager::get_status() {
        auto response = GetStatus::CommandTransaction::run(*device);
        return response.data().dissect();
    }

    uint32_t NitrokeyManager::get_HOTP_code(uint8_t slot_number, const char *user_temporary_password) {
        if (!is_valid_hotp_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        auto gh = get_payload<GetHOTP>();
        gh.slot_number = get_internal_slot_number_for_hotp(slot_number);

        if(user_temporary_password != nullptr && strlen(user_temporary_password)!=0){ //FIXME use string instead of strlen
            authorize_packet<GetHOTP, UserAuthorize>(gh, user_temporary_password, device);
        }

        auto resp = GetHOTP::CommandTransaction::run(*device, gh);
        return resp.data().code;
    }


    bool NitrokeyManager::is_valid_hotp_slot_number(uint8_t slot_number) const { return slot_number < 3; }
    bool NitrokeyManager::is_valid_totp_slot_number(uint8_t slot_number) const { return slot_number < 0x10-1; } //15
    uint8_t NitrokeyManager::get_internal_slot_number_for_totp(uint8_t slot_number) const { return (uint8_t) (0x20 + slot_number); }
    uint8_t NitrokeyManager::get_internal_slot_number_for_hotp(uint8_t slot_number) const { return (uint8_t) (0x10 + slot_number); }

    uint32_t NitrokeyManager::get_TOTP_code(uint8_t slot_number, uint64_t challenge, uint64_t last_totp_time,
                                            uint8_t last_interval,
                                            const char *user_temporary_password) {
        if(!is_valid_totp_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        slot_number = get_internal_slot_number_for_totp(slot_number);
        auto gt = get_payload<GetTOTP>();
        gt.slot_number = slot_number;
        gt.challenge = challenge;
        gt.last_interval = last_interval;
        gt.last_totp_time = last_totp_time;

        if(user_temporary_password != nullptr && strlen(user_temporary_password)!=0){ //FIXME use string instead of strlen
            authorize_packet<GetTOTP, UserAuthorize>(gt, user_temporary_password, device);
        }
        auto resp = GetTOTP::CommandTransaction::run(*device, gt);
        return resp.data().code;
    }

    bool NitrokeyManager::erase_slot(uint8_t slot_number, const char *temporary_password) {
        auto p = get_payload<EraseSlot>();
        p.slot_number = slot_number;

        authorize_packet<EraseSlot, Authorize>(p, temporary_password, device);

        auto resp = EraseSlot::CommandTransaction::run(*device,p);
        return true;
    }

    bool NitrokeyManager::erase_hotp_slot(uint8_t slot_number, const char *temporary_password) {
        if (!is_valid_hotp_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        slot_number = get_internal_slot_number_for_hotp(slot_number);
        return erase_slot(slot_number, temporary_password);
    }

    bool NitrokeyManager::erase_totp_slot(uint8_t slot_number, const char *temporary_password) {
        if (!is_valid_totp_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        slot_number = get_internal_slot_number_for_totp(slot_number);
        return erase_slot(slot_number, temporary_password);
    }

    template <typename T, typename U>
    void vector_copy(T& dest, std::vector<U> &vec){
        const size_t d_size = sizeof(dest);
        if(d_size < vec.size()){
            throw TargetBufferSmallerThanSource(vec.size(), d_size);
        }
        std::fill(dest, dest+d_size, 0);
        std::copy(vec.begin(), vec.end(), dest);
    }

    bool NitrokeyManager::write_HOTP_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint64_t hotp_counter,
                                          bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                                          const char *temporary_password) {
        if (!is_valid_hotp_slot_number(slot_number)) throw InvalidSlotException(slot_number);

        slot_number = get_internal_slot_number_for_hotp(slot_number);
        auto payload = get_payload<WriteToHOTPSlot>();
        payload.slot_number = slot_number;
        auto secret_bin = misc::hex_string_to_byte(secret);
        vector_copy(payload.slot_secret, secret_bin);
        strcpyT(payload.slot_name, slot_name);
        strcpyT(payload.slot_token_id, token_ID);
      switch (device->get_device_model() ){
        case DeviceModel::PRO: {
          payload.slot_counter = hotp_counter;
          break;
        }
        case DeviceModel::STORAGE: {
          std::string counter = std::to_string(hotp_counter);
          strcpyT(payload.slot_counter_s, counter.c_str());
          break;
        }
        default:
          nitrokey::log::Log::instance()(  std::string(__FILE__) + std::to_string(__LINE__) +
                   std::string(__FUNCTION__) + std::string(" Unhandled device model for HOTP")
              , nitrokey::log::Loglevel::DEBUG);
          break;
      }
        payload.use_8_digits = use_8_digits;
        payload.use_enter = use_enter;
        payload.use_tokenID = use_tokenID;

        authorize_packet<WriteToHOTPSlot, Authorize>(payload, temporary_password, device);

        auto resp = WriteToHOTPSlot::CommandTransaction::run(*device, payload);
        return true;
    }

    bool NitrokeyManager::write_TOTP_slot(uint8_t slot_number, const char *slot_name, const char *secret, uint16_t time_window,
                                              bool use_8_digits, bool use_enter, bool use_tokenID, const char *token_ID,
                                              const char *temporary_password) {
        auto payload = get_payload<WriteToTOTPSlot>();
        if (!is_valid_totp_slot_number(slot_number)) throw InvalidSlotException(slot_number);

        slot_number = get_internal_slot_number_for_totp(slot_number);
        payload.slot_number = slot_number;
        auto secret_bin = misc::hex_string_to_byte(secret);
        vector_copy(payload.slot_secret, secret_bin);
        strcpyT(payload.slot_name, slot_name);
        strcpyT(payload.slot_token_id, token_ID);
        payload.slot_interval = time_window; //FIXME naming
        payload.use_8_digits = use_8_digits;
        payload.use_enter = use_enter;
        payload.use_tokenID = use_tokenID;

        authorize_packet<WriteToTOTPSlot, Authorize>(payload, temporary_password, device);

        auto resp = WriteToTOTPSlot::CommandTransaction::run(*device, payload);
        return true;
    }

    const char * NitrokeyManager::get_totp_slot_name(uint8_t slot_number) {
        if (!is_valid_totp_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        slot_number = get_internal_slot_number_for_totp(slot_number);
        return get_slot_name(slot_number);
    }
    const char * NitrokeyManager::get_hotp_slot_name(uint8_t slot_number) {
        if (!is_valid_hotp_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        slot_number = get_internal_slot_number_for_hotp(slot_number);
        return get_slot_name(slot_number);
    }

    const char * NitrokeyManager::get_slot_name(uint8_t slot_number)  {
        auto payload = get_payload<GetSlotName>();
        payload.slot_number = slot_number;
        auto resp = GetSlotName::CommandTransaction::run(*device, payload);
        return strdup((const char *) resp.data().slot_name);
    }

    bool NitrokeyManager::first_authenticate(const char *pin, const char *temporary_password) {
        auto authreq = get_payload<FirstAuthenticate>();
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
        change_PIN_general<ChangeUserPin, PasswordKind::User>(current_PIN, new_PIN);
    }

    void NitrokeyManager::change_admin_PIN(char *current_PIN, char *new_PIN) {
        change_PIN_general<ChangeAdminPin, PasswordKind::Admin>(current_PIN, new_PIN);
    }

    template <typename ProCommand, PasswordKind StoKind>
    void NitrokeyManager::change_PIN_general(char *current_PIN, char *new_PIN) {
        switch (device->get_device_model()){
            case DeviceModel::PRO:
            {
                auto p = get_payload<ProCommand>();
                strcpyT(p.old_pin, current_PIN);
                strcpyT(p.new_pin, new_PIN);
                ProCommand::CommandTransaction::run(*device, p);
            }
                break;
            //in Storage change admin/user pin is divided to two commands with 20 chars field len
            case DeviceModel::STORAGE:
            {
                auto p = get_payload<ChangeAdminUserPin20Current>();
                strcpyT(p.password, current_PIN);
                p.set_kind(StoKind);
                auto p2 = get_payload<ChangeAdminUserPin20New>();
                strcpyT(p2.password, new_PIN);
                p2.set_kind(StoKind);
                ChangeAdminUserPin20Current::CommandTransaction::run(*device, p);
                ChangeAdminUserPin20New::CommandTransaction::run(*device, p2);
            }
                break;
        }

    }

    void NitrokeyManager::enable_password_safe(const char *user_pin) {
        //The following command will cancel enabling PWS if it is not supported
        auto a = get_payload<IsAESSupported>();
        strcpyT(a.user_password, user_pin);
        IsAESSupported::CommandTransaction::run(*device, a);

        auto p = get_payload<EnablePasswordSafe>();
        strcpyT(p.user_password, user_pin);
        EnablePasswordSafe::CommandTransaction::run(*device, p);
    }

    vector <uint8_t> NitrokeyManager::get_password_safe_slot_status() {
        auto responsePayload = GetPasswordSafeSlotStatus::CommandTransaction::run(*device);
        vector<uint8_t> v = vector<uint8_t>(responsePayload.data().password_safe_status,
                                            responsePayload.data().password_safe_status
                                            + sizeof(responsePayload.data().password_safe_status));
        return v;
    }

    uint8_t NitrokeyManager::get_user_retry_count() {
        if(device->get_device_model() == DeviceModel::STORAGE){
          stick20::GetDeviceStatus::CommandTransaction::run(*device);
        }
        auto response = GetUserPasswordRetryCount::CommandTransaction::run(*device);
        return response.data().password_retry_count;
    }
    uint8_t NitrokeyManager::get_admin_retry_count() {
        if(device->get_device_model() == DeviceModel::STORAGE){
          stick20::GetDeviceStatus::CommandTransaction::run(*device);
        }
        auto response = GetPasswordRetryCount::CommandTransaction::run(*device);
        return response.data().password_retry_count;
    }

    void NitrokeyManager::lock_device() {
        LockDevice::CommandTransaction::run(*device);
    }

    const char *NitrokeyManager::get_password_safe_slot_name(uint8_t slot_number) {
        if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        auto p = get_payload<GetPasswordSafeSlotName>();
        p.slot_number = slot_number;
        auto response = GetPasswordSafeSlotName::CommandTransaction::run(*device, p);
        return strdup((const char *) response.data().slot_name);
    }

    bool NitrokeyManager::is_valid_password_safe_slot_number(uint8_t slot_number) const { return slot_number < 16; }

    const char *NitrokeyManager::get_password_safe_slot_login(uint8_t slot_number) {
        if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        auto p = get_payload<GetPasswordSafeSlotLogin>();
        p.slot_number = slot_number;
        auto response = GetPasswordSafeSlotLogin::CommandTransaction::run(*device, p);
        return strdup((const char *) response.data().slot_login);
    }

    const char *NitrokeyManager::get_password_safe_slot_password(uint8_t slot_number) {
        if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        auto p = get_payload<GetPasswordSafeSlotPassword>();
        p.slot_number = slot_number;
        auto response = GetPasswordSafeSlotPassword::CommandTransaction::run(*device, p);
        return strdup((const char *) response.data().slot_password);
    }

    void NitrokeyManager::write_password_safe_slot(uint8_t slot_number, const char *slot_name, const char *slot_login,
                                                       const char *slot_password) {
        if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
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
        if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
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
        switch (device->get_device_model()) {
            case DeviceModel::PRO: {
                auto p = get_payload<BuildAESKey>();
                strcpyT(p.admin_password, admin_password);
                BuildAESKey::CommandTransaction::run(*device, p);
                break;
            }
            case DeviceModel::STORAGE : {
                auto p = get_payload<stick20::CreateNewKeys>();
                strcpyT(p.password, admin_password);
                p.set_defaults();
                stick20::CreateNewKeys::CommandTransaction::run(*device, p);
                break;
            }
        }
    }

    void NitrokeyManager::factory_reset(const char *admin_password) {
        auto p = get_payload<FactoryReset>();
        strcpyT(p.admin_password, admin_password);
        FactoryReset::CommandTransaction::run(*device, p);
    }

    void NitrokeyManager::unlock_user_password(const char *admin_password, const char *new_user_password) {
      switch (device->get_device_model()){
        case DeviceModel::PRO: {
          auto p = get_payload<stick10::UnlockUserPassword>();
          strcpyT(p.admin_password, admin_password);
          strcpyT(p.user_new_password, new_user_password);
          stick10::UnlockUserPassword::CommandTransaction::run(*device, p);
          break;
        }
        case DeviceModel::STORAGE : {
          auto p2 = get_payload<ChangeAdminUserPin20Current>();
          p2.set_defaults();
          strcpyT(p2.password, admin_password);
          ChangeAdminUserPin20Current::CommandTransaction::run(*device, p2);
          auto p3 = get_payload<stick20::UnlockUserPin>();
          p3.set_defaults();
          strcpyT(p3.password, new_user_password);
          stick20::UnlockUserPin::CommandTransaction::run(*device, p3);
          break;
        }
      }
    }


    void NitrokeyManager::write_config(uint8_t numlock, uint8_t capslock, uint8_t scrolllock, bool enable_user_password,
                                       bool delete_user_password, const char *admin_temporary_password) {
        auto p = get_payload<WriteGeneralConfig>();
        p.numlock = (uint8_t) numlock;
        p.capslock = (uint8_t) capslock;
        p.scrolllock = (uint8_t) scrolllock;
        p.enable_user_password = (uint8_t) enable_user_password;
        p.delete_user_password = (uint8_t) delete_user_password;

        authorize_packet<WriteGeneralConfig, Authorize>(p, admin_temporary_password, device);

        WriteGeneralConfig::CommandTransaction::run(*device, p);
    }

    vector<uint8_t> NitrokeyManager::read_config() {
        auto responsePayload = GetStatus::CommandTransaction::run(*device);
        vector<uint8_t> v = vector<uint8_t>(responsePayload.data().general_config,
                                            responsePayload.data().general_config+sizeof(responsePayload.data().general_config));
        return v;
    }

    bool NitrokeyManager::is_AES_supported(const char *user_password) {
        auto a = get_payload<IsAESSupported>();
        strcpyT(a.user_password, user_password);
        IsAESSupported::CommandTransaction::run(*device, a);
        return true;
    }

}
