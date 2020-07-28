/*
 * Copyright (c) 2015-2018 Nitrokey UG
 *
 * This file is part of libnitrokey.
 *
 * libnitrokey is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * libnitrokey is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libnitrokey. If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0
 */

#include "libnitrokey/NitrokeyManager.h"
#include "NitrokeyManagerOTP.h"
#include "libnitrokey/LibraryException.h"
#include "libnitrokey/cxx_semantics.h"
#include "libnitrokey/misc.h"
#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <stick10_commands.h>
#include <stick20_commands.h>
#include <unordered_map>

std::mutex nitrokey::proto::send_receive_mtx;

namespace nitrokey{
    std::mutex mex_dev_com_manager;

using nitrokey::misc::strcpyT;


    // package type to auth, auth type [Authorize,UserAuthorize]
    template <typename S, typename A, typename T>
    void NitrokeyManager::authorize_packet(T &package, const char *admin_temporary_password, shared_ptr<Device> device){
      if (!is_authorization_command_supported()){
        LOG("Authorization command not supported, skipping", Loglevel::WARNING);
      }
        auto auth = get_payload<A>();
        strcpyT(auth.temporary_password, admin_temporary_password);
        auth.crc_to_authorize = S::CommandTransaction::getCRC(package);
        A::CommandTransaction::run(device, auth);
    }

    shared_ptr <NitrokeyManager> NitrokeyManager::_instance = nullptr;

    NitrokeyManager::NitrokeyManager() : device(nullptr)
    {
        set_debug(false);
    }
    NitrokeyManager::~NitrokeyManager() {
        std::lock_guard<std::mutex> lock(mex_dev_com_manager);

        for (auto d : connected_devices){
            if (d.second == nullptr) continue;
            d.second->disconnect();
            connected_devices[d.first] = nullptr;
        }
    }

    bool NitrokeyManager::set_current_device_speed(int retry_delay, int send_receive_delay){
      if (retry_delay < 20 || send_receive_delay < 20){
        LOG("Delay set too low: " + to_string(retry_delay) +" "+ to_string(send_receive_delay), Loglevel::WARNING);
        return false;
      }

      std::lock_guard<std::mutex> lock(mex_dev_com_manager);
      if(device == nullptr) {
        return false;
      }
      device->set_receiving_delay(std::chrono::duration<int, std::milli>(send_receive_delay));
      device->set_retry_delay(std::chrono::duration<int, std::milli>(retry_delay));
      return true;
    }

    std::vector<DeviceInfo> NitrokeyManager::list_devices(){
        std::lock_guard<std::mutex> lock(mex_dev_com_manager);

        return Device::enumerate();
    }

    std::vector<std::string> NitrokeyManager::list_devices_by_cpuID(){
        using misc::toHex;
        //disconnect default device
        disconnect();

        std::lock_guard<std::mutex> lock(mex_dev_com_manager);
        LOGD1("Disconnecting registered devices");
        for (auto & kv : connected_devices_byID){
            if (kv.second != nullptr)
                kv.second->disconnect();
        }
        connected_devices_byID.clear();

        LOGD1("Enumerating devices");
        std::vector<std::string> res;
        const auto v = Device::enumerate();
        LOGD1("Discovering IDs");
        for (auto & i: v){
            if (i.m_deviceModel != DeviceModel::STORAGE)
                continue;
            auto p = i.m_path;
            auto d = make_shared<Stick20>();
            LOGD1( std::string("Found: ") + p );
            d->set_path(p);
            try{
                if (d->connect()){
                    device = d;
                    std::string id;
                    try {
                        const auto status = get_status_storage();
                        const auto sc_id = toHex(status.ActiveSmartCardID_u32);
                        const auto sd_id = toHex(status.ActiveSD_CardID_u32);
                        id += sc_id + ":" + sd_id;
                        id += "_p_" + p;
                    }
                    catch (const LongOperationInProgressException &e) {
                        LOGD1(std::string("Long operation in progress, setting ID to: ") + p);
                        id = p;
                    }

                    connected_devices_byID[id] = d;
                    res.push_back(id);
                    LOGD1( std::string("Found: ") + p + " => " + id);
                } else{
                    LOGD1( std::string("Could not connect to: ") + p);
                }
            }
            catch (const DeviceCommunicationException &e){
                LOGD1( std::string("Exception encountered: ") + p);
            }
        }
        return res;
    }

    bool NitrokeyManager::connect_with_ID(const std::string id) {
        std::lock_guard<std::mutex> lock(mex_dev_com_manager);

        auto position = connected_devices_byID.find(id);
        if (position == connected_devices_byID.end()) {
            LOGD1(std::string("Could not find device ")+id + ". Refresh devices list with list_devices_by_cpuID().");
            return false;
        }

        auto d = connected_devices_byID[id];
        device = d;
        current_device_id = id;

        //validate connection
        try{
            get_status();
        }
        catch (const LongOperationInProgressException &){
            //ignore
        }
        catch (const DeviceCommunicationException &){
            d->disconnect();
            current_device_id = "";
            connected_devices_byID[id] = nullptr;
            connected_devices_byID.erase(position);
            return false;
        }
        nitrokey::log::Log::setPrefix(id);
        LOGD1("Device successfully changed");
        return true;
    }

        /**
         * Connects device to path.
         * Assumes devices are not being disconnected and caches connections (param cache_connections).
         * @param path os-dependent device path
         * @return false, when could not connect, true otherwise
         */
    bool NitrokeyManager::connect_with_path(std::string path) {
        const bool cache_connections = false;

        std::lock_guard<std::mutex> lock(mex_dev_com_manager);

        if (cache_connections){
            if(connected_devices.find(path) != connected_devices.end()
                    && connected_devices[path] != nullptr) {
                device = connected_devices[path];
                return true;
            }
        }

	auto vendor_id = NITROKEY_VID;
        auto info_ptr = hid_enumerate(vendor_id, 0);
	if (!info_ptr) {
	  vendor_id = PURISM_VID;
	  info_ptr = hid_enumerate(vendor_id, 0);
	}
        auto first_info_ptr = info_ptr;
        if (!info_ptr)
          return false;

        misc::Option<DeviceModel> model;
        while (info_ptr && !model.has_value()) {
            if (path == std::string(info_ptr->path)) {
                model = product_id_to_model(info_ptr->vendor_id, info_ptr->product_id);
            }
            info_ptr = info_ptr->next;
        }
        hid_free_enumeration(first_info_ptr);

        if (!model.has_value())
            return false;

        auto p = Device::create(model.value());
        if (!p)
            return false;
        p->set_path(path);

        if(!p->connect()) return false;

        if(cache_connections){
            connected_devices [path] = p;
        }

        device = p; //previous device will be disconnected automatically
        current_device_id = path;
        nitrokey::log::Log::setPrefix(path);
        LOGD1("Device successfully changed");
        return true;
    }

    bool NitrokeyManager::connect() {
        std::lock_guard<std::mutex> lock(mex_dev_com_manager);
        vector< shared_ptr<Device> > devices = { make_shared<Stick10>(), make_shared<Stick20>(),
						 make_shared<LibremKey>() };
        bool connected = false;
        for( auto & d : devices ){
            if (d->connect()){
                device = std::shared_ptr<Device>(d);
                connected = true;
            }
        }
        return connected;
    }


    void NitrokeyManager::set_log_function(std::function<void(std::string)> log_function){
      static nitrokey::log::FunctionalLogHandler handler(log_function);
      nitrokey::log::Log::instance().set_handler(&handler);
    }

    bool NitrokeyManager::set_default_commands_delay(int delay){
      if (delay < 20){
        LOG("Delay set too low: " + to_string(delay), Loglevel::WARNING);
        return false;
      }
      Device::set_default_device_speed(delay);
      return true;
    }

    bool NitrokeyManager::connect(const char *device_model) {
      std::lock_guard<std::mutex> lock(mex_dev_com_manager);
      LOG(__FUNCTION__, nitrokey::log::Loglevel::DEBUG_L2);
      switch (device_model[0]){
            case 'P':
                device = make_shared<Stick10>();
                break;
            case 'S':
                device = make_shared<Stick20>();
                break;
            case 'L':
                device = make_shared<LibremKey>();
                break;
            default:
                throw std::runtime_error("Unknown model");
        }
        return device->connect();
    }

    bool NitrokeyManager::connect(device::DeviceModel device_model) {
        const char *model_string;
        switch (device_model) {
            case device::DeviceModel::PRO:
                model_string = "P";
                break;
            case device::DeviceModel::STORAGE:
                model_string = "S";
                break;
            case device::DeviceModel::LIBREM:
                model_string = "L";
                break;
            default:
                throw std::runtime_error("Unknown model");
        }
        return connect(model_string);
    }

    shared_ptr<NitrokeyManager> NitrokeyManager::instance() {
      static std::mutex mutex;
      std::lock_guard<std::mutex> lock(mutex);
        if (_instance == nullptr){
            _instance = make_shared<NitrokeyManager>();
        }
        return _instance;
    }



    bool NitrokeyManager::disconnect() {
      std::lock_guard<std::mutex> lock(mex_dev_com_manager);
      return _disconnect_no_lock();
    }

  bool NitrokeyManager::_disconnect_no_lock() {
    //do not use directly without locked mutex,
    //used by could_be_enumerated, disconnect
    if (device == nullptr){
      return false;
    }
    const auto res = device->disconnect();
    device = nullptr;
    return res;
  }

  bool NitrokeyManager::is_connected() throw(){
      std::lock_guard<std::mutex> lock(mex_dev_com_manager);
      if(device != nullptr){
        auto connected = device->could_be_enumerated();
        if(connected){
          return true;
        } else {
          _disconnect_no_lock();
          return false;
        }
      }
      return false;
  }

  bool NitrokeyManager::could_current_device_be_enumerated() {
    std::lock_guard<std::mutex> lock(mex_dev_com_manager);
    if (device != nullptr) {
      return device->could_be_enumerated();
    }
    return false;
  }

    void NitrokeyManager::set_loglevel(int loglevel) {
      loglevel = max(loglevel, static_cast<int>(Loglevel::ERROR));
      loglevel = min(loglevel, static_cast<int>(Loglevel::DEBUG_L2));
      Log::instance().set_loglevel(static_cast<Loglevel>(loglevel));
    }

    void NitrokeyManager::set_loglevel(Loglevel loglevel) {
      Log::instance().set_loglevel(loglevel);
    }

    void NitrokeyManager::set_debug(bool state) {
        if (state){
            Log::instance().set_loglevel(Loglevel::DEBUG);
        } else {
            Log::instance().set_loglevel(Loglevel::ERROR);
        }
    }


    string NitrokeyManager::get_serial_number() {
      try {
        auto serial_number = this->get_serial_number_as_u32();
        if (serial_number == 0) {
          return "NA";
        } else {
          return nitrokey::misc::toHex(serial_number);
        }
      } catch (DeviceNotConnected& e) {
        return "";
      }
    }

    uint32_t NitrokeyManager::get_serial_number_as_u32() {
        if (device == nullptr) { throw DeviceNotConnected("device not connected"); }
      switch (device->get_device_model()) {
        case DeviceModel::LIBREM:
        case DeviceModel::PRO: {
          auto response = GetStatus::CommandTransaction::run(device);
          return response.data().card_serial_u32;
        }
          break;

        case DeviceModel::STORAGE:
        {
          auto response = stick20::GetDeviceStatus::CommandTransaction::run(device);
          return response.data().ActiveSmartCardID_u32;
        }
          break;
      }
      return 0;
    }

    stick10::GetStatus::ResponsePayload NitrokeyManager::get_status(){
      try{
        auto response = GetStatus::CommandTransaction::run(device);
        return response.data();
      }
      catch (DeviceSendingFailure &e){
//        disconnect();
        throw;
      }
    }

    string NitrokeyManager::get_status_as_string() {
        auto response = GetStatus::CommandTransaction::run(device);
        return response.data().dissect();
    }

    // 15

    bool NitrokeyManager::first_authenticate(const char *pin, const char *temporary_password) {
        auto authreq = get_payload<FirstAuthenticate>();
        strcpyT(authreq.card_password, pin);
        strcpyT(authreq.temporary_password, temporary_password);
        FirstAuthenticate::CommandTransaction::run(device, authreq);
        return true;
    }

    void NitrokeyManager::change_user_PIN(const char *current_PIN, const char *new_PIN) {
        change_PIN_general<ChangeUserPin, PasswordKind::User>(current_PIN, new_PIN);
    }

    void NitrokeyManager::change_admin_PIN(const char *current_PIN, const char *new_PIN) {
        change_PIN_general<ChangeAdminPin, PasswordKind::Admin>(current_PIN, new_PIN);
    }

    template <typename ProCommand, PasswordKind StoKind>
    void NitrokeyManager::change_PIN_general(const char *current_PIN, const char *new_PIN) {
        switch (device->get_device_model()){
            case DeviceModel::LIBREM:
            case DeviceModel::PRO:
            {
                auto p = get_payload<ProCommand>();
                strcpyT(p.old_pin, current_PIN);
                strcpyT(p.new_pin, new_PIN);
                ProCommand::CommandTransaction::run(device, p);
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
                ChangeAdminUserPin20Current::CommandTransaction::run(device, p);
                ChangeAdminUserPin20New::CommandTransaction::run(device, p2);
            }
                break;
        }

    }

    void NitrokeyManager::enable_password_safe(const char *user_pin) {
        //The following command will cancel enabling PWS if it is not supported
        auto a = get_payload<IsAESSupported>();
        strcpyT(a.user_password, user_pin);
        IsAESSupported::CommandTransaction::run(device, a);

        auto p = get_payload<EnablePasswordSafe>();
        strcpyT(p.user_password, user_pin);
        EnablePasswordSafe::CommandTransaction::run(device, p);
    }

    vector <uint8_t> NitrokeyManager::get_password_safe_slot_status() {
        auto responsePayload = GetPasswordSafeSlotStatus::CommandTransaction::run(device);
        vector<uint8_t> v = vector<uint8_t>(responsePayload.data().password_safe_status,
                                            responsePayload.data().password_safe_status
                                            + sizeof(responsePayload.data().password_safe_status));
        return v;
    }

    uint8_t NitrokeyManager::get_user_retry_count() {
        if(device->get_device_model() == DeviceModel::STORAGE){
          stick20::GetDeviceStatus::CommandTransaction::run(device);
        }
        auto response = GetUserPasswordRetryCount::CommandTransaction::run(device);
        return response.data().password_retry_count;
    }

    uint8_t NitrokeyManager::get_admin_retry_count() {
        if(device->get_device_model() == DeviceModel::STORAGE){
          stick20::GetDeviceStatus::CommandTransaction::run(device);
        }
        auto response = GetPasswordRetryCount::CommandTransaction::run(device);
        return response.data().password_retry_count;
    }

    void NitrokeyManager::lock_device() {
        LockDevice::CommandTransaction::run(device);
    }

    char * NitrokeyManager::get_password_safe_slot_name(uint8_t slot_number) {
        if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        auto p = get_payload<GetPasswordSafeSlotName>();
        p.slot_number = slot_number;
        auto response = GetPasswordSafeSlotName::CommandTransaction::run(device, p);
        return strndup((const char *) response.data().slot_name, max_string_field_length);
    }

    bool NitrokeyManager::is_valid_password_safe_slot_number(uint8_t slot_number) const { return slot_number < 16; }

    char * NitrokeyManager::get_password_safe_slot_login(uint8_t slot_number) {
        if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        auto p = get_payload<GetPasswordSafeSlotLogin>();
        p.slot_number = slot_number;
        auto response = GetPasswordSafeSlotLogin::CommandTransaction::run(device, p);
        return strndup((const char *) response.data().slot_login, max_string_field_length);
    }

    char * NitrokeyManager::get_password_safe_slot_password(uint8_t slot_number) {
        if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        auto p = get_payload<GetPasswordSafeSlotPassword>();
        p.slot_number = slot_number;
        auto response = GetPasswordSafeSlotPassword::CommandTransaction::run(device, p);
        return strndup((const char *) response.data().slot_password, max_string_field_length); //FIXME use secure way
    }

    void NitrokeyManager::write_password_safe_slot(uint8_t slot_number, const char *slot_name, const char *slot_login,
                                                       const char *slot_password) {
        if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        auto p = get_payload<SetPasswordSafeSlotData>();
        p.slot_number = slot_number;
        strcpyT(p.slot_name, slot_name);
        strcpyT(p.slot_password, slot_password);
        SetPasswordSafeSlotData::CommandTransaction::run(device, p);

        auto p2 = get_payload<SetPasswordSafeSlotData2>();
        p2.slot_number = slot_number;
        strcpyT(p2.slot_login_name, slot_login);
        SetPasswordSafeSlotData2::CommandTransaction::run(device, p2);
    }

    void NitrokeyManager::erase_password_safe_slot(uint8_t slot_number) {
        if (!is_valid_password_safe_slot_number(slot_number)) throw InvalidSlotException(slot_number);
        auto p = get_payload<ErasePasswordSafeSlot>();
        p.slot_number = slot_number;
        ErasePasswordSafeSlot::CommandTransaction::run(device, p);
    }

    void NitrokeyManager::user_authenticate(const char *user_password, const char *temporary_password) {
        auto p = get_payload<UserAuthenticate>();
        strcpyT(p.card_password, user_password);
        strcpyT(p.temporary_password, temporary_password);
        UserAuthenticate::CommandTransaction::run(device, p);
    }

    void NitrokeyManager::build_aes_key(const char *admin_password) {
        switch (device->get_device_model()) {
            case DeviceModel::LIBREM:
            case DeviceModel::PRO: {
                auto p = get_payload<BuildAESKey>();
                strcpyT(p.admin_password, admin_password);
                BuildAESKey::CommandTransaction::run(device, p);
                break;
            }
            case DeviceModel::STORAGE : {
                auto p = get_payload<stick20::CreateNewKeys>();
                strcpyT(p.password, admin_password);
                p.set_defaults();
                stick20::CreateNewKeys::CommandTransaction::run(device, p);
                break;
            }
        }
    }

    void NitrokeyManager::factory_reset(const char *admin_password) {
        auto p = get_payload<FactoryReset>();
        strcpyT(p.admin_password, admin_password);
        FactoryReset::CommandTransaction::run(device, p);
    }

    void NitrokeyManager::unlock_user_password(const char *admin_password, const char *new_user_password) {
      switch (device->get_device_model()){
        case DeviceModel::LIBREM:
        case DeviceModel::PRO: {
          auto p = get_payload<stick10::UnlockUserPassword>();
          strcpyT(p.admin_password, admin_password);
          strcpyT(p.user_new_password, new_user_password);
          stick10::UnlockUserPassword::CommandTransaction::run(device, p);
          break;
        }
        case DeviceModel::STORAGE : {
          auto p2 = get_payload<ChangeAdminUserPin20Current>();
          p2.set_defaults();
          strcpyT(p2.password, admin_password);
          ChangeAdminUserPin20Current::CommandTransaction::run(device, p2);
          auto p3 = get_payload<stick20::UnlockUserPin>();
          p3.set_defaults();
          strcpyT(p3.password, new_user_password);
          stick20::UnlockUserPin::CommandTransaction::run(device, p3);
          break;
        }
      }
    }

    bool NitrokeyManager::is_authorization_command_supported(){
      //authorization command is supported for versions equal or below:
        auto m = std::unordered_map<DeviceModel , int, EnumClassHash>({
                                               {DeviceModel::PRO, 7},
                                               {DeviceModel::LIBREM, 7},
                                               {DeviceModel::STORAGE, 53},
         });
        return get_minor_firmware_version() <= m[device->get_device_model()];
    }

    DeviceModel NitrokeyManager::get_connected_device_model() const{
        if (device == nullptr){
            throw DeviceNotConnected("device not connected");
        }
      return device->get_device_model();
    }

    bool NitrokeyManager::is_smartcard_in_use(){
      try{
        stick20::CheckSmartcardUsage::CommandTransaction::run(device);
      }
      catch(const CommandFailedException & e){
        return e.reason_smartcard_busy();
      }
      return false;
    }

    uint8_t NitrokeyManager::get_minor_firmware_version(){
      switch(device->get_device_model()){
        case DeviceModel::LIBREM:
        case DeviceModel::PRO:{
          auto status_p = GetStatus::CommandTransaction::run(device);
          return status_p.data().firmware_version_st.minor; //7 or 8
        }
        case DeviceModel::STORAGE:{
          auto status = stick20::GetDeviceStatus::CommandTransaction::run(device);
          auto test_firmware = status.data().versionInfo.build_iteration != 0;
          if (test_firmware)
            LOG("Development firmware detected. Increasing minor version number.", nitrokey::log::Loglevel::WARNING);
          return status.data().versionInfo.minor + (test_firmware? 1 : 0);
        }
      }
      return 0;
    }
    uint8_t NitrokeyManager::get_major_firmware_version(){
      switch(device->get_device_model()){
        case DeviceModel::LIBREM:
        case DeviceModel::PRO:{
          auto status_p = GetStatus::CommandTransaction::run(device);
          return status_p.data().firmware_version_st.major; //0
        }
        case DeviceModel::STORAGE:{
          auto status = stick20::GetDeviceStatus::CommandTransaction::run(device);
          return status.data().versionInfo.major;
        }
      }
      return 0;
    }

    bool NitrokeyManager::is_AES_supported(const char *user_password) {
        auto a = get_payload<IsAESSupported>();
        strcpyT(a.user_password, user_password);
        IsAESSupported::CommandTransaction::run(device, a);
        return true;
    }

    const string NitrokeyManager::get_current_device_id() const {
        return current_device_id;
    }

  void NitrokeyManager::enable_firmware_update_pro(const char *firmware_pin) {
    auto p = get_payload<FirmwareUpdate>();
    strcpyT(p.firmware_password, firmware_pin);
    FirmwareUpdate::CommandTransaction::run(device, p);
  }

  void
  NitrokeyManager::change_firmware_update_password_pro(const char *firmware_pin_current, const char *firmware_pin_new) {
    auto p = get_payload<FirmwarePasswordChange>();
    strcpyT(p.firmware_password_current, firmware_pin_current);
    strcpyT(p.firmware_password_new, firmware_pin_new);
    FirmwarePasswordChange::CommandTransaction::run(device, p);
  }

}
