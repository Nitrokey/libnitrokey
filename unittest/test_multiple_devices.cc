
static const char *const default_admin_pin = "12345678";
static const char *const default_user_pin = "123456";
const char * temporary_password = "123456789012345678901234";
const char * RFC_SECRET = "12345678901234567890";

#include "catch.hpp"

#include <iostream>
#include <NitrokeyManager.h>
#include <iostream>
#include <stick20_commands.h>

using namespace nitrokey;


TEST_CASE("List devices", "[BASIC]") {
    shared_ptr<Stick20> d = make_shared<Stick20>();
    auto v = d->enumerate();
    REQUIRE(v.size() > 0);
    for (auto a : v){
        std::cout << a;
        d->set_path(a);
        d->connect();
        auto res = GetStatus::CommandTransaction::run(d);
        auto res2 = GetDeviceStatus::CommandTransaction::run(d);
        std::cout << " " << res.data().card_serial_u32 << " "
                  << res.data().get_card_serial_hex()
                  << " " << std::to_string(res2.data().versionInfo.minor)
                  << std::endl;
        d->disconnect();
    }
}

TEST_CASE("Regenerate AES keys", "[BASIC]") {
    shared_ptr<Stick20> d = make_shared<Stick20>();
    auto v = d->enumerate();
    REQUIRE(v.size() > 0);

    std::vector<shared_ptr<Stick20>> devices;
    for (auto a : v){
        std::cout << a << endl;
        d = make_shared<Stick20>();
        d->set_path(a);
        d->connect();
        devices.push_back(d);
    }

    for (auto d : devices){
        auto res2 = GetDeviceStatus::CommandTransaction::run(d);
        std::cout << std::to_string(res2.data().versionInfo.minor) << std::endl;
//        nitrokey::proto::stick20::CreateNewKeys::CommandPayload p;
//        p.set_defaults();
//        memcpy(p.password, "12345678", 8);
//        auto res3 = nitrokey::proto::stick20::CreateNewKeys::CommandTransaction::run(d, p);
    }

    for (auto d : devices){
        //TODO watch out for multiple hid_exit calls
        d->disconnect();
    }
}


TEST_CASE("Use API", "[BASIC]") {
    auto nm = NitrokeyManager::instance();
    nm->set_loglevel(2);
    auto v = nm->list_devices();
    REQUIRE(v.size() > 0);

    for (int i=0; i<10; i++){
        for (auto a : v) {
            std::cout <<"Connect with: " << a <<
            " " << std::boolalpha << nm->connect_with_path(a) << " ";
            try{
                auto status_storage = nm->get_status_storage();
                std::cout << status_storage.ActiveSmartCardID_u32
                          << " " << status_storage.ActiveSD_CardID_u32
                          << std::endl;

                nm->fill_SD_card_with_random_data("12345678");
            }
            catch (const LongOperationInProgressException &e){
                std::cout << "long operation in progress on " << a
                        << " " << std::to_string(e.progress_bar_value) << std::endl;
                this_thread::sleep_for(1000ms);
            }
        }
        std::cout <<"Iteration: " << i << std::endl;
    }

}